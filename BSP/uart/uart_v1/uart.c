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
#include "..\..\hal\mcu\stm32f1xx\Inc\stm32f1xx_hal_conf.h"

#include "cmsis_os2.h"
#include <string.h>

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define __HAL_DMA_SET_COUNTER(__HANDLE__, __COUNT__) ((__HANDLE__)->Instance->CNDTR = __COUNT__)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
    UART_HandleTypeDef  tHandle;
    uint8_t            *pchRxBuf;
    uint16_t            hwRxBufSize;

    uint16_t            hwRecvCount;
    uint16_t            hwRecvCountLast;
    uint16_t            hwTimeCount;

    uart_func_t         fnRecvCB;
    void *              pRecvCBParameter;

    uart_func_t         fnSendCB;
    void *              pSendCBParameter;

    /*! os compoent */
    osSemaphoreId_t     tTxSem;
    osSemaphoreId_t     tRxSem;
    osMutexId_t         tTxMutexLock;
    osMutexId_t         tRxMutexLock;
    osTimerId_t         tTimer;

    uint8_t             chState;
} stm_uart_t;

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

static stm_uart_t s_tSTUartMap[TOTAL_UART_NUM];

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
static int32_t uart_datawidth_transform(uint8_t chDataWidth, uint32_t *pwReg);
static int32_t uart_parity_transform(uint8_t chParity, uint32_t *pwReg);
static int32_t uart_stopbits_transform(uint8_t chStopBit, uint32_t *pwReg);
static int32_t uart_mode_transform(uint8_t chMode, uint32_t *pwReg);
static int32_t uart_flow_control_transform(uint8_t chFlowControl, uint32_t *pwReg);

static int32_t get_map_pos(uint16_t hwPort)
{
    uint8_t i = 0;

    for (i = 0; (i < TOTAL_UART_NUM) && (hwPort != c_tUartMap[i].chPort); i++);
    return TOTAL_GPIO_NUM == hwPort ? XHAL_FAIL : i;
}

static stm_uart_t* get_map_stm_uart(int32_t nPos)
{
    return (0 <= nPos && nPos < TOTAL_GPIO_NUM) ? &s_tSTUartMap[nPos] : NULL;
}

int32_t xhal_uart_init(uart_dev_t *ptDev)
{
    int32_t nPos = 0;
    int32_t nRet = 0;
    UART_HandleTypeDef *ptHandle = NULL;
    stm_uart_t* ptSTUart = NULL;

    if (NULL == ptDev) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = &s_tSTUartMap[nPos];
    ptHandle = &ptSTUart->tHandle;

    ptSTUart->fnRecvCB = NULL;

    /*! data conversion to register */
    ptHandle->Init.BaudRate = ptDev->tConfig.wBaudrate;

    nRet = uart_datawidth_transform(ptDev->tConfig.chDataWidth, &ptHandle->Init.WordLength);
    nRet |= uart_parity_transform(ptDev->tConfig.chParity, &ptHandle->Init.Parity);
    nRet |= uart_stopbits_transform(ptDev->tConfig.chStopBits, &ptHandle->Init.StopBits);
    nRet |= uart_mode_transform(ptDev->tConfig.chMode, &ptHandle->Init.Mode);
    nRet |= uart_flow_control_transform(ptDev->tConfig.chFlowControl, &ptHandle->Init.HwFlowCtl);
    if (XHAL_OK != nRet) {
        return XHAL_FAIL;
    }

    /*! static allocation of receiveing buffer */
    ptSTUart->pchRxBuf = c_tUartMap[nPos].pchBuf;
    ptSTUart->hwRxBufSize = c_tUartMap[nPos].hwBufSize;

    ptHandle->Instance = (USART_TypeDef *)c_tUartMap[nPos].ptUartPhy;
    ptHandle->Init.OverSampling = c_tUartMap[nPos].wOverSampling;

    if (HAL_OK != HAL_UART_Init(ptHandle)) {
        return XHAL_FAIL;
    }

    /*! [PE FE ORE] in DMA receive mode*/
    __HAL_UART_ENABLE_IT(&ptSTUart->tHandle, UART_IT_ERR);

    /*! OS compoent */
    ptSTUart->tTxMutexLock = osMutexNew(NULL);
    ptSTUart->tRxMutexLock = osMutexNew(NULL);

    ptSTUart->tTxSem = osSemaphoreNew(1, 0, NULL);

    /*! accquire rx semaphore in two function :
        xhal_uart_poll_dma_receive
        xhal_uart_wait_dma_received_until
    */
    ptSTUart->tRxSem = osSemaphoreNew(1, 0, NULL);

    if ((NULL == ptSTUart->tTxMutexLock) || (NULL == ptSTUart->tRxMutexLock) ||
        (NULL == ptSTUart->tTxSem) || (NULL == ptSTUart->tRxSem)) {
        return XHAL_FAIL;
    }

    return XHAL_OK;
}

