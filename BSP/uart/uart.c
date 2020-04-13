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
*       uart.c *                                                     *
*                                                                    *
**********************************************************************
*/

#include ".\uart.h"
#include ".\xhal_uart.h"
#include "Driver_USART.h"
#include "cmsis_os2.h"

#define USART_EVENT_RECEIVE_COMPLETE   (1UL << 0)
#define USART_EVENT_RX_TIMEOUT         (1UL << 1)
#define USART_EVENT_RX_OVERFLOW        (1UL << 2)
#define USART_EVENT_TX_COMPLETE        (1UL << 3)

#define USART_EVENT_RECEIVE_ABOUT      (USART_EVENT_RECEIVE_COMPLETE | USART_EVENT_RX_TIMEOUT | USART_EVENT_RX_OVERFLOW)
#define USART_EVENT_ALL                (USART_EVENT_RECEIVE_ABOUT | USART_EVENT_TX_COMPLETE)

#define ARM_USART_EVENT_ALL            (ARM_USART_EVENT_TX_COMPLETE | ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RX_OVERFLOW)
#define ARM_USART_ERROR_EVENT_ALL      (ARM_USART_EVENT_TX_UNDERFLOW | ARM_USART_EVENT_RX_BREAK | ARM_USART_EVENT_RX_FRAMING_ERROR | ARM_USART_EVENT_RX_PARITY_ERROR)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    ARM_USART_SignalEvent_t fnSingalEventCB;
    osEventFlagsId_t        tEventFlagId;

    uart_func_t fnRecvCB;
    void       *pRecvCBParameter;
} usart_singal_handler_t;

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
extern const uart_mapping_t c_tUartMap[TOTAL_UART_NUM];

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static void usart_event_callback_num_0(uint32_t event);
static void usart_event_callback_num_1(uint32_t event);
static void usart_event_callback_num_2(uint32_t event);
static void usart_event_callback_num_3(uint32_t event);

static usart_singal_handler_t s_tSingalHandlerMap[TOTAL_UART_NUM] = {
    {.fnSingalEventCB = usart_event_callback_num_0},
    {.fnSingalEventCB = usart_event_callback_num_1},
    {.fnSingalEventCB = usart_event_callback_num_2},
//    {.fnSingalEventCB = usart_event_callback_num_3},
};

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
static int32_t uart_stopbits_transform(uint8_t chStopBit, uint32_t *pwReg);
static int32_t uart_parity_transform(uint8_t chParity, uint32_t *pwReg);
static int32_t uart_flow_control_transform(uint8_t chFlowControl, uint32_t *pwReg);

static int32_t get_map_pos(uint16_t hwPort)
{
    int32_t i = 0;
    for (i = 0; (i < TOTAL_UART_NUM) && (hwPort != c_tUartMap[i].chPort); i++);
    return TOTAL_UART_NUM == i ? XHAL_FAIL : i;
}

int32_t xhal_uart_init(uart_dev_t *ptDev)
{
    int32_t nRet, nPos;
    uint32_t wValue = 0, wTemp = 0;
    ARM_DRIVER_USART *ptDrv = NULL;
    usart_singal_handler_t *ptHandler = NULL;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    // Get ARM Driver
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;

    ptHandler = &s_tSingalHandlerMap[nPos];
    ptHandler->tEventFlagId = osEventFlagsNew(NULL);
    if (NULL == ptHandler->tEventFlagId) {
        return XHAL_FAIL;
    }

    wValue |= ARM_USART_MODE_ASYNCHRONOUS;
    wValue |= ARM_USART_DATA_BITS_8;    // 有校验位也是8位，与ST库不同

    nRet = uart_parity_transform(ptDev->tConfig.chParity, &wTemp);
    if (XHAL_OK != nRet) {
        return XHAL_FAIL;
    }
    wValue |= wTemp;

    nRet |= uart_stopbits_transform(ptDev->tConfig.chStopBits, &wTemp);
    if (XHAL_OK != nRet) {
        return XHAL_FAIL;
    }
    wValue |= wTemp;

    nRet |= uart_flow_control_transform(ptDev->tConfig.chFlowControl, &wTemp);
    if (XHAL_OK != nRet) {
        return XHAL_FAIL;
    }
    wValue |= wTemp;

    // TODO cmsis driver 第一次上电接收数据未启动DMA
    nRet = ptDrv->Initialize(ptHandler->fnSingalEventCB);
    nRet &= ptDrv->PowerControl(ARM_POWER_FULL);
    nRet &= ptDrv->Control(wValue, ptDev->tConfig.wBaudrate);
    nRet &= ptDrv->Control(ARM_USART_CONTROL_TX, 1);
    nRet &= ptDrv->Control(ARM_USART_CONTROL_RX, 1);

    return (ARM_DRIVER_OK == nRet) ? XHAL_OK : XHAL_FAIL;
}

