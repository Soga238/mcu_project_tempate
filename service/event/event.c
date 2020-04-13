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
*       event.c *                                                    *
*                                                                    *
**********************************************************************
*/

#include ".\event.h"

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/


/**
 * \brief       等待EVENT触发
 * \param[in]   ptEvent     EVENT数据结构体
 * \return      true
 *              false
 */
bool wait_event(event_t *ptEvent)
{
    if (NULL != ptEvent) {
        if (ptEvent->bIsSet) {
            if (ptEvent->bAutoReset) {
                ptEvent->bIsSet = false;
            }
            return true;
        }
    }
    return false;
}

/**
 * \brief       触发EVENT
 * \param[in]   ptEvent     EVENT数据结构体
 * \return      true
 *              false
 */
bool set_event(event_t *ptEvent)
{
    if (NULL != ptEvent) {
        ptEvent->bIsSet = true;
        return true;
    }
    return false;
}

/**
 * \brief       初始化EVENT结构体
 * \param[in]   ptEvent     EVENT数据结构体
 * \param[in]   bIsAutoReset    自动复位开关
 * \return      true
 *              false
 */
bool init_event(event_t *ptEvent, bool bValue, bool bManual)
{
    if (NULL == ptEvent) {
        return false;
    }

    if (bManual) {
        ptEvent->bAutoReset = false;
    } else {
        ptEvent->bAutoReset = true;
    }
    ptEvent->bIsSet = bValue;
    return true;
}

/**
 * \brief       复位EVENT到未触发状态
 * \param[in]   ptEvent     EVENT数据结构体
 * \return      true
 *              false
 */

bool reset_event(event_t *ptEvent)
{
    if (NULL != ptEvent) {
        ptEvent->bIsSet = false;
        return true;
    }
    return false;
}


/*************************** End of file ****************************/
