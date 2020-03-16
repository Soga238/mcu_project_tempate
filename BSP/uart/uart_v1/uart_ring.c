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
*       uart_ring.c *                                                *
*                                                                    *
**********************************************************************
*/

#include ".\uart_ring.h"

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

static void receive_callback(void *parameter, uint8_t *pchBuf, uint16_t hwLength)
{
    uart_ringbuf_dev_t *ptDev = (uart_ringbuf_dev_t *)parameter;
    
    if (NULL != ptDev) {        
        ringbuf_put(&ptDev->tRxRing, pchBuf, hwLength);
    }
}

int32_t xhal_uart_ringbuf_init(uart_ringbuf_dev_t *ptDev, uart_dev_t *ptUartDev, uint8_t *pchRxBuf, uint16_t hwRxBufSize)
{
    if (NULL == ptDev || NULL == ptUartDev) {
        return XHAL_FAIL;
    }
    
    if (NULL == pchRxBuf || 0 == hwRxBufSize) {
        return XHAL_FAIL;
    }
    
    ptDev->ptUartDev = ptUartDev;
    
    if (XHAL_OK != xhal_uart_init(ptDev->ptUartDev)) {
        return XHAL_FAIL;
    }
    
    xhal_uart_set_received_callback(ptDev->ptUartDev, receive_callback, (void *)ptDev);
    
    if (!ringbuf_init(&ptDev->tRxRing, pchRxBuf, hwRxBufSize)) {
        return XHAL_FAIL;
    }
    
    return XHAL_OK;
}

int32_t xhal_uart_ringbuf_receive(uart_ringbuf_dev_t *ptDev, uint8_t *pchDst, uint16_t hwLength, uint32_t wTimeout)
{
    int32_t nBytes = 0;
    
    if (NULL == ptDev || NULL == pchDst) {
        return XHAL_FAIL;
    }
    
    nBytes = ringbuf_get(&ptDev->tRxRing, pchDst, hwLength);
    if (nBytes <= 0) {
        
        /*! start UART RX dma */
        if (XHAL_OK == xhal_uart_start_dma_receive(ptDev->ptUartDev)) {
            xhal_uart_wait_dma_received_until(ptDev->ptUartDev, wTimeout);
        }
        
        return ringbuf_get(&ptDev->tRxRing, pchDst, hwLength);
    }
    
    return nBytes;
}

/*************************** End of file ****************************/
