#include "./multi_host_param.h"
#include "./multi_host_port.h"
#include "./multi_host_cache.h"

#define __PLOOC_CLASS_IMPLEMENT
#include "../../utilities/ooc.h"

#include "cmsis_os2.h"

#define QUEUE_SIZE              2

// 根据端口角色，处理raw_data。 若是从机raw_data 转 proxy_response_t
def_simple_fsm(process_raw_data_list,
               def_params(
                   uint8_t chIndex;
                   uint8_t chPort;
                   fsm(transform_request) tTransfomRequest;
                   fsm(transform_response) tTransfomResponse;
               ))
end_def_simple_fsm(process_raw_data_list)

declare_fsm_initialiser(process_raw_data_list)
declare_fsm_implementation(process_raw_data_list)

//--------------------------------------------------------

def_simple_fsm(process_request_list,
               def_params(
                   proxy_request_t *ptRequest;
                   osMessageQueueId_t tQueueID;))
end_def_simple_fsm(process_request_list)
declare_fsm_initialiser(process_request_list)
declare_fsm_implementation(process_request_list)

//--------------------------------------------------------

// 读取数据并转发
def_simple_fsm(process_slave_port,
               def_params(
                   uint8_t chPort;
                   proxy_request_t *ptRequest;
                   osMessageQueueId_t tQueueID;
                   const proxy_setting_t *ptSetting;
                   proxy_response_t *ptResponse;

                   fsm(read_cache_data) tReadCache;
                   fsm(forward_raw_data) tForwardData;
                   fsm(safe_read_and_write_operation) tOperation;
               ))
end_def_simple_fsm(process_slave_port)

declare_fsm_initialiser(process_slave_port, args(uint8_t chPort))
declare_fsm_implementation(process_slave_port)

//--------------------------------------------------------

fsm(process_raw_data_list) s_tProcessRawDataList;
fsm(process_request_list) s_tProcessRequestList;
fsm(process_slave_port) s_tProcessSlavePort;
fsm(process_slave_port) s_tProcessSlavePort_2;


fsm(receive_data) s_tReceive;
fsm(send_data) s_tSend;

fsm(update_cache_record) s_tUpdateRecord;
fsm(delete_expired_cache_record) s_tDeleteRecord;

struct {
    uint8_t chPort;
    osMessageQueueId_t tQueue;
} s_tSlavePortRequestQueueBuf[TOTAL_PORT_NUM];

static osMessageQueueId_t get_slave_queue_by_port(uint8_t chPort);
static void register_slave_port_queue(uint8_t chPort, osMessageQueueId_t tQueueId);

/**
 *   代理主机初始化参数
*/
void modbus_proxy_device_init(void)
{
    const port_resource_t *ptRes = NULL;

    multi_host_param_init();
    multi_host_recv_send_init();
    multi_host_request_response_init();

    cache_record_init();

    for (uint8_t i = 0; i < proxy_get_slave_port_num(); i++) {
        ptRes = proxy_get_slave_port_resource(i);
        if (NULL != ptRes) {
            s_tSlavePortRequestQueueBuf[i].chPort = ptRes->chPort;
            s_tSlavePortRequestQueueBuf[i].tQueue = NULL;
        }
    }

    /*! TODO 并发读取，可能无法保证request请求保序处理 */
    init_fsm(process_slave_port, &s_tProcessSlavePort, args(CH_PORT4));
    init_fsm(process_slave_port, &s_tProcessSlavePort_2, args(CH_PORT4));

    init_fsm(process_raw_data_list, &s_tProcessRawDataList);
    init_fsm(process_request_list, &s_tProcessRequestList);
    init_fsm(receive_data, &s_tReceive);
    init_fsm(send_data, &s_tSend);

    init_fsm(update_cache_record, &s_tUpdateRecord);
    init_fsm(delete_expired_cache_record, &s_tDeleteRecord);
}

/*
 *  代理主机处理函数
 **/
void modbus_proxy_device_process_task(void)
{
    call_fsm(process_slave_port, &s_tProcessSlavePort);
    call_fsm(process_slave_port, &s_tProcessSlavePort_2);

    call_fsm(process_raw_data_list, &s_tProcessRawDataList);
    call_fsm(process_request_list, &s_tProcessRequestList);
    call_fsm(receive_data, &s_tReceive);

    call_fsm(update_cache_record, &s_tUpdateRecord);
    call_fsm(delete_expired_cache_record, &s_tDeleteRecord);

}

/*
 *  代理主机发送任务处理函数，独立发送任务是防止发送会阻塞任务的执行
 **/
void modbus_proxy_device_send_task(void)
{
    call_fsm(send_data, &s_tSend);
}

/*
 * 处理端口请求，获取主机端口的请求，如果是内部请求直接返回。
 * 需要转发的请求，等待应答。
*/

