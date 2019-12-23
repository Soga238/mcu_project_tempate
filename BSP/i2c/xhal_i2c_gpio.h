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
*       xhal_i2c_gpio.h *                                            *
*                                                                    *
**********************************************************************
*/
#ifndef __XHAL_I2C_GPIO_H__
#define __XHAL_I2C_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include ".\i2c_gpio.h"

typedef struct {
    uint8_t         chPort; 
    i2c_gpio_dev_t  tDev;
}i2c_gpio_dev_mapping_t;

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