int32_t xhal_uart_deinit(uart_dev_t *ptDev)
{
    int32_t nPos = 0;
    stm_uart_t* ptSTUart = NULL;

    if (NULL == ptDev) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = &s_tSTUartMap[nPos];
    ptSTUart->chState = 0;

    if (NULL != ptSTUart->tTimer) {
        if (osTimerIsRunning(ptSTUart->tTimer)) {
            osTimerStop(ptSTUart->tTimer);
        }

        if (osOK == osTimerDelete(ptSTUart->tTimer)) {
            ptSTUart->tTimer = NULL;
        } else {
            SYSLOG_F("del timer failed");
        }
    }

    HAL_DMA_Abort(ptSTUart->tHandle.hdmarx);
    HAL_DMA_Abort(ptSTUart->tHandle.hdmatx);

    HAL_UART_AbortReceive(&ptSTUart->tHandle);
    HAL_UART_AbortTransmit(&ptSTUart->tHandle);

    if (osOK != osMutexDelete(ptSTUart->tTxMutexLock)) {
        SYSLOG_F("del txLock failed");
        return XHAL_FAIL;
    }

    ptSTUart->tTxMutexLock = NULL;

    if (osOK != osMutexDelete(ptSTUart->tRxMutexLock)) {
        SYSLOG_F("del txLock failed");
        return XHAL_FAIL;
    }

    ptSTUart->tRxMutexLock = NULL;

    if (osOK != osSemaphoreDelete(ptSTUart->tTxSem)) {
        SYSLOG_F("del txSem failed");
        return XHAL_FAIL;
    }

    ptSTUart->tTxSem = NULL;

    if (osOK != osSemaphoreDelete(ptSTUart->tRxSem)) {
        SYSLOG_F("del rxLock failed");
        return XHAL_FAIL;
    }

    ptSTUart->tRxSem = NULL;

    return XHAL_OK;
}

/**
 *  \beief This function cannot be called from Interrupt Service Routines.
 */
int32_t xhal_uart_send_dma(uart_dev_t *ptDev, const void *pData, uint32_t wSize, uint32_t wTimeout)
{
    int32_t nPos = 0;
    int32_t nRet = XHAL_OK;
    stm_uart_t *ptSTUart = NULL;
    UART_HandleTypeDef *ptHandle = NULL;

    if (NULL == ptDev || NULL == pData) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = get_map_stm_uart(nPos);
    ptHandle = &ptSTUart->tHandle;    /*! USART1, etc */

    osMutexAcquire(ptSTUart->tTxMutexLock, osWaitForever);

    if (HAL_OK != HAL_UART_Transmit_DMA(ptHandle, (uint8_t *)pData, wSize)) {
        nRet = XHAL_FAIL;
        goto _exit;
    }

    __HAL_UART_ENABLE_IT(ptHandle, UART_IT_TC);

    if (osOK != osSemaphoreAcquire(ptSTUart->tTxSem, wTimeout)) {
        nRet = XHAL_FAIL;
    }

_exit:
    osMutexRelease(ptSTUart->tTxMutexLock);

    return nRet;
}

static void check_dma_receive(void *argument)
{
    stm_uart_t *ptSTUart = (stm_uart_t *)argument;
    uint16_t hwRecvCount;
    enum { START = 0, CHECK_DMA_CNDTR, TIME_COUNT, HANDLE_CALLBACK};

    switch (ptSTUart->chState) {
    case START:
        __HAL_DMA_SET_COUNTER(ptSTUart->tHandle.hdmarx, ptSTUart->hwRxBufSize);
        ptSTUart->hwTimeCount = 0;
        ptSTUart->chState = CHECK_DMA_CNDTR;
    // break;

    case CHECK_DMA_CNDTR:
        hwRecvCount = ptSTUart->hwRxBufSize - \
                      __HAL_DMA_GET_COUNTER(ptSTUart->tHandle.hdmarx);
        if (0 == hwRecvCount) {
            break;
        }

        if (ptSTUart->hwRecvCount != hwRecvCount) {
            ptSTUart->hwRecvCount = hwRecvCount;
            ptSTUart->hwTimeCount = 0;
            break;
        }

        ptSTUart->chState = TIME_COUNT;

    case TIME_COUNT:
        if ((ptSTUart->hwTimeCount++) >= 2) {
            HAL_UART_AbortReceive(&ptSTUart->tHandle);
            osSemaphoreRelease(ptSTUart->tRxSem);
            ptSTUart->chState = HANDLE_CALLBACK;
        } else {
            ptSTUart->chState = CHECK_DMA_CNDTR;
            break;
        }

    case HANDLE_CALLBACK:
        if (NULL != ptSTUart->fnRecvCB) {
            ptSTUart->fnRecvCB(ptSTUart->pRecvCBParameter,
                               ptSTUart->pchRxBuf, ptSTUart->hwRecvCount);
        }
        ptSTUart->chState = START;
        break;
    }
}

