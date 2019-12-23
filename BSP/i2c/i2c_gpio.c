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
*       i2c_gpio.c *                                                 *
*                                                                    *
**********************************************************************
*/

#include ".\i2c.h"
#include ".\i2c_gpio.h"

/*********************************************************************
*
*       Configuration, default values
*
**********************************************************************
*/

#define I2C_GPIO_SDA_HIGH(ptDev)        xhal_gpio_set_by_port(ptDev->chSdaPinPort, GPIO_LEVEL_HIGH)
#define I2C_GPIO_SDA_LOW(ptDev)         xhal_gpio_set_by_port(ptDev->chSdaPinPort, GPIO_LEVEL_LOW)

#define I2C_GPIO_SCL_HIGH(ptDev)        xhal_gpio_set_by_port(ptDev->chSclPinPort, GPIO_LEVEL_HIGH)
#define I2C_GPIO_SCL_LOW(ptDev)         xhal_gpio_set_by_port(ptDev->chSclPinPort, GPIO_LEVEL_LOW)

#define I2C_GPIO_USER_DEALY(ptDev)      i2c_gpio_delay(ptDev)

#define I2C_GPIO_SDA_VAL_GET(ptDev)     __xhal_get_gpio_val(ptDev->chSdaPinPort)
#define I2C_GPIO_SCL_VAL_GET(ptDev)     __xhal_get_gpio_val(ptDev->chSclPinPort)

#define __BIT_ISSET(__val, __i)         ((__val) & (1u << (__i)))            
/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

static inline int8_t __xhal_get_gpio_val(uint8_t chPort)
{
    uint32_t wGpioValue = 0;

    /*! no safety */
    xhal_gpio_get_by_port(chPort, &wGpioValue);
    return (int8_t)(wGpioValue > 0);
}

static inline void __xhal_gpio_hw_init(const i2c_gpio_dev_t *ptDev)
{
    xhal_gpio_init_by_port(ptDev->chSclPinPort);
    xhal_gpio_init_by_port(ptDev->chSdaPinPort);
}

static inline void i2c_gpio_delay(const i2c_gpio_dev_t *ptDev)
{
    volatile uint32_t wCount = ptDev->wSpeed;

    while(wCount--);
}

/**
 * @brief       Generate a start signal
 * \param[in]
 * \return      None
 */
void i2c_gpio_start(const i2c_gpio_dev_t *ptDev)
{
    I2C_GPIO_SDA_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SCL_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SDA_LOW(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SCL_LOW(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);
}

/**
 * \brief       Generate a stop signal
 * \param[in]
 * \return      None
 */
void i2c_gpio_stop(const i2c_gpio_dev_t *ptDev)
{
    I2C_GPIO_SDA_LOW(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SCL_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SDA_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);
}

/**
 * \brief       Get response status
 * \param[in]
 * \return      response status
 */
bool i2c_gpio_ack_get(const i2c_gpio_dev_t *ptDev)
{
    bool bAcked = false;

    I2C_GPIO_SDA_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SCL_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    bAcked = (I2C_GPIO_SDA_VAL_GET(ptDev) == 0);

    I2C_GPIO_SCL_LOW(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    return bAcked;
}

/**
 * \brief       Get response status
 * \param[in]
 * \param[in]   true send ack, false send nack
 * \return      response status
 */
void i2c_gpio_ack_put(const i2c_gpio_dev_t *ptDev, bool bAck)
{
    bAck ? I2C_GPIO_SDA_LOW(ptDev) : I2C_GPIO_SDA_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    I2C_GPIO_SCL_HIGH(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);
    
    I2C_GPIO_SCL_LOW(ptDev);
    I2C_GPIO_USER_DEALY(ptDev);

    if (bAck) {
        bAck ? I2C_GPIO_SDA_HIGH(ptDev) : 0;
        I2C_GPIO_USER_DEALY(ptDev);
    }
}

/**
 * \brief       Write a byte
 * \param[in]
 * \param[in]
 * \return      
 */
bool i2c_gpio_write_byte(const i2c_gpio_dev_t *ptDev, uint8_t chData)
{
    for (int8_t i = 7; i >= 0; i--) {

        __BIT_ISSET(chData, i) ? I2C_GPIO_SDA_HIGH(ptDev) : I2C_GPIO_SDA_LOW(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);

        I2C_GPIO_SCL_HIGH(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);

        I2C_GPIO_SCL_LOW(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);
    }
    
    return i2c_gpio_ack_get(ptDev);
}

/**
 * \brief       Read a byte
 * \param[in]
 * \return      
 */
uint8_t i2c_gpio_read_byte(const i2c_gpio_dev_t *ptDev)
{
    uint8_t chData = 0x00;

    for (int8_t i = 0; i < 8; i++) {
        chData <<= 1;
        I2C_GPIO_SCL_HIGH(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);
        
        // chData = I2C_GPIO_SDA_VAL_GET(ptDev) > 0;
        if (I2C_GPIO_SDA_VAL_GET(ptDev)) {
            chData += 1;
        }
        
        I2C_GPIO_SCL_LOW(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);
    }

    return chData;
}

/**
 * \brief       Bus release
 * \param[in]
 * \return      
 */
bool i2c_gpio_bus_release(const i2c_gpio_dev_t *ptDev)
{
    if (I2C_GPIO_SDA_VAL_GET(ptDev)) {
        return true;
    }

    for (int8_t i = 0; i < 9; i++) {
        I2C_GPIO_SCL_LOW(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);

        if (I2C_GPIO_SDA_VAL_GET(ptDev)) {
            return true;
        }

        I2C_GPIO_SCL_HIGH(ptDev);
        I2C_GPIO_USER_DEALY(ptDev);
    }

    return false;
}

bool i2c_gpio_init(const i2c_gpio_dev_t *ptDev)
{
    __xhal_gpio_hw_init(ptDev);

    return true;
}

/*************************** End of file ****************************/
