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
*       at_device_tcp.c *                                            *
*                                                                    *
**********************************************************************
*/
#include "..\include\at_device_tcp.h"
#include "..\include\at_device_tcp_cmd.h"
#include "..\include\at_port.h"

#include "..\service\ring_buf\ring_buf.h"
#include "..\bsp\gpio\gpio.h"
#include "..\bsp\uart\uart.h"

#define __PLOOC_CLASS_IMPLEMENT
#include "..\utilities\ooc.h"

#include <string.h>
#include <stdio.h>

/* Private define --------------------------------------------------*/
#define RECV_BUFFER_SIZE                2048    // 必须2的n次方
#define HANDLE_BUFFER_SIZE              1280
#define URC_TABLE_SIZE                  5
#define EXEC_REPETITIONS                3

#define EVENT_FLAG_MASK_OPEN_TCP        (1u)
#define EVENT_FLAG_MASK_OPEN_TCP_SUCC   (1u << 1)
#define EVENT_FLAG_MASK_OPEN_TCP_FAIL   (1u << 2)
#define EVENT_FLAG_MASK_SEND_DATA       (1u << 3)
#define EVENT_FLAG_MASK_SEND_SUCC       (1u << 4)
#define EVENT_FLAG_MASK_SEND_FAIL       (1u << 5)
#define EVENT_FLAG_MASK_RECV_CPL        (1u << 6)
#define EVENT_FLAG_MASK_TCP_CLOSED      (1u << 7)

def_simple_fsm(init_device,
               def_params(
                   struct at_client *ptClient;
                   gsm_param_t *ptGsmParam;
                   uint8_t chRetryCount;
               ));
end_def_simple_fsm(init_device)
declare_fsm_initialiser(init_device, args(struct at_client *ptClient, gsm_param_t *ptGsmParam))
declare_fsm_implementation(init_device)

def_simple_fsm(check_state,
               def_params(
                   struct at_client *ptClient;
                   gsm_param_t *ptGsmParam;
                   uint8_t chRetryCount;
               ));
end_def_simple_fsm(check_state)
declare_fsm_initialiser(check_state, args(struct at_client *ptClient, gsm_param_t *ptGsmParam))
declare_fsm_implementation(check_state)

def_simple_fsm(process_event,
               def_params(
                   struct at_client *ptClient;
                   bool *pbIsOnline;
               ))
end_def_simple_fsm(process_event)
declare_fsm_initialiser(process_event, args(struct at_client *ptClient, bool *pbIsOnline))
declare_fsm_implementation(process_event, args(bool *pbIsRequestCheckStatus))

def_simple_fsm(connect_to_server,
               def_params(
                   struct at_client *ptClient;
                   gsm_param_t *ptGsmParam;
                   bool *pbIsDevInited;
                   bool *pbIsOnline;
                   bool bIsRequestCheckTcp;
                   fsm(init_device) tDevInit;
                   fsm(check_state) tDevCheck;
                   fsm(process_event) tProcessEvent;
               ))
end_def_simple_fsm(connect_to_server)
declare_fsm_initialiser(connect_to_server)
declare_fsm_implementation(connect_to_server)

/* Global variables ------------------------------------------------*/
uint8_t s_chRingBuffer[RECV_BUFFER_SIZE];
char_ring_t g_tRingBuf;

/*! TODO：不知原因，使用ARM库，高波特率下丢失数据 */
uart_dev_t g_tATUart = {
    .chPort = PORT_UART_2, .pPrivData = NULL,
    .tConfig = {
        38400,
        UART_NO_PARITY,
        DATA_WIDTH_8BIT,
        UART_STOP_BITS_1,
        FLOW_CONTROL_DISABLED,
        MODE_TX_RX
    },
};

/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
uint8_t s_chHandleBuffer[HANDLE_BUFFER_SIZE];
static bool s_bDeviceInitialized = false;
static bool s_bIsOnline = false;

static gsm_param_t s_tGsmParam;
static struct at_client *s_ptATClient;

