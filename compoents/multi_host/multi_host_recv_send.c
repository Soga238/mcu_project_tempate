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
*       multi_host_recv_send.c *                                     *
*                                                                    *
**********************************************************************
*/
#include "./multi_host_recv_send.h"
#include "./multi_host_port.h"
#include "./multi_host_param.h"
#include "./dlink/dlink.h"

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/

// �������ж˿ڵ�����
def_simple_fsm(receive_data,
               def_params(
                   raw_data_t *ptData;
                   uint8_t chCounter;
                   uint8_t chPort;
                   uint8_t chBuffer[RAW_DATA_BUF_SIZE];
                   uint16_t hwSize;
               ))
end_def_simple_fsm(receive_data)
declare_fsm_implementation(receive_data);
declare_fsm_initialiser(receive_data);

// ���͵��˿ڵ�����
def_simple_fsm(send_port_data,
               def_params(
                   raw_data_t *ptData;
                   uint8_t chPort;
               ))
end_def_simple_fsm(send_port_data)
declare_fsm_implementation(send_port_data);
declare_fsm_initialiser(send_port_data, args(uint8_t chPort))

// �������ж˿ڵ�����
def_simple_fsm(send_data,
               def_params(
                   raw_data_t *ptData;
                   fsm(send_port_data) tBuf[TOTAL_PORT_NUM];
               ))
end_def_simple_fsm(send_data)
declare_fsm_implementation(send_data);
declare_fsm_initialiser(send_data);

// ת������
def_simple_fsm(forward_raw_data,
               def_params(
                   raw_data_t *ptData;
                   uint8_t chDstPort;
                   const uint8_t *pchBuf;
                   uint16_t hwSize;
                   critical_sector_t *ptLock;
               ))
declare_fsm_initialiser(forward_raw_data,
                        args(
                            uint8_t chDstPort,
                            const uint8_t *pchBuf,
                            uint16_t hwSize
                        ))
declare_fsm_implementation(forward_raw_data)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static struct port_lock_t {
    uint8_t chPort;
    critical_sector_t tLock;
} s_tLockBuffer[TOTAL_PORT_NUM];    // ÿ���˿ڶ�Ӧ�ķ�����

dlink_list_t s_tLinkRawDataRecv;    // �˿�ԭʼ���ݽ��մ洢�������������ӻ�
dlink_list_t s_tLinkRawDataSend;    // �˿�ԭʼ���ݷ��ʹ洢�������������ӻ�

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
static raw_data_t *create_raw_data_object(uint8_t chPort, const uint8_t *pchBuf, uint16_t hwBufSize);
static critical_sector_t *get_lock(uint8_t chPort);
static int32_t find_same_raw_data_object_hook(void *res, void *data);
static int32_t find_same_port_hook(void *res, void *data);

/*********************************************************************
*
*       Function Implention
*
**********************************************************************
*/

/**
 * \brief   ��ʼ�����ݽ�����������ݷ�������
 */

void multi_host_recv_send_init(void)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        s_tLockBuffer[i].chPort = g_chPortBuf[i];
        INIT_CRITICAL_SECTOR(&s_tLockBuffer[i].tLock);
    }

    dlink_list_init(&s_tLinkRawDataRecv);
    dlink_list_init(&s_tLinkRawDataSend);

}

/**
 * \brief   �����˿ڽ��պ������յ����ݲ��뵽���ݽ���������
 * \return  fsm_rt_t
 */
fsm_initialiser(receive_data)
init_body()

