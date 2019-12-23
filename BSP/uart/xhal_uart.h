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
*       xhal_uart.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __XHAL_UART_H__
#define __XHAL_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    uint8_t  chPort;
    void     *ptUartPhy;
    
    /*! attr */
    struct {
        uint32_t wOverSampling;
        uint16_t hwBufSize;
        uint8_t *pchBuf;
    };

}uart_mapping_t;

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