static osThreadAttr_t thread_attr = {
    .stack_size = 640,
    .priority = osPriorityNormal7,
};

static osEventFlagsId_t s_tEventFlagId;

static struct {
    const uint8_t *pchBuffer;
    int32_t nBufferSize;
    uint32_t wTimeout;
} s_tSendInfo;

static struct {
    uint8_t *pchBuffer;
    int32_t nBufferSize;
    int32_t nRecvSize;
} s_tRecvInfo;

static fsm(connect_to_server) s_tRun;

/*
    TODO: 由于AIR208 TCP模式下接收的数据不会自动添加 \r\n 后缀， 无法使用AT库的尾缀判断
    服务器主动在尾部添加 \r\n 实际数据减要去 \r\n 占用的字符长度
*/
static void recv_callback(struct at_client *client, const char *data, rt_size_t size);
static void unused_callback(struct at_client *client, const char *data, rt_size_t size);
static void tcp_closed_callback(struct at_client *client, const char *data, rt_size_t size);

static struct at_urc s_tUrcTable[URC_TABLE_SIZE] = {
    {
        .cmd_prefix = "+IPD",
        .cmd_suffix = NULL,             // TCP返回的固件不能依靠尾缀\r\n判断结束
        .func = recv_callback
    }, {
        .cmd_prefix = "CLOSED",         // 排除URC数据干扰
        .cmd_suffix = "\r\n",
        .func = tcp_closed_callback     // 触发TCP关闭事件
    }, {
        .cmd_prefix = "SMS Ready",      // 排除URC数据干扰
        .cmd_suffix = "\r\n",
        .func = unused_callback
    }, {
        .cmd_prefix = "+CPIN: READY",   // 排除URC数据干扰
        .cmd_suffix = "\r\n",
        .func = unused_callback
    }, {
        .cmd_prefix = "Call Ready",     // 排除URC数据干扰
        .cmd_suffix = "\r\n",
        .func = unused_callback
    }
};

/* Private function prototypes -------------------------------------*/
static void __at_device_restart(void);
static int32_t __at_device_tcp_send(const uint8_t *pchBuf, uint16_t hwLength);

/* Private functions -----------------------------------------------*/
void gsm_thread(void *argument)
{
    init_fsm(connect_to_server, &s_tRun);
    for (;;) {
        call_fsm(connect_to_server, &s_tRun);
        osDelay(10);
    }
}

bool at_device_init(void)
{
    if (0 == ringbuf_init(&g_tRingBuf, s_chRingBuffer, sizeof(s_chRingBuffer))) {
        return false;
    }

    if (XHAL_OK != xhal_uart_init(&g_tATUart)) {
        return false;
    }

    s_ptATClient = (struct at_client *)at_client_get(0);
    if (NULL == s_ptATClient) {
        return false;
    }

    /*! create client parser thread */
    if (RT_EOK != at_client_init(0, 256)) {
        return false;
    }

    s_tEventFlagId = osEventFlagsNew(NULL);
    osThreadNew(gsm_thread, NULL, &thread_attr);
    at_obj_set_urc_table(s_ptATClient, s_tUrcTable, URC_TABLE_SIZE);

    return true;
}

bool at_device_open_tcp(void)
{
    if (s_bDeviceInitialized && !s_bIsOnline) {
        osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_OPEN_TCP);
    }
    return s_bIsOnline;
}

bool at_device_close_tcp(void)
{
    return true;
}

int32_t at_device_tcp_send(const uint8_t *pchBuf, uint16_t hwLength, uint32_t wTimeout)
{
    uint32_t wEvent;

    if (s_bDeviceInitialized && s_bIsOnline) {
        s_tSendInfo.pchBuffer = pchBuf;
        s_tSendInfo.nBufferSize = hwLength;
        osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_SEND_DATA);
        wEvent = osEventFlagsWait(s_tEventFlagId,
                                  EVENT_FLAG_MASK_SEND_FAIL | EVENT_FLAG_MASK_SEND_SUCC,
                                  osFlagsWaitAny, wTimeout);
        if (wEvent & EVENT_FLAG_MASK_SEND_SUCC) {
            return hwLength;
        }
    }
    return 0;
}

