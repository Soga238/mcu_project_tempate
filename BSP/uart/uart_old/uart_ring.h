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
*       uart_ring.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __UART_TING_H__
#define __UART_TING_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include ".\uart.h"
#include "..\service\ringbuf\ringbuf.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
    uart_dev_t      *ptUartDev;
    char_ring_t     tRxRing;
    uint32_t        wReadTimeout;
}uart_ringbuf_dev_t;
    
/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern int32_t xhal_uart_ringbuf_init(uart_ringbuf_dev_t *ptDev, uart_dev_t *ptUartDev, uint8_t *pchRxBuf, uint16_t hwRxBufSize);
extern int32_t xhal_uart_ringbuf_receive(uart_ringbuf_dev_t *ptDev, uint8_t *pchDst, uint16_t hwLength, uint32_t wTimeout);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
