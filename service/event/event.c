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
 * \brief       �ȴ�EVENT����
 * \param[in]   ptEvent     EVENT���ݽṹ��
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
 * \brief       ����EVENT
 * \param[in]   ptEvent     EVENT���ݽṹ��
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
 * \brief       ��ʼ��EVENT�ṹ��
 * \param[in]   ptEvent     EVENT���ݽṹ��
 * \param[in]   bIsAutoReset    �Զ���λ����
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
 * \brief       ��λEVENT��δ����״̬
 * \param[in]   ptEvent     EVENT���ݽṹ��
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