int32_t at_device_tcp_recv(uint8_t *pchBuf, uint16_t hwLength, uint32_t wTimeout)
{
    uint32_t wEvent;

    s_tRecvInfo.pchBuffer = pchBuf;
    s_tRecvInfo.nBufferSize = hwLength;

    wEvent = osEventFlagsGet(s_tEventFlagId);
    if (EVENT_FLAG_MASK_RECV_CPL & wEvent) {
        osEventFlagsClear(s_tEventFlagId, EVENT_FLAG_MASK_RECV_CPL);
        return s_tRecvInfo.nRecvSize;
    }
    return 0;
}

static void recv_callback(struct at_client *client, const char *data, rt_size_t size)
{
    int32_t nLength = 0;
    uint8_t *pchBuf;

    /*! TODO： BIN数据是二进制文件不能依靠\r\n判断URC数据， 手动处理*/
    /*! 将缓冲区内的所有数据提取出来 */
    int32_t nReadLength = ringbuf_get(&g_tRingBuf, s_chHandleBuffer, sizeof(s_chHandleBuffer));
    if (1 != sscanf((char *)s_chHandleBuffer, ",%d:", &nLength)) {
        ALOG_W("IPD sscanf failed\r\n");
        return;
    }

    pchBuf = (uint8_t *)strstr((char *)s_chHandleBuffer, ":") + 1;
    if (((int32_t)(pchBuf - s_chHandleBuffer) + nLength) != nReadLength) {
        ALOG_W("IPD incomplete\r\n");
        return;
    }

    if ((nLength > 2) && (NULL != s_tRecvInfo.pchBuffer)) {
        pchBuf = (uint8_t *)strstr((char *)s_chHandleBuffer, ":") + 1;
        s_tRecvInfo.nRecvSize = MIN(nLength - 2, s_tRecvInfo.nBufferSize);
        memcpy(s_tRecvInfo.pchBuffer, pchBuf, s_tRecvInfo.nRecvSize);
        osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_RECV_CPL);
    }
}

static void tcp_closed_callback(struct at_client *client, const char *data, rt_size_t size)
{
    osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_TCP_CLOSED);
}

fsm_initialiser(connect_to_server)
init_body(
    this.ptClient = s_ptATClient;
    this.ptGsmParam = &s_tGsmParam;
    this.pbIsDevInited = &s_bDeviceInitialized;
    this.pbIsOnline = &s_bIsOnline;
)

fsm_implementation(connect_to_server)
{
    def_states(INIT_DEVICE, PROCESS_EVENT, CEHCK_STATUS)
    fsm_rt_t tRetval;

    body_begin()

    on_start(
        *this.pbIsDevInited = false;
        *this.pbIsOnline = false;
        this.bIsRequestCheckTcp = false;
        init_fsm(init_device, &this.tDevInit, args(this.ptClient, this.ptGsmParam));
    )

    state(INIT_DEVICE) {
        tRetval = call_fsm(init_device, &this.tDevInit);
        if (fsm_rt_cpl == tRetval) {
            *this.pbIsDevInited = true;
            init_fsm(check_state, &this.tDevCheck, args(this.ptClient, this.ptGsmParam));
            init_fsm(process_event, &this.tProcessEvent, args(this.ptClient, this.pbIsOnline));
            transfer_to(PROCESS_EVENT);
        }  else {
            osDelay(1000);
            if (fsm_rt_err == tRetval) {
                fsm_report(fsm_rt_err);
            }
        }
    }

    state(PROCESS_EVENT) {
        if (fsm_rt_cpl == call_fsm(process_event, &this.tProcessEvent, args(&this.bIsRequestCheckTcp))) {
            if (this.bIsRequestCheckTcp) {
                transfer_to(CEHCK_STATUS);
            }
        }
    }

    state(CEHCK_STATUS) {
        tRetval = call_fsm(check_state, &this.tDevCheck);
        if (fsm_rt_cpl == tRetval) {
            if (0 == this.ptGsmParam->wTcpState) {
                osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_TCP_CLOSED);
            }
            ALOG_D("CSQ:%d TCP:%d\r\n", this.ptGsmParam->wCSQSignal, this.ptGsmParam->wTcpState);
            transfer_to(PROCESS_EVENT);
        } else if (fsm_rt_err == tRetval) {
            fsm_report(fsm_rt_err);
        }
    }

    body_end()
}

