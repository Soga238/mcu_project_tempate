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
*       bsp.c *                                                      *
*                                                                    *
**********************************************************************
*/

#include ".\gpio\xhal_gpio.h"
#include ".\uart\xhal_uart.h"
#include ".\i2c\xhal_i2c_gpio.h"

#include "..\service\key\key.h"
#include "Driver_USART.h"

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

const gpio_mapping_t c_tGpioMap[TOTAL_GPIO_NUM] = {
    {PORT_LED0,     LED0_GPIO_Port,             LED0_Pin},
    {PORT_LED1,     LED1_GPIO_Port,             LED1_Pin},
    {PORT_LED2,     LED2_GPIO_Port,             LED2_Pin},
    {PORT_LED3,     LED3_GPIO_Port,             LED3_Pin},
    {PORT_LED4,     LED4_GPIO_Port,             LED4_Pin},
    {PORT_LED5,     LED5_GPIO_Port,             LED5_Pin},
    {PORT_GSM_RUN,  GSM_RUN_GPIO_Port,          GSM_RUN_Pin},
    {PORT_GSM_PWR,  GSM_POWER_GPIO_Port,        GSM_POWER_Pin},
    {PORT_EEP_PRT,  EEPROM_PROTECT_GPIO_Port,   EEPROM_PROTECT_Pin},
    {PORT_KEY,      KEY_IN_GPIO_Port,           KEY_IN_Pin},
    {PORT_485_DIR,  RS485_DIR_GPIO_Port,        RS485_DIR_Pin},
    {PORT_E24_SDA,  I2C1_SDA_GPIO_Port,         I2C1_SDA_Pin},
    {PORT_E24_SCL,  I2C1_SCL_GPIO_Port,         I2C1_SCL_Pin}
};

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART3;

const uart_mapping_t c_tUartMap[TOTAL_UART_NUM] = {
    {
        PORT_UART_GSM,
        (void *) &Driver_USART3,
        {
            UART_OVERSAMPLING_16,
            0,
            NULL
        }
    },
    {
        PORT_UART_MB,
        (void *) &Driver_USART1,
        {
            UART_OVERSAMPLING_16,
            0,
            NULL
        }
    },
};

const i2c_gpio_dev_mapping_t c_tI2cDevMap[TOTAL_I2C_DEV_NUM] = {
    {
        .chPort = PORT_I2C1,
        .tDev = {
            .chSdaPinPort = PORT_E24_SDA,
            .chSclPinPort = PORT_E24_SCL,
            .wSpeed = 30
        }
    }
};

uint8_t get_key_scan_value(void)
{
    uint32_t wKeyVal = 1;

    if (XHAL_OK == xhal_gpio_get_by_port(PORT_KEY, &wKeyVal)) {
        if (0 == wKeyVal) {
            return KEY_VALUE1;
        }
    }
    return KEY_NULL;
}

/*************************** End of file ****************************/