fsm_initialiser(process_slave_port, args(uint8_t chPort))
init_body({
    this.chPort = chPort;

    this.tQueueID = get_slave_queue_by_port(this.chPort);
    if (NULL == this.tQueueID)
    {
        this.tQueueID = osMessageQueueNew(QUEUE_SIZE, sizeof(proxy_request_t *), NULL);
        register_slave_port_queue(this.chPort, this.tQueueID);
    }

    this.ptSetting = proxy_get_global_setting();
})

fsm_implementation(process_slave_port)
{
    def_states(CHECK_REQUEST_QUEUE, READ_CACHE_DATA, READ_DATA, WRITE_DATA, REPLY_RESPONSE)

    body_begin()

    on_start(
        this.ptResponse = NULL;
    )

    state(CHECK_REQUEST_QUEUE) { //-V729
        if (osOK == osMessageQueueGet(this.tQueueID, &this.ptRequest, NULL, 0)) {
            MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_RED"[request] from port[%d] to port[%d]\r\n", this.ptRequest->chPortSrc, this.ptRequest->chPortDstBuf[0]);
            MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_RED"[request] request 0x%08x rawdata 0x%08x\r\n", this.ptRequest, this.ptRequest->ptRaw);

            this.ptResponse = NULL;

            // Write operation
            if (is_request_write_operation(this.ptRequest)) {
                init_fsm(safe_read_and_write_operation, &this.tOperation, args(this.ptRequest));
                update_state_to(WRITE_DATA);
            }

            // Read operation
            if (0 == this.ptSetting->hwCacheExpirationTime) { // 0表示不缓存
                init_fsm(safe_read_and_write_operation, &this.tOperation, args(this.ptRequest));
                update_state_to(READ_DATA);
            } else {
                init_fsm(read_cache_data, &this.tReadCache, args(this.ptRequest));
                update_state_to(READ_CACHE_DATA);
            }
        }
    }

    state(READ_CACHE_DATA) {
        if (fsm_rt_cpl == call_fsm(read_cache_data, &this.tReadCache, args(&this.ptResponse))) {
            if (NULL != this.ptResponse) {
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[cache] from port[%d] to port[%d]\r\n", this.ptResponse->chPortSrc, this.ptRequest->chPortSrc);
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[cache] response 0x%08x rawdata 0x%08x\r\n", this.ptResponse, this.ptResponse->ptRaw);

                init_fsm(forward_raw_data, &this.tForwardData,
                         args(this.ptRequest->chPortSrc, this.ptResponse->ptRaw->chBuf, this.ptResponse->ptRaw->hwBufSize));

                update_state_to(REPLY_RESPONSE);
            } else {
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"[cache] read failed\r\n");
                init_fsm(safe_read_and_write_operation, &this.tOperation, args(this.ptRequest));
                update_state_to(READ_DATA);
            }
        }
    }

    state(READ_DATA) {
        if (fsm_rt_cpl == call_fsm(safe_read_and_write_operation, &this.tOperation, args(&this.ptResponse))) {
            if (NULL != this.ptResponse) {
                create_cache_record(this.ptRequest);
                refresh_cache_record(this.ptRequest, this.ptResponse);

                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[response] from port[%d] to port[%d]\r\n", this.ptResponse->chPortSrc, this.ptRequest->chPortSrc);
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[response] response 0x%08x rawdata 0x%08x\r\n", this.ptResponse, this.ptResponse->ptRaw);

                init_fsm(forward_raw_data, &this.tForwardData,
                         args(this.ptRequest->chPortSrc, this.ptResponse->ptRaw->chBuf, this.ptResponse->ptRaw->hwBufSize));

                update_state_to(REPLY_RESPONSE);
            } else {
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"[response] read failed\r\n");
                RAW_DATA_FREE(this.ptRequest->ptRaw);
                PROXY_REQUEST_FREE(this.ptRequest);
                reset_fsm();
            }
        }
    }

    state(WRITE_DATA) {
        if (fsm_rt_cpl == call_fsm(safe_read_and_write_operation, &this.tOperation, args(&this.ptResponse))) {
            if (NULL != this.ptResponse) {
                delete_cache_record_by_request(this.ptRequest);
                refresh_cache_record(this.ptRequest, this.ptResponse);

                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[response] from port[%d] to port[%d]\r\n", this.ptResponse->chPortSrc, this.ptRequest->chPortSrc);
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_YELLOW"[response] response 0x%08x rawdata 0x%08x\r\n", this.ptResponse, this.ptResponse->ptRaw);

                init_fsm(forward_raw_data, &this.tForwardData,
                         args(this.ptRequest->chPortSrc, this.ptResponse->ptRaw->chBuf, this.ptResponse->ptRaw->hwBufSize));

                update_state_to(REPLY_RESPONSE);
            } else {
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"[response] write failed\r\n");
                RAW_DATA_FREE(this.ptRequest->ptRaw);
                PROXY_REQUEST_FREE(this.ptRequest);
                reset_fsm();
            }
        }
    }

    state(REPLY_RESPONSE) {
        if (fsm_rt_cpl == call_fsm(forward_raw_data, &this.tForwardData)) {
            RAW_DATA_FREE(this.ptResponse->ptRaw);
            PROXY_RESPONSE_FREE(this.ptResponse);

            RAW_DATA_FREE(this.ptRequest->ptRaw);
            PROXY_REQUEST_FREE(this.ptRequest);

            transfer_to(CHECK_REQUEST_QUEUE);
        }
    }

    body_end()
}

