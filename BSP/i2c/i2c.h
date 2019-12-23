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
*       i2c.h *                                                      *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\bsp_cfg.h"

#define I2C_MODE_MASTER             1
/*! #define I2C_MODE_SLAVE      2 */

#define I2C_MEM_ADDR_SIZE_8BIT      1
#define I2C_MEM_ADDR_SIZE_16BIT     2

#define I2C_BUS_BIT_RATES_100K      100000
#define I2C_BUS_BIT_RATES_400K      400000
#define I2C_BUS_BIT_RATES_3400K     3400000

#define I2C_ADDRESS_WIDTH_7BIT      0
#define I2C_ADDRESS_WIDTH_10BIT     1

typedef struct {
    uint16_t    hwDevAddr;
    uint8_t     chMode;
    uint8_t     chAddressWidth;
    uint32_t    wFreq;
}i2c_config_t;

typedef struct {
    uint8_t         chPort;
    i2c_config_t    tConfig;
    void           *pPrivData;
}i2c_dev_t;   

extern int32_t xhal_i2c_init(i2c_dev_t *ptDev);
extern int32_t xhal_i2c_mem_write(i2c_dev_t *ptDev, uint16_t hwDevAddr, uint16_t hwAddress, uint8_t *pData, uint16_t hwSize);
extern int32_t xhal_i2c_mem_read(i2c_dev_t *ptDev, uint16_t hwDevAddr, uint16_t hwAddress, uint8_t *pDst, uint16_t hwSize);
extern int32_t xhal_i2c_check_device(i2c_dev_t *ptDev, uint16_t hwDevAddr);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