fsm_implementation(receive_data)
{
    def_states(CHEKC_PORT, CHECK_RECEIVE, MALLOC_DATA_BUF, SAVE_DATA)
    body_begin()

    on_start(
        this.chCounter = 0;
        this.ptData = NULL;
    )

    state(CHEKC_PORT) {
        if (RAW_DATA_FREE_SIZE() <= 2) { // ���Ʊ����ⲿ���ݵĸ���
            fsm_on_going();
        }

        if (this.chCounter < TOTAL_PORT_NUM) {
            this.chPort = g_chPortBuf[this.chCounter++];
            update_state_to(CHECK_RECEIVE)
        } else {
            fsm_cpl();
        }
    }

    state(CHECK_RECEIVE) {
        this.hwSize = proxy_recv(this.chPort, this.chBuffer, RAW_DATA_BUF_SIZE);
        if ((0 == this.hwSize) || (RAW_DATA_BUF_SIZE < this.hwSize)) {
            update_state_to(CHEKC_PORT);
        }

        if (check_crc16(this.chBuffer, this.hwSize)) {
            update_state_to(MALLOC_DATA_BUF);
        } else {
            update_state_to(CHEKC_PORT);
        }
    }

    state(MALLOC_DATA_BUF) {
        this.ptData = create_raw_data_object(this.chPort, this.chBuffer, this.hwSize);
        if (NULL != this.ptData) {
            update_state_to(SAVE_DATA);
        }
    }

    state(SAVE_DATA) {
        if (0 == DLINK_LIST_INSERT_LAST(&s_tLinkRawDataRecv, this.ptData)) {
            MLOG_RAW("[recv_data] from port[%d] 0x%08x\r\n", this.ptData->chPort, this.ptData);
            this.ptData = NULL;
            update_state_to(CHEKC_PORT);
        }
    }

    body_end()
}

/**
 * \brief       �����ݷ���������ȡ�����ݲ���������
 * \return      fsm_rt_t
 */
fsm_initialiser(send_data)
init_body({
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++)
    {
        init_fsm(send_port_data, &this.tBuf[i], args(g_chPortBuf[i]));
    }
})

fsm_implementation(send_data)
{
    def_states(SEND_DATA)
    body_begin()

    state(SEND_DATA) { //-V729
        for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
            call_fsm(send_port_data, &this.tBuf[i]);
        }
    }

    body_end()
}

/**
* \brief       �����ݷ���������ȡ�����ݲ���������
* \return      fsm_rt_t
*/
fsm_initialiser(send_port_data, args(uint8_t chPort))
init_body(
    this.chPort = chPort;
)

fsm_implementation(send_port_data)
{
    def_states(CHECK_DATA_LIST, SEND_DATA, CHECK_SEND_COMPLETE)

    int32_t nIndex;
    body_begin()

    state(CHECK_DATA_LIST) { //-V729
        nIndex = dlink_list_search(&s_tLinkRawDataSend, find_same_port_hook, &this.chPort);
        if (0 <= nIndex) {
            this.ptData = DLINK_LIST_POP(&s_tLinkRawDataSend, nIndex);
            if (NULL != this.ptData) {
                update_state_to(SEND_DATA);
            }
        } else {
            fsm_cpl();
        }
    }

    state(SEND_DATA) {
        MLOG_RAW("[send_data] to port[%d] 0x%08x\r\n", this.ptData->chPort, this.ptData);
        proxy_send(this.ptData->chPort, this.ptData->chBuf, this.ptData->hwBufSize);
        update_state_to(CHECK_SEND_COMPLETE);
    }

    state(CHECK_SEND_COMPLETE) {
        if (0 == proxy_check_send_complete(this.ptData->chPort)) {
            RAW_DATA_FREE(this.ptData);
            transfer_to(CHECK_DATA_LIST);
        }
    }

    body_end()
}

/**
 * \brief       ���ݶ˿ں�ת������
 * \param[in]   chDstPort   Ŀ��˿ں�
 * \param[in]   pchBuf      �ֽ�����ͷָ��
 * \param[in]   hwSize      �ֽ����ݳ���
 * \return      fsm_rt_t
 */
fsm_initialiser(forward_raw_data, args(uint8_t chDstPort, const uint8_t *pchBuf, uint16_t hwSize))
init_body(
    this.chDstPort = chDstPort;
    this.pchBuf = pchBuf;
    this.hwSize = hwSize;
    this.ptLock = get_lock(this.chDstPort);
)