fsm_initialiser(process_event, args(struct at_client *ptClient, bool *pbIsOnline))
init_body(
    this.ptClient = ptClient;
    this.pbIsOnline = pbIsOnline;
)

fsm_implementation(process_event, args(bool *pbIsRequestCheckStatus))
{
    def_states(CHECK_EVENT, OPEN_TCP, SEND_DATA, CLOSE_TCP)
    uint32_t wEvent;

    body_begin()

    on_start(
        *pbIsRequestCheckStatus = false;
    )

    state(CHECK_EVENT) {
        wEvent = osEventFlagsGet(s_tEventFlagId);
        if (wEvent & EVENT_FLAG_MASK_TCP_CLOSED) {
            *this.pbIsOnline = false;
            osEventFlagsClear(s_tEventFlagId, EVENT_FLAG_MASK_TCP_CLOSED);
        } else if (wEvent & EVENT_FLAG_MASK_OPEN_TCP) {
            update_state_to(OPEN_TCP);
        } else if (wEvent & EVENT_FLAG_MASK_SEND_DATA) {
            update_state_to(SEND_DATA);
        } else {
            fsm_cpl();
        }
    }

    state(OPEN_TCP) {
        if (at_cmd_connect_to_server(this.ptClient)) {
            *this.pbIsOnline = true;
            ALOG_I("connect server succ\r\n");
        } else {
            *pbIsRequestCheckStatus = true;
        }
        osEventFlagsClear(s_tEventFlagId, EVENT_FLAG_MASK_OPEN_TCP);
        transfer_to(CHECK_EVENT);
    }

    state(SEND_DATA) {
        if (0 < __at_device_tcp_send(s_tSendInfo.pchBuffer, s_tSendInfo.nBufferSize)) {
            osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_SEND_SUCC);
        } else {
            *pbIsRequestCheckStatus = true;
            osEventFlagsSet(s_tEventFlagId, EVENT_FLAG_MASK_SEND_FAIL);
            ALOG_W("SEND FAILED\r\n");
        }
        osEventFlagsClear(s_tEventFlagId, EVENT_FLAG_MASK_SEND_DATA);
        transfer_to(CHECK_EVENT);
    }

    state(CLOSE_TCP) {
        transfer_to(CHECK_EVENT);
    }

    body_end()
}

fsm_initialiser(init_device, args(struct at_client *ptClient, gsm_param_t *ptGsmParam))
init_body(
    this.ptClient = ptClient;
    this.ptGsmParam = ptGsmParam;
)

