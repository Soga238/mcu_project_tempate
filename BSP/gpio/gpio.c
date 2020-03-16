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
*       gpio.c *                                                     *
*                                                                    *
**********************************************************************
*/

#include ".\gpio.h"
#include ".\xhal_gpio.h"
#include ".\hal_gpio.h"
/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

extern const gpio_mapping_t c_tGpioMap[TOTAL_GPIO_NUM];

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
int32_t xhal_gpio_init(gpio_dev_t *ptGpio)
{
    /*! init in cubeMX */
    return XHAL_OK;
}

static inline int32_t get_map_pos(uint16_t hwPort)
{
    uint8_t i = 0;

    for (i = 0; (i < TOTAL_GPIO_NUM) && (hwPort != c_tGpioMap[i].hwPort); i++);
    
    return TOTAL_GPIO_NUM == i ? XHAL_FAIL : i;
}

int32_t xhal_gpio_get(gpio_dev_t *ptGpio, int32_t *pnValue)
{
    int32_t nPos = 0;

    if (NULL == ptGpio || NULL == pnValue) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptGpio->chPort);
    if (XHAL_FAIL != nPos) {
        *pnValue = (GPIO_PIN_SET == HAL_GPIO_ReadPin(c_tGpioMap[nPos].pGpioGroup, c_tGpioMap[nPos].hwPin)) ? \
                    GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

int32_t xhal_gpio_set(gpio_dev_t *ptGpio, int32_t nValue)
{
    int32_t nPos = 0;
    GPIO_PinState tPinState;

    if (NULL == ptGpio) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptGpio->chPort);
    if (XHAL_FAIL != nPos) {
        tPinState = GPIO_LEVEL_LOW == nValue ? GPIO_PIN_RESET : GPIO_PIN_SET;
        HAL_GPIO_WritePin(c_tGpioMap[nPos].pGpioGroup, \
                          c_tGpioMap[nPos].hwPin,      \
                          tPinState);
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

int32_t xhal_gpio_toggle(gpio_dev_t *ptGpio)
{
    int32_t nPos = 0;

    if (NULL == ptGpio) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(ptGpio->chPort);
    if (XHAL_FAIL != nPos) {
        HAL_GPIO_TogglePin(c_tGpioMap[nPos].pGpioGroup, c_tGpioMap[nPos].hwPin);
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

/************************* FUNCTION EXTEND **************************/

int32_t xhal_gpio_init_by_port(uint8_t chPort)
{
    /*! init in cubeMX */
    return XHAL_OK;
}

int32_t xhal_gpio_get_by_port(uint8_t chPort, int32_t *pnValue)
{
    int32_t nPos = 0;

    if (NULL == pnValue) {
        return XHAL_FAIL;
    }

    nPos = get_map_pos(chPort);
    if (XHAL_FAIL != nPos) {
        *pnValue = (GPIO_PIN_SET == HAL_GPIO_ReadPin(c_tGpioMap[nPos].pGpioGroup, c_tGpioMap[nPos].hwPin)) ? \
                    GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

int32_t xhal_gpio_set_by_port(uint8_t chPort, int32_t nValue)
{
    int32_t nPos = 0;
    GPIO_PinState tPinState;

    nPos = get_map_pos(chPort);
    if (XHAL_FAIL != nPos) {
        tPinState = GPIO_LEVEL_LOW == nValue ? GPIO_PIN_RESET : GPIO_PIN_SET;
        HAL_GPIO_WritePin(c_tGpioMap[nPos].pGpioGroup, \
                          c_tGpioMap[nPos].hwPin,      \
                          tPinState);
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

int32_t xhal_gpio_toggle_by_port(uint8_t chPort)
{
    int32_t nPos = 0;

    nPos = get_map_pos(chPort);
    if (XHAL_FAIL != nPos) {
        HAL_GPIO_TogglePin(c_tGpioMap[nPos].pGpioGroup, c_tGpioMap[nPos].hwPin);
        return XHAL_OK;
    }

    return XHAL_FAIL;
}

/*************************** End of file ****************************/
