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
*       i2c.c *                                                      *
*                                                                    *
**********************************************************************
*/

#include ".\i2c.h"
#include ".\xhal_i2c_gpio.h"

/*********************************************************************
*
*       Configuration, default values
*
**********************************************************************
*/

#define I2C_WRITE_BIT       0
#define I2C_READ_BIT        1

#define CHECK_PARAM(pDev)                               \
    do {                                                \
        if (NULL == ptDev) {                            \
            return XHAL_FAIL;                           \
        }                                               \
                                                        \
        ptGpioDev = get_map_gpio_dev(ptDev->chPort);    \
        if (NULL == ptGpioDev) {                        \
            return XHAL_FAIL;                           \
        }                                               \
    }while(0)
    
/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

extern const i2c_gpio_dev_mapping_t c_tI2cDevMap[TOTAL_I2C_DEV_NUM];
    
/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

const i2c_gpio_dev_t* get_map_gpio_dev(uint8_t chPort)
{
    uint8_t i;

    for (i = 0; (i < TOTAL_I2C_DEV_NUM) && (chPort != c_tI2cDevMap[i].chPort); ++i);

    return i == TOTAL_I2C_DEV_NUM ? NULL : &c_tI2cDevMap[i].tDev;
}

int32_t xhal_i2c_init(i2c_dev_t *ptDev)
{
    const i2c_gpio_dev_t *ptGpioDev = NULL;

    CHECK_PARAM(ptDev);

    if (i2c_gpio_init(ptGpioDev)) {
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

/**
 *  \breif 
 *
 */
int32_t xhal_i2c_mem_write(i2c_dev_t *ptDev, uint16_t hwDevAddr, uint16_t hwAddress, uint8_t *pData, uint16_t hwSize)
{
    uint16_t i = 0;
    int32_t nResult = XHAL_FAIL;
    const i2c_gpio_dev_t *ptGpioDev = NULL;

    CHECK_PARAM(ptDev);

    i2c_gpio_start(ptGpioDev);

    if(!i2c_gpio_write_byte(ptGpioDev, hwDevAddr | I2C_WRITE_BIT)) {
        goto __exit;
    }

    if (I2C_ADDRESS_WIDTH_7BIT == ptDev->tConfig.chAddressWidth) {
        if(!i2c_gpio_write_byte(ptGpioDev, (uint8_t)hwAddress)) {
            goto __exit;
        }
    } else if (I2C_ADDRESS_WIDTH_10BIT == ptDev->tConfig.chAddressWidth) {
        if(!i2c_gpio_write_byte(ptGpioDev, hwAddress >> 8)) {
            goto __exit;
        }
        if(!i2c_gpio_write_byte(ptGpioDev, hwAddress)) {
            goto __exit;
        }
    }

    for (i = 0; i < hwSize; i++) {
        if (!i2c_gpio_write_byte(ptGpioDev, pData[i])) {
            goto __exit;
        }
    }
    
    nResult = XHAL_OK;
    
__exit:
    i2c_gpio_stop(ptGpioDev);
    i2c_gpio_bus_release(ptGpioDev);
    return nResult;
}

/**
 *  \breif 
 *
 */
int32_t xhal_i2c_mem_read(i2c_dev_t *ptDev, uint16_t hwDevAddr, uint16_t hwAddress, uint8_t *pDst, uint16_t hwSize)
{
    uint16_t i = 0;
    int32_t nResult = XHAL_FAIL;
    const i2c_gpio_dev_t *ptGpioDev = NULL;

    CHECK_PARAM(ptDev);

    i2c_gpio_start(ptGpioDev);
    if(!i2c_gpio_write_byte(ptGpioDev, hwDevAddr | I2C_WRITE_BIT)) {
        goto __exit;
    }
    
    if (I2C_ADDRESS_WIDTH_7BIT == ptDev->tConfig.chAddressWidth) {
        if(!i2c_gpio_write_byte(ptGpioDev, (uint8_t)hwAddress)) {
            goto __exit;
        }
    } else if (I2C_ADDRESS_WIDTH_10BIT == ptDev->tConfig.chAddressWidth) {
        if(!i2c_gpio_write_byte(ptGpioDev, hwAddress >> 8)) {
            goto __exit;
        }
        if(!i2c_gpio_write_byte(ptGpioDev, hwAddress)) {
            goto __exit;
        }
    }
    
    i2c_gpio_start(ptGpioDev); /*! start repeat */
    if(!i2c_gpio_write_byte(ptGpioDev, hwDevAddr | I2C_READ_BIT)) {
        goto __exit;
    }
    
    for (i = 0; i < hwSize; i++) {
        pDst[i] = i2c_gpio_read_byte(ptGpioDev);
        i2c_gpio_ack_put(ptGpioDev, i != (hwSize - 1)); /*! send nack final */
    }
    
    nResult = XHAL_OK;

__exit:
    i2c_gpio_stop(ptGpioDev);
    i2c_gpio_bus_release(ptGpioDev);
    return nResult;
}

/**
 * \brief       Check bus idle status
 * \param[in]
 * \return      
 */
int32_t xhal_i2c_check_device(i2c_dev_t *ptDev, uint16_t hwDevAddr)
{
    int32_t nResult = XHAL_FAIL;
    const i2c_gpio_dev_t *ptGpioDev = NULL;
    
    CHECK_PARAM(ptDev);
    
    i2c_gpio_start(ptGpioDev);
    if(i2c_gpio_write_byte(ptGpioDev, hwDevAddr | I2C_WRITE_BIT)) {
        nResult = XHAL_OK;
    }
    
    i2c_gpio_stop(ptGpioDev);
    i2c_gpio_bus_release(ptGpioDev);
    return nResult;
}


/*
static int32_t stm32_i2c_param_transform(i2c_config_t *ptConfig, I2C_HandleTypeDef *ptHandle)
{
    I2C_InitTypeDef *ptInit = NULL;

    if ((NULL == ptConfig) || (NULL == ptHandle)) {
        return XHAL_FAIL;
    }

    ptInit = &ptHandle->Init;

    switch (ptConfig->wAddressWidth) {
        case I2C_ADDRESS_WIDTH_7BIT:
            ptInit->AddressingMode = I2C_ADDRESSINGMODE_7BIT;
            break;

        case I2C_ADDRESS_WIDTH_10BIT:
            ptInit->AddressingMode = I2C_ADDRESSINGMODE_10BIT;
            break;

        default:
            return XHAL_FAIL;
    }

    switch (ptConfig->wFreq) {
        case I2C_BUS_BIT_RATES_100K:
            ptInit->ClockSpeed = 100000;
            break;

        case I2C_BUS_BIT_RATES_400K:
            ptInit->ClockSpeed = 400000;
            ptInit->DutyCycle = I2C_DUTYCYCLE_2;
            break;

        default:
            return XHAL_FAIL;
    }

    switch (ptConfig->chMode) {
        case I2C_MODE_MASTER:
            // ptHandle->Mode = I2C_MODE_MASTER;
            break;

        default:
            return XHAL_FAIL;
    }

    return XHAL_OK;
}
*/

/*************************** End of file ****************************/
