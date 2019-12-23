/*********************************************************************
*                      Hangzhou Lingzhi Lzlinks                      *
*                        Internet of Things                          *
**********************************************************************
*                                                                    *
*            (c) 2018 - 8102 Hangzhou Lingzhi Lzlinks                *
*                                                                    *
*       www.lzlinks.com     Support: embedzjh@gmail.com              *
*                                                                    *
**********************************************************************
*                                                                    *
*       multi_host_request_response.c *                              *
*                                                                    *
**********************************************************************
*/

#include "./multi_host_request_response.h"
#include "./multi_host_param.h"
#include "./multi_host_utilities.h"
#include "./multi_host_port.h"
#include "./dlink/dlink.h"

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/

// 将raw_data 转换成 proxy_request_t
def_simple_fsm(transform_request,
               def_params(
                   raw_data_t *ptData;
                   proxy_request_t *ptRequest;
               ))
end_def_simple_fsm(transform_request)

declare_fsm_initialiser(transform_request)
declare_fsm_implementation(transform_request, args(uint8_t chPort))

// 将raw_data 转换成 proxy_response_t
def_simple_fsm(transform_response,
               def_params(
                   raw_data_t *ptData;
                   proxy_response_t *ptResponse;
               ))
end_def_simple_fsm(rawdata_transfotransform_responserm_response)

declare_fsm_initialiser(transform_response)
declare_fsm_implementation(transform_response, args(uint8_t chPort))

// 获取无缓存的应答，即直接从端口获取设备数据
def_simple_fsm(read_and_write_operation,
               def_params(
                   uint8_t chPort;
                   proxy_request_t *ptRequest;
                   uint16_t hwCounter;
                   raw_data_t *ptRaw;
                   fsm(forward_raw_data) tForwardRawData;
                   uint32_t wBaudrate;
                   uint32_t wTimeout;
               ))
end_def_simple_fsm(read_and_write_operation)

declare_fsm_initialiser(read_and_write_operation, args(proxy_request_t *ptRequest))
declare_fsm_implementation(read_and_write_operation, args(proxy_response_t **pptResponse))

// 获取无缓存的应答，即直接从端口获取设备数据
def_simple_fsm(safe_read_and_write_operation,
               def_params(
                   fsm(read_and_write_operation) tOperation;
                   critical_sector_t *ptLock;
               ))
end_def_simple_fsm(safe_read_and_write_operation)

declare_fsm_initialiser(safe_read_and_write_operation, args(proxy_request_t *ptRequest))
declare_fsm_implementation(safe_read_and_write_operation, args(proxy_response_t **pptResponse))

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

dlink_list_t s_tLinkRequestRecv;            // 主机端口请求接收链表
dlink_list_t s_tLiknResponseRecv;           // 从机端口应答接收链表

static struct port_operation_lock_t {
    uint8_t chPort;
    critical_sector_t tLock;
} s_tOperationLockBuffer[TOTAL_PORT_NUM];

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

static proxy_response_t *receive_response_object(uint8_t chPort);
static void clear_received_response_object(uint8_t chPort);
static critical_sector_t *get_operation_lock(uint8_t chPort);

/*********************************************************************
*
*       Function Implention
*
**********************************************************************
*/

/**
 * \brief       接收端口的数据，并把该数据转换为 request 请求,
 *              最后插入到 request 请求链表中
 * \param[in]   chPort     端口号
 * \return      fsm_rt_t
 */
fsm_initialiser(transform_request)
init_body()