int32_t xhal_uart_deinit(uart_dev_t *ptDev)
{
    int32_t nPos;
    ARM_DRIVER_USART *ptDrv = NULL;
    usart_singal_handler_t *ptHandler = NULL;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    // Get ARM Driver
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;
    ptHandler = &s_tSingalHandlerMap[nPos];

    ptDrv->Control(ARM_USART_ABORT_TRANSFER, 1);
    ptDrv->Uninitialize();

    osEventFlagsDelete(ptHandler->tEventFlagId);
    ptHandler->tEventFlagId = NULL;

    return XHAL_OK;
}

int32_t xhal_uart_send_in_dma_mode(uart_dev_t *ptDev, const void *pData, uint32_t wSize, uint32_t wTimeout)
{
    int32_t nPos;
    ARM_DRIVER_USART *ptDrv = NULL;
    usart_singal_handler_t *ptHandler = NULL;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    // Get ARM Driver
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;
    ptHandler = &s_tSingalHandlerMap[nPos];

    osEventFlagsClear(ptHandler->tEventFlagId, USART_EVENT_TX_COMPLETE);
    if (ARM_DRIVER_OK == ptDrv->Send(pData, wSize)) {
        osEventFlagsWait(ptHandler->tEventFlagId, USART_EVENT_TX_COMPLETE, osFlagsWaitAny, wTimeout);
        return XHAL_OK;
    }
    return XHAL_FAIL;
}

int32_t xhal_uart_recv_in_dma_mode(uart_dev_t *ptDev, uint8_t *pDst, uint32_t wBytes, uint32_t wTimeout)
{
    int32_t nPos;
    ARM_DRIVER_USART *ptDrv = NULL;
    usart_singal_handler_t *ptHandler = NULL;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    // Get ARM Driver
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;
    ptHandler = &s_tSingalHandlerMap[nPos];

    osEventFlagsClear(ptHandler->tEventFlagId, USART_EVENT_RECEIVE_ABOUT);
    ptDrv->Control(ARM_USART_ABORT_RECEIVE, 1); // abort uart receive
    ptDrv->Receive(pDst, wBytes);

    osEventFlagsWait(ptHandler->tEventFlagId, USART_EVENT_RECEIVE_ABOUT, osFlagsWaitAny, wTimeout);
    
    return (int32_t)ptDrv->GetRxCount();
}

uint32_t xhal_uart_get_rx_count(uart_dev_t *ptDev)
{
    int32_t nPos;
    ARM_DRIVER_USART *ptDrv = NULL;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;
    return (XHAL_FAIL != nPos) ? ptDrv->GetRxCount() : 0;
}

int32_t xhal_uart_is_send_completed(uart_dev_t *ptDev)
{
    int32_t nPos;
    ARM_DRIVER_USART *ptDrv = NULL;
    ARM_USART_STATUS tStatus;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    ptDrv = (ARM_DRIVER_USART *)c_tUartMap[nPos].ptUartPhy;
    tStatus = ptDrv->GetStatus();
    return (0 == tStatus.tx_busy) ? XHAL_OK : XHAL_FAIL;
}

int32_t xhal_uart_is_recv_completed(uart_dev_t *ptDev)
{
    int32_t nPos;
    usart_singal_handler_t* ptHandler = NULL;
    uint32_t wRetval = 0;

    ASSERT(NULL != ptDev);

    nPos = get_map_pos(ptDev->chPort);
    ptHandler = &s_tSingalHandlerMap[nPos];

    wRetval = osEventFlagsGet(ptHandler->tEventFlagId);
    if (USART_EVENT_RECEIVE_ABOUT & wRetval) {
        /*! clear receive complete event flag! */
        osEventFlagsClear(ptHandler->tEventFlagId, USART_EVENT_RECEIVE_ABOUT);
        return XHAL_OK;
    }
    return XHAL_FAIL;
}

static int32_t uart_parity_transform(uint8_t chParity, uint32_t *pwReg)
{
    switch (chParity) {
        case UART_NO_PARITY:
            *pwReg = ARM_USART_PARITY_NONE;
            break;

        case UART_ODD_PARITY:
            *pwReg = ARM_USART_PARITY_ODD;
            break;

        case UART_EVEN_PARITY:
            *pwReg = ARM_USART_PARITY_EVEN;
            break;

        default:
            return XHAL_FAIL;
    }

    return XHAL_OK;
}

