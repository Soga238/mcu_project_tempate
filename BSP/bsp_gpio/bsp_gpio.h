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
*       BSP GPIO * GPIO DRIVE                                        *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

#include "stm32f10x_gpio.h"
/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define GPIO_PORT_LED1      GPIOD
#define GPIO_PIN_LED1       GPIO_Pin_2

#define GPIO_PORT_LED2      GPIOC
#define GPIO_PIN_LED2       GPIO_Pin_8

#define GPIO_PORT_LED3      GPIOB
#define GPIO_PIN_LED3       GPIO_Pin_15

#define GPIO_PORT_LED4      GPIOC
#define GPIO_PIN_LED4       GPIO_Pin_9

#define GPIO_PORT_LED5      GPIOB
#define GPIO_PIN_LED5       GPIO_Pin_8

#define GPIO_PORT_LED6      GPIOA
#define GPIO_PIN_LED6       GPIO_Pin_7

#define GPIO_PORT_LED7      GPIOC
#define GPIO_PIN_LED7       GPIO_Pin_6

#define GPIO_PORT_LED8      GPIOC
#define GPIO_PIN_LED8       GPIO_Pin_7

#define LED1_ON()           do{GPIO_PORT_LED1->BSRR = GPIO_PIN_LED1;}while(0)
#define LED2_ON()           do{GPIO_PORT_LED2->BSRR = GPIO_PIN_LED2;}while(0)
#define LED3_ON()           do{GPIO_PORT_LED3->BSRR = GPIO_PIN_LED3;}while(0)
#define LED4_ON()           do{GPIO_PORT_LED4->BSRR = GPIO_PIN_LED4;}while(0)
#define LED5_ON()           do{GPIO_PORT_LED5->BSRR = GPIO_PIN_LED5;}while(0)
#define LED6_ON()           do{GPIO_PORT_LED6->BSRR = GPIO_PIN_LED6;}while(0)
#define LED7_ON()           do{GPIO_PORT_LED7->BSRR = GPIO_PIN_LED7;}while(0)
#define LED8_ON()           do{GPIO_PORT_LED8->BSRR = GPIO_PIN_LED8;}while(0)

#define LED1_OFF()          do{GPIO_PORT_LED1->BRR  = GPIO_PIN_LED1;}while(0)
#define LED2_OFF()          do{GPIO_PORT_LED2->BRR  = GPIO_PIN_LED2;}while(0)
#define LED3_OFF()          do{GPIO_PORT_LED3->BRR  = GPIO_PIN_LED3;}while(0)
#define LED4_OFF()          do{GPIO_PORT_LED4->BRR  = GPIO_PIN_LED4;}while(0)
#define LED5_OFF()          do{GPIO_PORT_LED5->BRR  = GPIO_PIN_LED5;}while(0)
#define LED6_OFF()          do{GPIO_PORT_LED6->BRR  = GPIO_PIN_LED6;}while(0)
#define LED7_OFF()          do{GPIO_PORT_LED7->BRR  = GPIO_PIN_LED7;}while(0)
#define LED8_OFF()          do{GPIO_PORT_LED8->BRR  = GPIO_PIN_LED8;}while(0)

#define LED1_TOGGLE()       do{GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;}while(0)
#define LED2_TOGGLE()       do{GPIO_PORT_LED2->ODR ^= GPIO_PIN_LED2;}while(0)
#define LED3_TOGGLE()       do{GPIO_PORT_LED3->ODR ^= GPIO_PIN_LED3;}while(0)
#define LED4_TOGGLE()       do{GPIO_PORT_LED4->ODR ^= GPIO_PIN_LED4;}while(0)
#define LED5_TOGGLE()       do{GPIO_PORT_LED5->ODR ^= GPIO_PIN_LED5;}while(0)
#define LED6_TOGGLE()       do{GPIO_PORT_LED6->ODR ^= GPIO_PIN_LED6;}while(0)
#define LED7_TOGGLE()       do{GPIO_PORT_LED7->ODR ^= GPIO_PIN_LED7;}while(0)
#define LED8_TOGGLE()       do{GPIO_PORT_LED8->ODR ^= GPIO_PIN_LED8;}while(0)

#define GPIO_PORT_IIC_SDA   GPIOB
#define GPIO_PIN_IIC_SDA    GPIO_Pin_14

#define GPIO_PORT_IIC_SCL   GPIOB
#define GPIO_PIN_IIC_SCL    GPIO_Pin_13

#define EEP_SDA_HIGH()      do{GPIO_PORT_IIC_SDA->BSRR = GPIO_PIN_IIC_SDA;}while(0)
#define EEP_SDA_LOW()       do{GPIO_PORT_IIC_SDA->BRR  = GPIO_PIN_IIC_SDA;}while(0)

#define EEP_SCL_HIGH()      do{GPIO_PORT_IIC_SCL->BSRR = GPIO_PIN_IIC_SCL;}while(0)
#define EEP_SCL_LOW()       do{GPIO_PORT_IIC_SCL->BRR  = GPIO_PIN_IIC_SCL;}while(0)

#define EEP_SDA_READ()      ((GPIO_PORT_IIC_SDA->IDR & GPIO_PIN_IIC_SDA) != 0)
#define EEP_SCL_READ()      ((GPIO_PORT_IIC_SCL->IDR & GPIO_PIN_IIC_SCL) != 0)

#define GPIO_PORT_EEP_WP    GPIOB
#define GPIO_PIN_EEP_WP     GPIO_Pin_15

#define EEP_WP_ON()         do{GPIO_PORT_EEP_WP->BSRR  = GPIO_PIN_EEP_WP;}while(0)
#define EEP_WP_OFF()        do{GPIO_PORT_EEP_WP->BRR   = GPIO_PIN_EEP_WP;}while(0)


/*********************************************************************
*
*       API Function
*
**********************************************************************
*/

extern void bsp_gpio_init(void);
extern void bsp_gpio_deinit(void);
#endif

/*************************** End of file ****************************/
