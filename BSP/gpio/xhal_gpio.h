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
*       xhal_gpio.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __XHAL_GPIO_H__
#define __XHAL_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "..\bsp_cfg.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
    uint16_t    hwPort;
    void        *pGpioGroup;
    uint16_t    hwPin;
}gpio_mapping_t;

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
