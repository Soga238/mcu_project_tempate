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
*       bsp_cfg.h *                                                  *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_CFG_H__
#define __BSP_CFG_H__

#ifdef __cplusplus
    extern "C" {
#endif

/*
#include "..\utility\lz_types.h"
#include "..\utility\lz_bitops.h"
#include "..\utility\lz_common.h"
#include "..\compoents\3rdparty\SEGGER RTT\SEGGER_RTT.h"
*/

#include "..\usrapp\usrapp_cfg.h"

#define __ulog(format, ...)         SEGGER_RTT_printf(0u, format"\n", ##__VA_ARGS__);
#define ULOG_D                      __ulog

#define XHAL_OK                     0
#define XHAL_FAIL                   -1

#define WAIT_FOREVER                osWaitForever
        
/*! user peripherals */
#include "..\hal\mcu\stm32f1xx\Inc\main.h"

/*! GPIO */
#define TOTAL_GPIO_NUM              13

#define PORT_LED0                   0
#define PORT_LED1                   1
#define PORT_LED2                   2
#define PORT_LED3                   3
#define PORT_LED4                   4
#define PORT_LED5                   5
#define PORT_GSM_RUN                6
#define PORT_GSM_PWR                7
#define PORT_EEP_PRT                8        
#define PORT_485_DIR                9 
#define PORT_KEY                    10  
#define PORT_E24_SDA                11
#define PORT_E24_SCL                12
        
/*! UART */
#define TOTAL_UART_NUM              2

#define PORT_UART_GSM               0
#define PORT_UART_MB                1        
        
#define GSM_UART_BUF_SIZE           256
#define MB_UART_BUF_SIZE            1024
        
/*! I2C */
#define I2C_IO_SIMULATION_ENABLED   1

#define TOTAL_I2C_DEV_NUM           1
#define PORT_I2C1                   0

/*! AT24X */

#define AT24C128

#define AT24X_DEV_NUM               1
#define PORT_AT24X128               0

#define AT24X128_DEV_ADDR           0xA0

/*£¡ KEY */

#define KEY_VALUE1                  0xA0

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