fsm_implementation(init_device)
{
    def_states(REBOOT, PHY_CONNECT, GET_INFO, GET_STATE, GET_IP, SET_IP_HEAD_MODE, SET_TCP_ECHO_MODE)

    body_begin()

    on_start(
        this.chRetryCount = 0;
    )

    state(REBOOT) {
        __at_device_restart();
        update_state_to(PHY_CONNECT);
    }

    state(PHY_CONNECT) {
        if (at_cmd_phy_connect(this.ptClient)) {
            update_state_to(GET_INFO);
        } else {
            fsm_report(fsm_rt_err);
        }
    }

    state(GET_INFO) {
        if (at_cmd_get_sim_info(this.ptClient, this.ptGsmParam)) {
            this.chRetryCount = 0;
            ALOG_I("%s %s\r\n", this.ptGsmParam->chCCIDBuf, this.ptGsmParam->chVerBuf);
            update_state_to(GET_STATE);
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    state(GET_STATE) {
        if (at_cmd_get_work_state(this.ptClient, this.ptGsmParam)) {
            this.chRetryCount = 0;
            ALOG_D("CSQ %d CGATT %d\r\n", this.ptGsmParam->wCSQSignal, this.ptGsmParam->wCGATT);
            if ((1 == this.ptGsmParam->wCGATT) && (15 <= this.ptGsmParam->wCSQSignal)) {
                update_state_to(GET_IP);
            }
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    state(GET_IP) {
        if (at_cmd_get_ip(this.ptClient, this.ptGsmParam)) {
            this.chRetryCount = 0;
            update_state_to(SET_IP_HEAD_MODE);
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    state(SET_IP_HEAD_MODE) {
        if (at_cmd_set_ip_head_mode(this.ptClient, 1)) {
            this.chRetryCount = 0;
            update_state_to(SET_TCP_ECHO_MODE);
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    state(SET_TCP_ECHO_MODE) {
        if (at_cmd_set_tcp_echo_mode(this.ptClient, 0)) {
            fsm_cpl();
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    body_end()
}

fsm_initialiser(check_state, args(struct at_client *ptClient, gsm_param_t *ptGsmParam))
init_body(
    this.ptClient = ptClient;
    this.ptGsmParam = ptGsmParam;
)

fsm_implementation(check_state)
{
    def_states(CEHCK_WORK_STATE, CHECK_TCP_STATE)

    body_begin()

    on_start(
        this.chRetryCount = 0;
    )

    state(CEHCK_WORK_STATE) {
        if (at_cmd_get_work_state(this.ptClient, this.ptGsmParam)) {
            this.chRetryCount = 0;
            update_state_to(CHECK_TCP_STATE);
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    state(CHECK_TCP_STATE) {
        if (at_cmd_get_tcp_state(this.ptClient, this.ptGsmParam)) {
            fsm_cpl();
        } else if (++this.chRetryCount > EXEC_REPETITIONS) {
            fsm_report(fsm_rt_err);
        }
    }

    body_end()
}

static int32_t __at_device_tcp_send(const uint8_t *pchBuf, uint16_t hwLength)
{
    int32_t nLength = -1;
    struct at_response *ptResp;

    ptResp = at_create_resp(64, 1, 1000);
    if (NULL == ptResp) {
        return -1;
    }

    // 方便调试
    //ringbuf_reset(&g_tRingBuf);

    // 等待OD 0A
    if (RT_EOK != at_obj_exec_cmd(s_ptATClient, ptResp, "AT+CIPSEND=%d\r\n", hwLength)) {
        goto __fail;
    }

    at_resp_set_info(ptResp, 64, 2, 6000);  // 静态内存

    if (RT_EOK == at_obj_exec_cmd(s_ptATClient, ptResp, "%s", (char *)pchBuf)) {
        if (NULL == at_resp_get_line_by_kw(ptResp, "SEND OK")) {
            goto __fail;
        }
    } else {
        goto __fail;
    }

    nLength = hwLength;
__fail:
    at_delete_resp(ptResp);
    return nLength;
}

static void __at_device_restart(void)
{
    xhal_gpio_set_by_port(PORT_GSM_PWR, GPIO_LEVEL_LOW);
    xhal_gpio_set_by_port(PORT_GSM_RUN, GPIO_LEVEL_HIGH);
    osDelay(2000);
    xhal_gpio_set_by_port(PORT_GSM_PWR, GPIO_LEVEL_HIGH);
    xhal_gpio_set_by_port(PORT_GSM_RUN, GPIO_LEVEL_LOW);
    osDelay(1000);
}

static void unused_callback(struct at_client *client, const char *data, rt_size_t size)
{
    // do nothing
}

/*************************** End of file ****************************/