/**
 *  \beief This function cannot be called from Interrupt Service Routines.
 */
int32_t xhal_uart_poll_dma_receive(uart_dev_t *ptDev, uint8_t *pDst, uint32_t wBytes, uint32_t wTimeout)
{
    int32_t nPos = 0;
    int32_t nBytes = 0;
    osStatus_t tStatus = osOK;
    stm_uart_t *ptSTUart = NULL;
    HAL_StatusTypeDef tRet = HAL_BUSY;

    if (NULL == ptDev || NULL == pDst) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    /*! create a timer for check dma receive */
    ptSTUart = get_map_stm_uart(nPos);

    osMutexAcquire(ptSTUart->tRxMutexLock, osWaitForever);

    /*! TODO receive left bytes in dma rx buffer */

    tRet = HAL_UART_Receive_DMA(&ptSTUart->tHandle, ptSTUart->pchRxBuf, ptSTUart->hwRxBufSize);
    if (HAL_OK != tRet) {
        nBytes = XHAL_FAIL;
        goto _exit;
    }

    ptSTUart->tTimer = (NULL != ptSTUart->tTimer) ? ptSTUart->tTimer : \
                       osTimerNew(check_dma_receive, osTimerPeriodic, ptSTUart, NULL);

    if (!osTimerIsRunning(ptSTUart->tTimer)) {
        if (osOK != osTimerStart(ptSTUart->tTimer, 5)) {
            goto _exit;
        }
    }

    /*! wait receive semaphore */
    tStatus = osSemaphoreAcquire(ptSTUart->tRxSem, wTimeout);
    if (osOK != tStatus) {

        HAL_DMA_Abort(ptSTUart->tHandle.hdmarx);
        HAL_UART_AbortReceive(&ptSTUart->tHandle);
        nBytes = 0;

    } else {
        nBytes = MIN(ptSTUart->hwRecvCount, wBytes);
        memcpy(pDst, ptSTUart->pchRxBuf, nBytes);
    }

_exit:
    osMutexRelease(ptSTUart->tRxMutexLock);

    return nBytes;
}

int32_t xhal_uart_set_received_callback(uart_dev_t *ptDev, uart_func_t fn, void *parameter)
{
    int32_t nPos = 0;
    stm_uart_t *ptSTUart = NULL;

    if (NULL == ptDev || NULL == fn) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = get_map_stm_uart(nPos);
    ptSTUart->fnRecvCB = fn;
    ptSTUart->pRecvCBParameter = parameter;

    return XHAL_OK;
}

int32_t xhal_uart_set_send_cpl_callback(uart_dev_t *ptDev, uart_func_t fn, void *parameter)
{
    int32_t nPos = 0;
    stm_uart_t *ptSTUart = NULL;

    if (NULL == ptDev || NULL == fn) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = get_map_stm_uart(nPos);
    ptSTUart->fnSendCB = fn;
    ptSTUart->pSendCBParameter = parameter;

    return XHAL_OK;
}

int32_t xhal_uart_wait_dma_received_until(uart_dev_t *ptDev, uint32_t wTimeout)
{
    int32_t nPos = 0;
    int32_t nBytes = 0;
    stm_uart_t *ptSTUart = NULL;

    if (NULL == ptDev) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    ptSTUart = get_map_stm_uart(nPos);

    /*! wait receive semaphore */
    if (osOK == osSemaphoreAcquire(ptSTUart->tRxSem, wTimeout)) {
        nBytes = ptSTUart->hwRecvCount;
    } else {
        return XHAL_FAIL;
    }

    return nBytes;
}