fsm_implementation(transform_request, args(uint8_t chPort))
{
    def_states(CHECK_RAW_DATA_LIST, MALLOC_REQUEST, TRANSFORM_RESQUEST, SAVE_REQUEST)
    body_begin()

    on_start(
        this.ptData = NULL;
        this.ptRequest = NULL;
    )

    state(CHECK_RAW_DATA_LIST) { //-V729
        this.ptData = receive_raw_data_by_source_port(chPort);
        if (NULL != this.ptData) {
            update_state_to(MALLOC_REQUEST);
        } else {
            fsm_cpl();
        }
    }

    state(MALLOC_REQUEST) {
        this.ptRequest = (proxy_request_t *)PROXY_REQUEST_MALLOC();
        if (NULL != this.ptRequest) {
            update_state_to(TRANSFORM_RESQUEST);
        }
    }

    state(TRANSFORM_RESQUEST) {
        if (0 == raw_data_transform_request(this.ptData, this.ptRequest)) {
            update_state_to(SAVE_REQUEST);
        } else {
            MLOG_RAW("0x%08x transform request failed\r\n", this.ptData);
            RAW_DATA_FREE(this.ptData);
            PROXY_REQUEST_FREE(this.ptRequest);
            fsm_cpl();
        }
    }

    state(SAVE_REQUEST) {
        if (0 == DLINK_LIST_INSERT_LAST(&s_tLinkRequestRecv, this.ptRequest)) {
            fsm_cpl();
        }
    }

    body_end()
}

/**
 * \brief       接收端口的数据，并把该数据转换为 response 应答,
 *              最后插入到 response 应答链表中
 * \param[in]   chPort     端口号
 * \return      fsm_rt_t
 */
fsm_initialiser(transform_response)
init_body()

fsm_implementation(transform_response, args(uint8_t chPort))
{
    def_states(CHECK_RAW_DATA_LIST, MALLOC_RESPONSE, TRANSFORM_RESPONSE, SAVE_RESPONSE)
    body_begin()

    on_start(
        this.ptData = NULL;
        this.ptResponse = NULL;
    )

    state(CHECK_RAW_DATA_LIST) { //-V729
        this.ptData = receive_raw_data_by_source_port(chPort);
        if (NULL != this.ptData) {
            update_state_to(MALLOC_RESPONSE);
        } else {
            fsm_cpl();
        }
    }

    state(MALLOC_RESPONSE) {
        this.ptResponse = (proxy_response_t *)PROXY_RESPONSE_MALLOC();
        if (NULL != this.ptResponse) {
            update_state_to(TRANSFORM_RESPONSE);
        }
    }

    state(TRANSFORM_RESPONSE) {
        if (0 == raw_data_transform_response(this.ptData, this.ptResponse)) {
            update_state_to(SAVE_RESPONSE);
        } else {
            MLOG_RAW("0x%08x transform response failed\r\n", this.ptData);
            RAW_DATA_FREE(this.ptData);
            PROXY_RESPONSE_FREE(this.ptResponse);
            fsm_cpl();
        }
    }

    state(SAVE_RESPONSE) {
        if (0 == DLINK_LIST_INSERT_LAST(&s_tLiknResponseRecv, this.ptResponse)) {
            fsm_cpl();
        }
    }

    body_end()
}

/**
 * \brief       转发数据给MODBUS从机，并获取MODBUS从机应答
 * \param[in]   ptRequest     request 请求结构体指针
 * \return      fsm_rt_t
 */
fsm_initialiser(read_and_write_operation, args(proxy_request_t *ptRequest))
init_body(
    const port_config_t *ptCfg = NULL;

    this.ptRequest = ptRequest;
    this.chPort = this.ptRequest->chPortDstBuf[0];  // 只可能是一个端口

    ptCfg = proxy_get_port_config(this.chPort);
    this.wBaudrate = ptCfg->tConfig.wBaudRate;
)

fsm_implementation(read_and_write_operation, args(proxy_response_t **pptResponse))
{
    def_states(CLEAN_CACHE, FORWARD_DATA, WAIT_RESPONSE)

    proxy_response_t *ptResponse = NULL;

    body_begin()

    on_start(
        this.wTimeout = calc_wait_response_time(this.ptRequest->ptRaw, this.wBaudrate);
        this.hwCounter = 0;
        init_fsm(forward_raw_data, &this.tForwardRawData,
                 args(this.chPort, this.ptRequest->ptRaw->chBuf, this.ptRequest->ptRaw->hwBufSize));
    )

    state(CLEAN_CACHE) { //-V729
        clear_received_raw_data_object(this.chPort);
        clear_received_response_object(this.chPort);
        update_state_to(FORWARD_DATA);
    }

    state(FORWARD_DATA) {
        if (fsm_rt_cpl == call_fsm(forward_raw_data, &this.tForwardRawData)) {
            update_state_to(WAIT_RESPONSE);
        }
    }

    state(WAIT_RESPONSE) {
        ptResponse = receive_response_object(this.chPort);
        if (NULL != ptResponse) {
            ptResponse->chPortDst = this.ptRequest->chPortSrc;
            *pptResponse = ptResponse;
            fsm_cpl();
        } else if (0 == (this.wTimeout--)) {
            MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"[response] timeout\r\n");
            *pptResponse = NULL;
            fsm_cpl();
        }
    }

    body_end()
}

