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
*       uart.h *                                                     *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\bsp_cfg.h"

/*********************************************************************
*
*       Configuration, default values
*
**********************************************************************
*/

/**
 *  \warning Do not modfiy macro definitions
 */
 
#define DATA_WIDTH_5BIT         0
#define DATA_WIDTH_6BIT         1
#define DATA_WIDTH_7BIT         2
#define DATA_WIDTH_8BIT         3
#define DATA_WIDTH_9BIT         4
        
#define FLOW_CONTROL_DISABLED   0
#define FLOW_CONTROL_CTS        1
#define FLOW_CONTROL_RTS        2
#define FLOW_CONTROL_CTS_RTS    3
        
#define UART_NO_PARITY          0
#define UART_ODD_PARITY         1
#define UART_EVEN_PARITY        2
 
#define UART_STOP_BITS_1        0
#define UART_STOP_BITS_2        1

#define MODE_TX                 0
#define MODE_RX                 1
#define MODE_TX_RX              2
    
/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
    uint32_t    wBaudrate;
    uint8_t     chParity;
    uint8_t     chDataWidth;
    uint8_t     chStopBits;
    uint8_t     chFlowControl;
    uint8_t     chMode;   
}uart_config_t;

typedef struct {
    uint8_t         chPort;         /*! uart port */
    uart_config_t   tConfig;        /*! uart config */
    void            *pPrivData;     /*! private data */
}uart_dev_t;
   
typedef void (*uart_func_t)(void *parameter, uint8_t *pchBuf, uint16_t hwLength);

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

/**
 *  \warning These function cannot be called from Interrupt Service Routines.
 *
 *      (+) xhal_uart_init
 *      (+) xhal_uart_send_dma
 *      (+) xhal_uart_poll_dma_receive
 *      (+) xhal_uart_start_dma_receive
 *      (+) xhal_uart_wait_dma_received_until
 */

extern int32_t xhal_uart_init(uart_dev_t *uart);
extern int32_t xhal_uart_send_dma(uart_dev_t *ptUartDev, const void *pData, uint32_t wSize, uint32_t wTimeout);
extern int32_t xhal_uart_poll_dma_receive(uart_dev_t *ptUartDev, uint8_t *pDst, uint32_t wBytes, uint32_t wTimeout);

extern int32_t xhal_uart_start_dma_receive(uart_dev_t *ptDev);
extern int32_t xhal_uart_wait_dma_received_until(uart_dev_t *ptDev, uint32_t wTimeout);

extern int32_t xhal_uart_set_received_callback(uart_dev_t *ptDev, uart_func_t fn, void *parameter);
extern int32_t xhal_uart_set_send_cpl_callback(uart_dev_t *ptDev, uart_func_t fn, void *parameter);

extern int32_t xhal_uart_deinit(uart_dev_t *ptDev);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
