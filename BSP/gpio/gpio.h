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
*       gpio.h *                                                     *
*                                                                    *
**********************************************************************
*/

#ifndef __GPIO_H__
#define __GPIO_H__

#include "..\bsp_cfg.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define GPIO_LEVEL_LOW          0  
#define GPIO_LEVEL_HIGH         1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    uint8_t     chPort;
    void        *pPrivData;
}gpio_dev_t;    

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

extern int32_t xhal_gpio_init(gpio_dev_t *ptGpio);
extern int32_t xhal_gpio_get(gpio_dev_t *ptGpio, uint32_t *pwValue);
extern int32_t xhal_gpio_set(gpio_dev_t *ptGpio, int32_t nValue);
extern int32_t xhal_gpio_toggle(gpio_dev_t *ptGpio);

extern int32_t xhal_gpio_init_by_port(uint8_t chPort);
extern int32_t xhal_gpio_get_by_port(uint8_t chPort, uint32_t *pwValue);
extern int32_t xhal_gpio_set_by_port(uint8_t chPort, int32_t nValue);
extern int32_t xhal_gpio_toggle_by_port(uint8_t chPort);

#endif

/*************************** End of file ****************************/
