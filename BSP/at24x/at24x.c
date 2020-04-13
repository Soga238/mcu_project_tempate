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
*       at24x.c *                                                    *
*                                                                    *
**********************************************************************
*/

#include ".\at24x.h"
#include "..\i2c\i2c.h"
#include "..\gpio\gpio.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#ifdef AT24C128
    #define  EE_PAGE_SIZE       64
    #define  EE_PAGE_NUM        256
    #define  EE_PAGE_MASK       0x3F
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    i2c_dev_t   *ptI2cDev;
    uint16_t    hwPageSize;
    uint16_t    hwPageNum;
    uint16_t    hwDevAddr;
} at24x_local_dev_t;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static i2c_dev_t s_tIIC = {
    .chPort = PORT_AT24X128,
    .tConfig = {
        .chAddressWidth = I2C_ADDRESS_WIDTH_10BIT,
        .hwDevAddr = 0,
        .chMode = I2C_MODE_MASTER,
    },
    .pPrivData = NULL
};

static at24x_local_dev_t s_tEEprom = {
    .ptI2cDev   = &s_tIIC,
    .hwPageSize = EE_PAGE_SIZE,
    .hwPageNum  = EE_PAGE_NUM,
    .hwDevAddr  = AT24X128_DEV_ADDR
};

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

bool at24x_init(void)
{
    return XHAL_OK == xhal_i2c_init(s_tEEprom.ptI2cDev);
}

bool at24x_write_bytes(uint16_t hwAddress, uint8_t *pchData, uint16_t hwSize)
{
    bool bRetval = true;
    uint16_t hwBytes = 0;

    while (hwSize > 0) {
        
        /*! Write data only on the same page */
        hwBytes = s_tEEprom.hwPageSize - (hwAddress & (s_tEEprom.hwPageSize - 1));        
        hwBytes = hwBytes > hwSize ? hwSize : hwBytes;
        
        while(!at24x_device_poll());
        
        if (XHAL_OK != xhal_i2c_mem_write(s_tEEprom.ptI2cDev, s_tEEprom.hwDevAddr, hwAddress, pchData, hwBytes)) {
            bRetval = false;
            break;
        }

        hwSize -= hwBytes;
        hwAddress += hwBytes;
        pchData += hwBytes;
    }

    return bRetval;
}

bool at24x_read_bytes(uint16_t hwAddress, uint8_t *pchDst, uint16_t hwSize)
{
    while(!at24x_device_poll());
    
    return XHAL_OK == xhal_i2c_mem_read(s_tEEprom.ptI2cDev, s_tEEprom.hwDevAddr, hwAddress, pchDst, hwSize);
}

bool at24x_device_poll(void)
{
    return XHAL_OK == xhal_i2c_check_device(s_tEEprom.ptI2cDev, s_tEEprom.hwDevAddr);
}

/*************************** End of file ****************************/
