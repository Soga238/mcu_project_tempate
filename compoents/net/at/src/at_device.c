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
*       at_device.c *                                                *
*                                                                    *
**********************************************************************
*/
#include "..\include\at_device.h"
#include "..\include\at_port.h"
#include "..\include\at_device_cmd.h"

#include "..\service\ring_buf\ring_buf.h"
#include "..\bsp\gpio\gpio.h"
#include "..\bsp\uart\uart.h"

#include <string.h>
#include <stdio.h>

/* Private define --------------------------------------------------*/
#define RECV_BUFFER_SIZE        256
#define URC_TABLE_SIZE          1

#define EXEC_REPETITIONS        10
#define RETRY_COUNT             3

def_simple_fsm(init_at_device,
               def_params(
                   uint8_t chErrorCount;
               ));
end_def_simple_fsm(init_at_device);

declare_fsm_implementation(init_at_device);
declare_fsm_initialiser(init_at_device);

/* Global variables ------------------------------------------------*/
uint8_t g_chBuffer[RECV_BUFFER_SIZE];
uint8_t g_chAtRecvBuffer[RECV_BUFFER_SIZE];

char_ring_t g_tRingBuf;

uart_dev_t g_tATUart = {
    .chPort = PORT_UART_GSM, .pPrivData = NULL,
    .tConfig = {
        115200,
        UART_NO_PARITY,
        DATA_WIDTH_8BIT,
        UART_STOP_BITS_1,
        FLOW_CONTROL_DISABLED,
        MODE_TX_RX
    },
};

/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
static bool s_bDeviceInitialized = false;
static bool s_bOnline = false;

static struct at_client *s_ptATClient;

static osThreadAttr_t thread_attr = {
    .stack_size = 384,
    .priority = osPriorityAboveNormal2,
};

static gpio_dev_t s_tGSMRunPin = {
    .chPort     =   PORT_GSM_RUN,
    .pPrivData  =   NULL
};

static gpio_dev_t s_tGSMPwrPin = {
    .chPort     =   PORT_GSM_PWR,
    .pPrivData  =   NULL
};

static gsm_param_t s_tGsmParam;

void recv_callback(struct at_client *client, const char *data, rt_size_t size);

static struct at_urc s_tUrcTable[URC_TABLE_SIZE] = {
    {
        .cmd_prefix = "+MSUB:",
        .cmd_suffix = "\r\n",
        .func = recv_callback
    },
};

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

static void gsm_uart_thread(void *argument)
{
    int32_t nRetval;
    for (;;) {
        nRetval = xhal_uart_recv_in_dma_mode(&g_tATUart, g_chAtRecvBuffer, sizeof(g_chAtRecvBuffer), WAIT_FOREVER);
        if (0 < nRetval) {
            ringbuf_put(&g_tRingBuf, g_chAtRecvBuffer, nRetval);
        } else {
            osDelay(50);
        }
    }
}

bool at_device_init(void)
{
    ringbuf_init(&g_tRingBuf, g_chBuffer, sizeof(g_chBuffer));

    xhal_gpio_init(&s_tGSMPwrPin);  // GSM电源和运行控制引脚
    xhal_gpio_init(&s_tGSMRunPin);

    if (XHAL_OK != xhal_uart_init(&g_tATUart)) {
        return false;
    }

    osThreadNew(gsm_uart_thread, NULL, &thread_attr);

    s_ptATClient = (struct at_client *)at_client_get(0);
    if (NULL == s_ptATClient) {
        return false;
    }

    /*! create client parser thread */
    if (RT_EOK != at_client_init(0, 256)) {
        return false;
    }

    at_obj_set_urc_table(s_ptATClient, s_tUrcTable, URC_TABLE_SIZE);

    return true;
}

WEAK void server_cmd_callback(const uint8_t *pchBuf, uint32_t wSize) {}

void recv_callback(struct at_client *client, const char *data, rt_size_t size)
{
    server_cmd_callback((uint8_t *)data, size);
}

int32_t at_device_pub_with_buf(uint8_t *pchBuf, uint16_t hwLength, uint32_t wTimeout)
{
    int8_t cRet = 0;
    struct at_response *ptResp = NULL;

    if (!s_bDeviceInitialized) {
        return 0;
    }

    ptResp = at_create_resp(128, 2, wTimeout);
    if (NULL == ptResp) {
        return 0;
    }

    if (RT_EOK == at_obj_exec_cmd_with_buf(s_ptATClient, ptResp, pchBuf, hwLength)) {
        cRet = 1;
    }

    at_delete_resp(ptResp);

    return cRet;
}

void at_device_restart(void)
{
    xhal_gpio_set_by_port(PORT_GSM_PWR, GPIO_LEVEL_LOW);
    xhal_gpio_set_by_port(PORT_GSM_RUN, GPIO_LEVEL_HIGH);
    osDelay(2500);
    xhal_gpio_set_by_port(PORT_GSM_PWR, GPIO_LEVEL_HIGH);
    xhal_gpio_set_by_port(PORT_GSM_RUN, GPIO_LEVEL_LOW);
}

