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

// ��raw_data ת���� proxy_request_t
def_simple_fsm(transform_request,
               def_params(
                   raw_data_t *ptData;
                   proxy_request_t *ptRequest;
               ))
end_def_simple_fsm(transform_request)

declare_fsm_initialiser(transform_request)
declare_fsm_implementation(transform_request, args(uint8_t chPort))

// ��raw_data ת���� proxy_response_t
def_simple_fsm(transform_response,
               def_params(
                   raw_data_t *ptData;
                   proxy_response_t *ptResponse;
               ))
end_def_simple_fsm(rawdata_transfotransform_responserm_response)

declare_fsm_initialiser(transform_response)
declare_fsm_implementation(transform_response, args(uint8_t chPort))

// ��ȡ�޻����Ӧ�𣬼�ֱ�ӴӶ˿ڻ�ȡ�豸����
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

// ��ȡ�޻����Ӧ�𣬼�ֱ�ӴӶ˿ڻ�ȡ�豸����
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

dlink_list_t s_tLinkRequestRecv;            // �����˿������������
dlink_list_t s_tLiknResponseRecv;           // �ӻ��˿�Ӧ���������

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
 * \brief       ���ն˿ڵ����ݣ����Ѹ�����ת��Ϊ request ����,
 *              �����뵽 request ����������
 * \param[in]   chPort     �˿ں�
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
 * \brief       ���ն˿ڵ����ݣ����Ѹ�����ת��Ϊ response Ӧ��,
 *              �����뵽 response Ӧ��������
 * \param[in]   chPort     �˿ں�
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
 * \brief       ת�����ݸ�MODBUS�ӻ�������ȡMODBUS�ӻ�Ӧ��
 * \param[in]   ptRequest     request ����ṹ��ָ��
 * \return      fsm_rt_t
 */
fsm_initialiser(read_and_write_operation, args(proxy_request_t *ptRequest))
init_body(
    const port_config_t *ptCfg = NULL;

    this.ptRequest = ptRequest;
    this.chPort = this.ptRequest->chPortDstBuf[0];  // ֻ������һ���˿�

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
* \brief       ��һ��ʱ���ڣ��Զ˿ڵĶ�д�������б�������ֹ�Զ˿ڵĲ�����д
* \param[in]   ptRequest     request ����ṹ��ָ��
* \param[in]   ptLock        ԭ�Ӳ���������
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
 * \brief ��ʼ�� request �� resposne �洢����
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
 * \brief  �� request ��������ȡ����һ������
 */
proxy_request_t *receive_request(void)
{
    return DLINK_LIST_POP_FIRST(&s_tLinkRequestRecv);
}

/**
 * \brief  Ѱ����ͬ�˿ںŵĹ��Ӻ���
 */
static int32_t find_same_source_port_hook(void *res, void *data)
{
    uint8_t *pchPort = (uint8_t *)data;
    proxy_response_t *ptResp = (proxy_response_t *)res;

    return ptResp->chPortSrc == *pchPort ? 0 : -1;
}

/**
 * \brief  �ͷ��ڴ�ռ�Ĺ��Ӻ���
 */
static int32_t destroy_response_object_hook(void *res)
{
    proxy_response_t *ptResp = (proxy_response_t *)res;

    RAW_DATA_FREE(ptResp->ptRaw);
    PROXY_RESPONSE_FREE(ptResp);
    return 0;
}

/**
 * \brief       ���ݴ��ݵĶ˿ںţ��� response Ӧ���������ҳ���һ��ƥ�������
 * \param[in]   chPort     �˿ں�
 */
static proxy_response_t *receive_response_object(uint8_t chSourcePort)
{
    int32_t nIndex = -1;

    nIndex = dlink_list_search(&s_tLiknResponseRecv, find_same_source_port_hook, &chSourcePort);

    return (0 <= nIndex) ? DLINK_LIST_POP(&s_tLiknResponseRecv, nIndex) : NULL;
}

/**
 * \brief       ���ݴ��ݵĶ˿ںţ��� response Ӧ������ɾ���˿ں���ͬ��Ӧ��
 * \param[in]   chPort     �˿ں�
 */
static void clear_received_response_object(uint8_t chSourcePort)
{
    dlink_list_delete_equal(&s_tLiknResponseRecv, find_same_source_port_hook, &chSourcePort, destroy_response_object_hook);
}

/**
* \brief       ��ȡ������
* \param[in]   chPort     �˿ں�
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
