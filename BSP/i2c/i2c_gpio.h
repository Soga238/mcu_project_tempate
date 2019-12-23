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
*       i2c_gpio.h *                                                 *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_I2C_GPIO_H__
#define __BSP_I2C_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\bsp_cfg.h"
#include "..\gpio\gpio.h"

typedef struct {
    uint8_t     chSdaPinPort;
    uint8_t     chSclPinPort;
    uint32_t    wSpeed;
}i2c_gpio_devinfo_t;

//#pragma language=extended       /*! use anonymous stucts */
typedef struct {
    // const i2c_gpio_devinfo_t tDevInfo;
//    struct {
        uint8_t     chSdaPinPort;
        uint8_t     chSclPinPort;
        uint32_t    wSpeed;
//    };
}i2c_gpio_dev_t;

extern bool     i2c_gpio_init(const i2c_gpio_dev_t *ptDev);
extern void     i2c_gpio_start(const i2c_gpio_dev_t *ptDev);
extern bool     i2c_gpio_ack_get(const i2c_gpio_dev_t *ptDev);
extern void     i2c_gpio_stop(const i2c_gpio_dev_t *ptDev);
extern bool     i2c_gpio_write_byte(const i2c_gpio_dev_t *ptDev, uint8_t chData);
extern uint8_t  i2c_gpio_read_byte(const i2c_gpio_dev_t *ptDev);
extern void     i2c_gpio_ack_put(const i2c_gpio_dev_t *ptDev, bool bAck);

extern bool     i2c_gpio_bus_release(const i2c_gpio_dev_t *ptDev);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