bool at_device_connect_server(at_mqtt_info_t *ptInfo)
{
    int8_t i;

    ASSERT(NULL != ptInfo);
    if (!s_bDeviceInitialized) {
        return false;
    }

    for (i = 0; i < RETRY_COUNT; i++) {
        if (at_cmd_connect_onenet(s_ptATClient, ptInfo->pchUserName, ptInfo->pchClientID, ptInfo->pchPassword)) {
            break;
        }
    }

    return i < RETRY_COUNT;
}

int32_t at_device_check_mqtt(void)
{
    int8_t i;

    if (!s_bDeviceInitialized) {
        return -1;
    }

    for (i = 0; i <= RETRY_COUNT; i++) {
        if (at_cmd_get_work_state(s_ptATClient, &s_tGsmParam)) {
            break;
        } else if (RETRY_COUNT == i) {
            s_bOnline = false;
            return -1;
        }
    }

    for (i = 0; i <= RETRY_COUNT; i++) {
        if (at_cmd_get_mqttstate(s_ptATClient, &s_tGsmParam.wMqttState)) {
            break;
        } else if (RETRY_COUNT == i) {
            s_bOnline = false;
            return -1;
        }
    }

    ALOG_D("CSQ:%d CGAT:%d MQTT:%d", s_tGsmParam.wCSQSignal, s_tGsmParam.wCGATT, s_tGsmParam.wMqttState);
    if ((1 == s_tGsmParam.wMqttState) || (2 == s_tGsmParam.wMqttState)) {
        s_bOnline = true;
        return 0;
    }

    return -1;
}

bool at_device_check_online(void)
{
    return s_bOnline;
}

bool at_device_get_gsm_param(at_gsm_param_t *ptGsm)
{
    if (NULL != ptGsm) {
        ptGsm->pchCCID = s_tGsmParam.chCCIDBuf;
        ptGsm->pchVer = s_tGsmParam.chVerBuf;
        ptGsm->chCSQSignal = (uint8_t)s_tGsmParam.wCSQSignal;
        ptGsm->chMqttState = (uint8_t)s_tGsmParam.wMqttState;
        ptGsm->wCid = s_tGsmParam.wCellId;
        ptGsm->wLoc = s_tGsmParam.wLac;
        return true;
    }
    return false;
}

fsm_initialiser(init_at_device)
init_body()

fsm_implementation(init_at_device)
{
    def_states(REBOOT, PHY_CONNECT, GET_INFO, GET_STATE, GET_IP, GET_LOCATION)

    body_begin()

    on_start(
        this.chErrorCount = 0;
        s_bDeviceInitialized = false;
    )

    state(REBOOT) {
        at_device_restart();
        update_state_to(PHY_CONNECT)
    }

    state(PHY_CONNECT) {
        if (at_cmd_phy_connect(s_ptATClient)) {
            transfer_to(GET_INFO);
        }
    }

    state(GET_INFO) {
        if (at_cmd_get_sim_info(s_ptATClient, &s_tGsmParam)) {
            this.chErrorCount = 0;
            transfer_to(GET_STATE);
        } else if (++this.chErrorCount > EXEC_REPETITIONS) {
            transfer_to(REBOOT);
        } else {
            osDelay(1000);
        }
    }

    state(GET_STATE) {
        if (at_cmd_get_work_state(s_ptATClient, &s_tGsmParam)) {
            if ((1 == s_tGsmParam.wCGATT) && (s_tGsmParam.wCSQSignal > NORMAL_MINIMUM_CSQ)) {
                transfer_to(GET_LOCATION);
            } else {
                ALOG_I("CSQ %d CGATT %d", s_tGsmParam.wCSQSignal, s_tGsmParam.wCGATT);
                osDelay(1000);
            }
        } else if (++this.chErrorCount > EXEC_REPETITIONS) {
            transfer_to(REBOOT);
        } else {
            osDelay(1000);
        }
    }

    state(GET_LOCATION) {
        if (at_cmd_get_nptm(s_ptATClient, &s_tGsmParam)) {
            this.chErrorCount = 0;
            transfer_to(GET_IP);
        } else if (++this.chErrorCount > EXEC_REPETITIONS) {
            transfer_to(REBOOT);
        } else {
            osDelay(1000);
        }
    }

    state(GET_IP) {
        if (at_cmd_get_ip(s_ptATClient, &s_tGsmParam)) {
            s_bDeviceInitialized = true;
            fsm_cpl();
        } else if (++this.chErrorCount > EXEC_REPETITIONS) {
            transfer_to(REBOOT);
        } else {
            osDelay(1000);
        }
    }

    body_end()
}

/*************************** End of file ****************************/
