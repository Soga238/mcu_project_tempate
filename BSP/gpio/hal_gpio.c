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
*       hal_gpio.c *                                                 *
*                                                                    *
**********************************************************************
---------------------------END-OF-HEADER------------------------------
Note: 使用ST HAL库中的API, 目的是让gpio.c的代码不需要被修改
----------------------------------------------------------------------
*/

/* Includes --------------------------------------------------------*/
#include ".\hal_gpio.h"

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_PinState bitstatus;

    if ((GPIOx->IDR & GPIO_Pin) != (uint32_t)GPIO_PIN_RESET) {
        bitstatus = GPIO_PIN_SET;
    } else {
        bitstatus = GPIO_PIN_RESET;
    }
    return bitstatus;
}

void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (PinState != GPIO_PIN_RESET) {
        GPIOx->BSRR = GPIO_Pin;
    } else {
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16u;
    }
}

void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if ((GPIOx->ODR & GPIO_Pin) != 0x00u) {
        GPIOx->BRR = (uint32_t)GPIO_Pin;
    } else {
        GPIOx->BSRR = (uint32_t)GPIO_Pin;
    }
}
/*************************** End of file ****************************/