fsm_implementation(forward_raw_data)
{
    def_states(MALLOC_DATA, PROTECT_OPERATION, SAVE_DATA, WAIT_SEND_OK)
    body_begin()

    state(MALLOC_DATA) { //-V729
        this.ptData = create_raw_data_object(this.chDstPort, this.pchBuf, this.hwSize);
        if (NULL != this.ptData) {
            update_state_to(PROTECT_OPERATION);
        }
    }

    state(PROTECT_OPERATION) {
        if (ENTER_CRITICAL_SECTOR(this.ptLock)) {
            update_state_to(SAVE_DATA);
        } else {
            SYSLOG_RAW("poll send lock failed\r\n");
        }
    }

    state(SAVE_DATA) {
        if (0 == DLINK_LIST_INSERT_LAST(&s_tLinkRawDataSend, this.ptData)) {
            transfer_to(WAIT_SEND_OK);
        }
    }

    state(WAIT_SEND_OK) { //-V729
        if (-1 == dlink_list_search(&s_tLinkRawDataSend, find_same_raw_data_object_hook, this.ptData)) {
            LEAVE_CRITICAL_SECTOR(this.ptLock);
            fsm_cpl();
        }
    }

    body_end()
}

/**
 * \brief       ����ͬһ������Ĺ��Ӻ���
 * \param[in]   res         �����ڴ��ַ
 * \param[in]   data        �����ڴ��ַ
 * \return      -1��ʧ�ܣ�
 */
static int32_t find_same_raw_data_object_hook(void *res, void *data)
{
    return res == data ? 0 : -1;
}

/**
 * \brief       ���ݶ˿ںŲ��Ҷ���Ĺ��Ӻ���
 * \param[in]   res         �����ڴ��ַ
 * \param[in]   data        �����ڴ��ַ
 * \return      -1��ʧ�ܣ�
 */
static int32_t find_same_port_hook(void *res, void *data)
{
    uint8_t *pchPort = (uint8_t *)data;
    raw_data_t *ptRawData = (raw_data_t *)res;

    return ptRawData->chPort == *pchPort ? 0 : -1;
}

/**
 * \brief       ����Դ�˿ںŴ����ݽ��������л�ȡ����
 * \param[in]   chPort       Դ�˿ں�
 * \return      NULL��ʧ�ܣ�
 */
raw_data_t *receive_raw_data_by_source_port(uint8_t chPort)
{
    uint8_t chNewPort = chPort;
    int32_t nIndex = -1;

    nIndex = dlink_list_search(&s_tLinkRawDataRecv, find_same_port_hook, &chNewPort);
    if (0 <= nIndex) {
        return DLINK_LIST_POP(&s_tLinkRawDataRecv, nIndex);
    }
    return NULL;
}

/**
 * \brief       �ͷ� raw_data_t ������Դ�Ĺ��Ӻ���
 * \param[in]   res         �����ڴ��ַ
 * \return      -1��ʧ�ܣ�
 */
static int32_t destroy_raw_data_object_hook(void *res)
{
    raw_data_t *ptRawData = (raw_data_t *)res;

    RAW_DATA_FREE(ptRawData);
    return 0;
}

/**
 * \brief       �����ݽ��������������ͬ�˿ںŵĶ���
 * \param[in]   chPort         Դ�˿ں�
 * \return      -1��ʧ�ܣ�
 */
void clear_received_raw_data_object(uint8_t chPort)
{
    dlink_list_delete_equal(&s_tLinkRawDataRecv, find_same_port_hook, &chPort, destroy_raw_data_object_hook);
}

/**
 * \brief       �����յ�������ת��Ϊ raw_data_t ���ͱ�������
 * \param[in]   chPort         Դ�˿ں�
 * \param[in]   pchBuf         �ֽ�����ͷָ��
 * \param[in]   hwBufSize      �ֽ����鳤��
 * \return      -1��ʧ�ܣ�
 */
static raw_data_t *create_raw_data_object(uint8_t chPort, const uint8_t *pchBuf, uint16_t hwBufSize)
{
    raw_data_t *ptRawData = (raw_data_t *)RAW_DATA_MALLOC();

    if (NULL != ptRawData) {
        ptRawData->chPort = chPort;
        memcpy(ptRawData->chBuf, pchBuf, MIN(hwBufSize, RAW_DATA_BUF_SIZE));
        ptRawData->hwBufSize = hwBufSize;
    }

    return ptRawData;
}

/**
* \brief       ��ȡ�˿ڷ�����
* \param[in]   chPort         Դ�˿ں�
* \return      -1��ʧ�ܣ�
*/
static critical_sector_t *get_lock(uint8_t chPort)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (s_tLockBuffer[i].chPort == chPort) {
            return &s_tLockBuffer[i].tLock;
        }
    }

    return NULL;
}

/*************************** End of file ****************************/