int32_t xhal_uart_start_dma_receive(uart_dev_t *ptDev)
{
    int32_t nPos = 0;
    int32_t nRetval = XHAL_OK;

    stm_uart_t *ptSTUart = NULL;
    HAL_StatusTypeDef tRet = HAL_BUSY;

    if (NULL == ptDev) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptDev->chPort);
    if (XHAL_FAIL == nPos) {
        return XHAL_FAIL;
    }

    /*! create a timer for check dma receive */
    ptSTUart = get_map_stm_uart(nPos);

    osMutexAcquire(ptSTUart->tRxMutexLock, osWaitForever);

    /*! TODO receive left bytes in dma rx buffer */

    tRet = HAL_UART_Receive_DMA(&ptSTUart->tHandle, ptSTUart->pchRxBuf, ptSTUart->hwRxBufSize);
    if (HAL_OK != tRet) {
        nRetval = XHAL_FAIL;
        goto __exit;
    }

    ptSTUart->tTimer = (NULL != ptSTUart->tTimer) ? ptSTUart->tTimer : \
                       osTimerNew(check_dma_receive, osTimerPeriodic, ptSTUart, NULL);

    if (!osTimerIsRunning(ptSTUart->tTimer)) {
        if (osOK != osTimerStart(ptSTUart->tTimer, 5)) {
            nRetval = XHAL_FAIL;
        }
    }

__exit:

    if (XHAL_OK != nRetval) {
        HAL_DMA_Abort(ptSTUart->tHandle.hdmarx);
        HAL_UART_AbortReceive(&ptSTUart->tHandle);
    }

    osMutexRelease(ptSTUart->tRxMutexLock);

    return nRetval;
}

static int32_t uart_datawidth_transform(uint8_t chDataWidth, uint32_t *pwReg)
{
    switch (chDataWidth) {
    case DATA_WIDTH_8BIT:
        *pwReg = UART_WORDLENGTH_8B;
        break;

    case DATA_WIDTH_9BIT:
        *pwReg = UART_WORDLENGTH_9B;
        break;

    default:
        return XHAL_FAIL;
    }

    return XHAL_OK;
}

static int32_t uart_parity_transform(uint8_t chParity, uint32_t *pwReg)
{
    switch (chParity) {
    case UART_NO_PARITY:
        *pwReg = UART_PARITY_NONE;
        break;

    case UART_ODD_PARITY:
        *pwReg = UART_PARITY_ODD;
        break;

    case UART_EVEN_PARITY:
        *pwReg = UART_PARITY_EVEN;
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
        *pwReg = UART_STOPBITS_1;
        break;

    case UART_STOP_BITS_2:
        *pwReg = UART_STOPBITS_2;
        break;

    default:
        return XHAL_FAIL;
    }

    return XHAL_OK;
}

static int32_t uart_mode_transform(uint8_t chMode, uint32_t *pwReg)
{
    switch (chMode) {
    case MODE_TX:
        *pwReg = UART_MODE_TX;
        break;

    case MODE_RX:
        *pwReg = UART_MODE_RX;
        break;

    case MODE_TX_RX:
        *pwReg = UART_MODE_TX_RX;
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
        *pwReg = UART_HWCONTROL_NONE;
        break;

    case FLOW_CONTROL_CTS:
        *pwReg = UART_HWCONTROL_CTS;
        break;

    case FLOW_CONTROL_RTS:
        *pwReg = UART_HWCONTROL_RTS;
        break;
    case FLOW_CONTROL_CTS_RTS:
        *pwReg = UART_HWCONTROL_RTS_CTS;

    default:
        return XHAL_FAIL;
    }

    return XHAL_OK;
}

static void uart_handler(const void *ptUartIns)
{
    volatile uint8_t i = 0;
    USART_TypeDef *ptIns = (USART_TypeDef *)ptUartIns;

    for (i = 0; i < TOTAL_UART_NUM; i++) {
        if (ptIns == c_tUartMap[i].ptUartPhy) {

            HAL_DMA_Abort(s_tSTUartMap[i].tHandle.hdmatx);
            HAL_UART_AbortTransmit(&s_tSTUartMap[i].tHandle);

            if (NULL != s_tSTUartMap[i].fnSendCB) {
                s_tSTUartMap[i].fnSendCB(s_tSTUartMap[i].pSendCBParameter, NULL, 0);
            }

            osSemaphoreRelease(s_tSTUartMap[i].tTxSem);
            break;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uart_handler(huart->Instance);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    READ_REG(huart->Instance->DR);
}

/*************************** End of file ****************************/