static int32_t uart_stopbits_transform(uint8_t chStopBit, uint32_t *pwReg)
{
    switch (chStopBit) {
        case UART_STOP_BITS_1:
            *pwReg = ARM_USART_STOP_BITS_1;
            break;

        case UART_STOP_BITS_2:
            *pwReg = ARM_USART_STOP_BITS_2;
            break;

        default:
            return XHAL_FAIL;
    }

    return XHAL_OK;
}

static int32_t uart_flow_control_transform(uint8_t chFlowControl, uint32_t *pwReg)
{
    switch (chFlowControl) {
        case FLOW_CONTROL_DISABLED:
            *pwReg = ARM_USART_FLOW_CONTROL_NONE;
            break;

        case FLOW_CONTROL_CTS:
            *pwReg = ARM_USART_FLOW_CONTROL_CTS;
            break;

        case FLOW_CONTROL_RTS:
            *pwReg = ARM_USART_FLOW_CONTROL_RTS;
            break;
        case FLOW_CONTROL_CTS_RTS:
            *pwReg = ARM_USART_FLOW_CONTROL_RTS_CTS;

        default:
            return XHAL_FAIL;
    }

    return XHAL_OK;
}

// 串口错误触发
static void error_event_trigger(int32_t nPos, uint32_t wEvent)
{
    ARM_DRIVER_USART *ptDrv = c_tUartMap[nPos].ptUartPhy;

    ptDrv->Control(ARM_USART_ABORT_RECEIVE, 1);
    ptDrv->Receive(c_tUartMap[nPos].pchBuf, c_tUartMap[nPos].hwBufSize);
}

// 串口发送接收事件触发
static void event_triger(int32_t nPos, uint32_t wEvent)
{
    usart_singal_handler_t *ptHandler = &s_tSingalHandlerMap[nPos];

    if (ARM_USART_EVENT_RX_OVERFLOW & wEvent) {
        osEventFlagsSet(ptHandler->tEventFlagId, USART_EVENT_RX_OVERFLOW);
    }

    if (ARM_USART_EVENT_RECEIVE_COMPLETE & wEvent) {
        osEventFlagsSet(ptHandler->tEventFlagId, USART_EVENT_RECEIVE_COMPLETE);
    }

    if (ARM_USART_EVENT_RX_TIMEOUT & wEvent) {
        osEventFlagsSet(ptHandler->tEventFlagId, USART_EVENT_RX_TIMEOUT);
    }

    if (ARM_USART_EVENT_TX_COMPLETE & wEvent) {
        osEventFlagsSet(ptHandler->tEventFlagId, USART_EVENT_TX_COMPLETE);
    }
}

// 序列号为0的串口事件回调函数
static void usart_event_callback_num_0(uint32_t wEvent)
{
    if (ARM_USART_EVENT_ALL & wEvent) {
        event_triger(0, wEvent);
    }

    if (ARM_USART_ERROR_EVENT_ALL & wEvent) {
        error_event_trigger(0, ARM_USART_ERROR_EVENT_ALL & wEvent);
    }
}

// 序列号为1的串口事件回调函数
static void usart_event_callback_num_1(uint32_t wEvent)
{
    if (ARM_USART_EVENT_ALL & wEvent) {
        event_triger(1, wEvent);
    }

    if (ARM_USART_ERROR_EVENT_ALL & wEvent) {
        error_event_trigger(1, ARM_USART_ERROR_EVENT_ALL & wEvent);
    }
}

// 序列号为2的串口事件回调函数
static void usart_event_callback_num_2(uint32_t wEvent)
{
    if (ARM_USART_EVENT_ALL & wEvent) {
        event_triger(2, wEvent);
    }

    if (ARM_USART_ERROR_EVENT_ALL & wEvent) {
        error_event_trigger(2, ARM_USART_ERROR_EVENT_ALL & wEvent);
    }
}

// 序列号为3的串口事件回调函数
static void usart_event_callback_num_3(uint32_t wEvent)
{
    if (ARM_USART_EVENT_ALL & wEvent) {
        event_triger(3, wEvent);
    }

    if (ARM_USART_ERROR_EVENT_ALL & wEvent) {
        error_event_trigger(3, ARM_USART_ERROR_EVENT_ALL & wEvent);
    }
}
