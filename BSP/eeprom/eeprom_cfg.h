#ifndef EEPROM_CFG_H
#define EEPROM_CFG_H

#define AT24C128

// 8-byte Page (1K, 2K), 16-byte Page (4K, 8K, 16K) Write Modes

#ifdef AT24C02
    #define EE_MODEL_NAME       "AT24C02"
    #define EE_DEV_ADDR         0xA0        /* 设备地址 */
    #define EE_PAGE_SIZE        8           /* 页面大小(字节) */
    #define EE_SIZE             256         /* 总容量(字节) */
    #define EE_ADDR_BYTES       1           /* 地址字节个数 */
#endif

#ifdef AT24C16
    #define EE_MODEL_NAME       "AT24C16"
    #define EE_DEV_ADDR         0xA0        /* 设备地址 */
    #define EE_PAGE_SIZE        16          /* 页面大小(字节) */
    #define EE_SIZE             2048        /* 总容量(字节) */
    #define EE_ADDR_BYTES       1           /* 地址字节个数 */
#endif


#ifdef AT24C128
    #define EE_MODEL_NAME       "AT24C128"
    #define EE_DEV_ADDR         0xA0
    #define EE_PAGE_SIZE        64
    #define EE_SIZE             (16 * 1024)
    #define EE_ADDR_BYTES       2
#endif

#define PORT_I2C_WP        GPIOA
#define PIN_I2C_WP         GPIO_PIN_0

/* 定义写保护引脚 */
#define EEPROM_WP_1()         do{PORT_I2C_WP->BSRR = PIN_I2C_WP;}while(0) 
#define EEPROM_WP_0()         do{PORT_I2C_WP->BRR = PIN_I2C_WP;}while(0)

#endif
