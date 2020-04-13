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
#include "./bsp_gpio.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define RCC_GPIOA_CLK_ENABLE()          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
#define RCC_GPIOB_CLK_ENABLE()          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
#define RCC_GPIOC_CLK_ENABLE()          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
#define RCC_GPIOD_CLK_ENABLE()          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

#define RCC_GPIOA_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
#define RCC_GPIOB_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
#define RCC_GPIOC_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, DISABLE);
#define RCC_GPIOD_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, DISABLE);
#define RCC_GPIOE_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, DISABLE);
#define RCC_GPIOF_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, DISABLE);
#define RCC_GPIOG_CLK_DISABLE()         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, DISABLE);

/**
 * \brief Initialization MCU Pin
 * \param
 * \return
 */
void bsp_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_GPIOA_CLK_ENABLE();
    RCC_GPIOC_CLK_ENABLE();
    RCC_GPIOD_CLK_ENABLE();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1;
    GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED2;
    GPIO_Init(GPIO_PORT_LED2, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED3;    // PB3 设置无效，与PB15 物理短接
    GPIO_Init(GPIO_PORT_LED3, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED4;
    GPIO_Init(GPIO_PORT_LED4, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED5;
    GPIO_Init(GPIO_PORT_LED5, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED6;
    GPIO_Init(GPIO_PORT_LED6, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED7;
    GPIO_Init(GPIO_PORT_LED7, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED8;
    GPIO_Init(GPIO_PORT_LED8, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_EEP_WP;
    GPIO_Init(GPIO_PORT_EEP_WP, &GPIO_InitStructure);
    EEP_WP_OFF();

    // EEPROM IIC SDA SCL
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IIC_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIO_PORT_IIC_SDA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IIC_SCL;
    GPIO_Init(GPIO_PORT_IIC_SCL, &GPIO_InitStructure);

}

void bsp_gpio_deinit(void)
{
    RCC_GPIOA_CLK_DISABLE();
    RCC_GPIOB_CLK_DISABLE();
    RCC_GPIOC_CLK_DISABLE();
    RCC_GPIOD_CLK_DISABLE();
    RCC_GPIOE_CLK_DISABLE();
    RCC_GPIOF_CLK_DISABLE();
    RCC_GPIOG_CLK_DISABLE();

    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD);
    GPIO_DeInit(GPIOE);
    GPIO_DeInit(GPIOF);
    GPIO_DeInit(GPIOG);
}

/*************************** End of file ****************************/