/**
* \brief       在一段时间内，对端口的读写操作进行保护，防止对端口的并发读写
* \param[in]   ptRequest     request 请求结构体指针
* \param[in]   ptLock        原子操作保护锁
* \return      fsm_rt_t
*/
fsm_initialiser(safe_read_and_write_operation, args(proxy_request_t *ptRequest))
init_body(
    this.ptLock = get_operation_lock(ptRequest->chPortDstBuf[0]);
    init_fsm(read_and_write_operation, &this.tOperation, args(ptRequest));
)

fsm_implementation(safe_read_and_write_operation, args(proxy_response_t **pptResponse))
{
    def_states(ENTER_SAFE_OPERATION, PROCESS_OPERATION, COMPARE_TIME)

    body_begin()

    state(ENTER_SAFE_OPERATION) { //-V729
        if (ENTER_CRITICAL_SECTOR(this.ptLock)) {
            update_state_to(PROCESS_OPERATION);
        }
    }

    state(PROCESS_OPERATION) {
        if (fsm_rt_cpl == call_fsm(read_and_write_operation, &this.tOperation, args(pptResponse))) {
            LEAVE_CRITICAL_SECTOR(this.ptLock);
            fsm_cpl();
        }
    }

    body_end()
}


/**
 * \brief 初始化 request 和 resposne 存储链表
 */
void multi_host_request_response_init(void)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        s_tOperationLockBuffer[i].chPort = g_chPortBuf[i];
        INIT_CRITICAL_SECTOR(&s_tOperationLockBuffer[i].tLock);
    }

    dlink_list_init(&s_tLinkRequestRecv);
    dlink_list_init(&s_tLiknResponseRecv);
}

/**
 * \brief  从 request 请求链表，取出第一条请求
 */
proxy_request_t *receive_request(void)
{
    return DLINK_LIST_POP_FIRST(&s_tLinkRequestRecv);
}

/**
 * \brief  寻找相同端口号的钩子函数
 */
static int32_t find_same_source_port_hook(void *res, void *data)
{
    uint8_t *pchPort = (uint8_t *)data;
    proxy_response_t *ptResp = (proxy_response_t *)res;

    return ptResp->chPortSrc == *pchPort ? 0 : -1;
}

/**
 * \brief  释放内存空间的钩子函数
 */
static int32_t destroy_response_object_hook(void *res)
{
    proxy_response_t *ptResp = (proxy_response_t *)res;

    RAW_DATA_FREE(ptResp->ptRaw);
    PROXY_RESPONSE_FREE(ptResp);
    return 0;
}

/**
 * \brief       根据传递的端口号，从 response 应答链表中找出第一条匹配的数据
 * \param[in]   chPort     端口号
 */
static proxy_response_t *receive_response_object(uint8_t chSourcePort)
{
    int32_t nIndex = -1;

    nIndex = dlink_list_search(&s_tLiknResponseRecv, find_same_source_port_hook, &chSourcePort);

    return (0 <= nIndex) ? DLINK_LIST_POP(&s_tLiknResponseRecv, nIndex) : NULL;
}

/**
 * \brief       根据传递的端口号，从 response 应答链表删除端口号相同的应答
 * \param[in]   chPort     端口号
 */
static void clear_received_response_object(uint8_t chSourcePort)
{
    dlink_list_delete_equal(&s_tLiknResponseRecv, find_same_source_port_hook, &chSourcePort, destroy_response_object_hook);
}

/**
* \brief       获取操作锁
* \param[in]   chPort     端口号
*/
static critical_sector_t *get_operation_lock(uint8_t chPort)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (s_tOperationLockBuffer[i].chPort == chPort) {
            return &s_tOperationLockBuffer[i].tLock;
        }
    }

    return NULL;
}

/*************************** End of file ****************************/
