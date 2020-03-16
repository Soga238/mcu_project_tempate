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
*       hal_gpio.h *                                                 *
*                                                                    *
**********************************************************************
*/
#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include "..\bsp_cfg.h"
#include "stm32f10x.h"

/* Exported constants ----------------------------------------------*/
typedef enum {
    GPIO_PIN_RESET = 0u,
    GPIO_PIN_SET
} GPIO_PinState;

/* Exported functions --------------------------------------------- */
extern GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
extern void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
extern void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif
#endif
/*************************** End of file ****************************/