fsm_initialiser(process_request_list)
init_body()

fsm_implementation(process_request_list)
{
    def_states(CHECK_REQUEST, UNICAST_REQUEST, BROADCAST_REQUEST)

    proxy_request_t *ptRequestCopy = NULL;
    body_begin()

    state(CHECK_REQUEST) { //-V729
        this.ptRequest = receive_request();
        if (NULL != this.ptRequest) {
            if (1 == this.ptRequest->chPortDstNum) {
                update_state_to(UNICAST_REQUEST);
            } else {
                update_state_to(BROADCAST_REQUEST);
            }
        }
    }

    state(UNICAST_REQUEST) {
        this.tQueueID = get_slave_queue_by_port(this.ptRequest->chPortDstBuf[0]);
        if (NULL != this.tQueueID) {
            if (osOK == osMessageQueuePut(this.tQueueID, &this.ptRequest, 0, 0)) {
                transfer_to(CHECK_REQUEST);
            } else {
                MLOG_RAW("Put &ptRequest 0x%08x into queue 0x%08x failed\r\n", this.ptRequest, this.tQueueID);
                MLOG_RAW("queue count %d\r\n", osMessageQueueGetCount(this.tQueueID));
            }
        } else {
            RAW_DATA_FREE(this.ptRequest->ptRaw);
            PROXY_REQUEST_FREE(this.ptRequest);
            transfer_to(CHECK_REQUEST);
        }
    }

    state(BROADCAST_REQUEST) {
        // todo 存在多个端口消费同一个请求，而不是请求的拷贝
        for (uint8_t i = 0; i < this.ptRequest->chPortDstNum; i++) {
            this.tQueueID = get_slave_queue_by_port(this.ptRequest->chPortDstBuf[i]);
            if (NULL == this.tQueueID) {
                continue;
            }

            ptRequestCopy = proxy_mb_request_copy(this.ptRequest);
            if (NULL == ptRequestCopy) {
                continue;
            }

            ptRequestCopy->chPortDstBuf[0] = this.ptRequest->chPortDstBuf[i];
            ptRequestCopy->chPortDstNum = 1;

            if (osOK != osMessageQueuePut(this.tQueueID, &ptRequestCopy, 0, 0)) {
                RAW_DATA_FREE(ptRequestCopy->ptRaw);
                PROXY_REQUEST_FREE(ptRequestCopy);
            }
        }

        RAW_DATA_FREE(this.ptRequest->ptRaw);
        PROXY_REQUEST_FREE(this.ptRequest);
        transfer_to(CHECK_REQUEST);
    }

    body_end()
}

fsm_initialiser(process_raw_data_list)
init_body()

fsm_implementation(process_raw_data_list)
{
    def_states(CHECK_PORT_ROLE, HANDLE_MASTER_PORT_DATA, HANDLE_SLAVE_PORT_DATA)
    const port_resource_t *c_ptRes = NULL;

    body_begin()

    on_start(
        this.chIndex = 0;
        init_fsm(transform_request, &this.tTransfomRequest);
        init_fsm(transform_response, &this.tTransfomResponse);
    )

    state(CHECK_PORT_ROLE) { //-V729
        if (this.chIndex >= TOTAL_PORT_NUM) {
            fsm_cpl();
        }

        this.chPort = g_chPortBuf[this.chIndex++];
        c_ptRes = proxy_get_port_resource(this.chPort);
        if (NULL != c_ptRes) {
            if (MASTER == c_ptRes->tRole) {
                update_state_to(HANDLE_MASTER_PORT_DATA);
            } else if (SLAVE == c_ptRes->tRole) {
                update_state_to(HANDLE_SLAVE_PORT_DATA);
            }
        }
    }

    state(HANDLE_MASTER_PORT_DATA) {
        if (fsm_rt_cpl == call_fsm(transform_request, &this.tTransfomRequest, args(this.chPort))) {
            transfer_to(CHECK_PORT_ROLE);
        }
    }

    state(HANDLE_SLAVE_PORT_DATA) {
        if (fsm_rt_cpl == call_fsm(transform_response, &this.tTransfomResponse, args(this.chPort))) {
            transfer_to(CHECK_PORT_ROLE);
        }
    }

    body_end()
}

static osMessageQueueId_t get_slave_queue_by_port(uint8_t chPort)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (s_tSlavePortRequestQueueBuf[i].chPort == chPort) {
            return s_tSlavePortRequestQueueBuf[i].tQueue;
        }
    }
    return NULL;
}

static void register_slave_port_queue(uint8_t chPort, osMessageQueueId_t tQueueId)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (s_tSlavePortRequestQueueBuf[i].chPort == chPort) {
            s_tSlavePortRequestQueueBuf[i].tQueue = tQueueId;
        }
    }
}
