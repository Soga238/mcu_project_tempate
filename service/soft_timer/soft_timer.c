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
*       Soft Timer * Soft Timer                                      *
*                                                                    *
**********************************************************************
*/

#include "soft_timer.h"
#include "stdlib.h"

/*********************************************************************
*
*       Global variables
*
**********************************************************************
*/
static uint32_t s_wTimeMaxValue  = MAX_VALUE_32_BIT;

static timer_table_t* s_ptTimerTableHead = NULL;

static timer_tick_fn s_pfGetSysTick = NULL;

static timer_hook_t s_tHooks = {
    malloc,
    free
};

static bool s_bTimerInitOk = false;

/**
 * \brief Initialize the timer control block.
 * \param pfGetSysTick Pointer to the function.
 * \param wMaxTime  Size of character array.
 * \return  true:   Success
 *          false:  Fail
 */
bool soft_timer_init(timer_tick_fn pfGetSysTick, uint32_t wMaxTime)
{
    if (NULL == pfGetSysTick) {
        return false;
    }

    if (s_bTimerInitOk) {
        return true;
    }

    s_ptTimerTableHead = (timer_table_t*)s_tHooks.malloc_fn(sizeof(timer_table_t));
    if (NULL == s_ptTimerTableHead) {
        return false;
    }

    s_ptTimerTableHead->next = NULL;
    s_pfGetSysTick = pfGetSysTick;
    s_wTimeMaxValue = wMaxTime;
    s_bTimerInitOk = true;

    return true;
}

bool soft_timer_init_hook(timer_hook_t *pHooks)
{
    if (NULL == pHooks) {
        return false;
    }

    if (NULL == pHooks->malloc_fn || NULL == pHooks->free_fn) {
        return false;
    }

    s_tHooks.malloc_fn = pHooks->malloc_fn;
    s_tHooks.free_fn = pHooks->free_fn;

    return true;
}

timer_table_t* soft_timer_create(uint32_t wTimeout, bool bPeriodic, timer_callback_fn pfTimerCallback, void *pArg)
{
    timer_table_t* ptTimerNode = NULL;
    timer_table_t* ptFind = NULL;
    if (NULL == s_ptTimerTableHead) {
        return NULL;
    }

    ptTimerNode = (timer_table_t*)s_tHooks.malloc_fn(sizeof(timer_table_t));
    if (NULL == ptTimerNode) {
        return NULL;
    }

    ptTimerNode->next = NULL;
    ptTimerNode->data.bPeriodic = bPeriodic;
    ptTimerNode->data.wStart = s_pfGetSysTick();
    ptTimerNode->data.wNow = ptTimerNode->data.wStart;
    ptTimerNode->data.wElapse = 0;
    ptTimerNode->data.wTimeout = wTimeout;
    ptTimerNode->data.pfTimerCallback = pfTimerCallback;
    ptTimerNode->data.pArg = pArg;

    for (ptFind = s_ptTimerTableHead; NULL != ptFind->next; ptFind=ptFind->next) {
        // do nothing
    }
    ptFind->next = ptTimerNode;

    return ptTimerNode;
}

bool soft_timer_delete(timer_table_t* ptNode)
{
    timer_table_t* ptFind = NULL;

    if (NULL == ptNode) {
        return false;
    }

    for (ptFind = s_ptTimerTableHead; NULL != ptFind; ptFind = ptFind->next) {
        if (ptNode == ptFind->next) {
            ptFind->next = ptNode->next;
            s_tHooks.free_fn(ptNode);
            ptNode = NULL;
            return true;
        }
    }
    return false;
}


bool soft_timer_reset(timer_table_t* ptNode)
{
    if (NULL == ptNode) {
        return false;
    }

    ptNode->data.wStart = s_pfGetSysTick();
    return true;
}

bool soft_timer_process(void)
{
    timer_table_t* ptFind = NULL;
    timer_table_t* ptNodeFree = NULL;

    if (NULL == s_ptTimerTableHead) {
        return false;
    }

    for (ptFind = s_ptTimerTableHead->next; ptFind != NULL;) {
        ptFind->data.wNow = s_pfGetSysTick();

        if(ptFind->data.wNow >= ptFind->data.wStart) {
            ptFind->data.wElapse = ptFind->data.wNow - ptFind->data.wStart;
        } else {
            ptFind->data.wElapse = (s_wTimeMaxValue - ptFind->data.wNow) + (ptFind->data.wStart - 0);
        }

        if(ptFind->data.wElapse >= ptFind->data.wTimeout) {
            if(ptFind->data.pfTimerCallback) {
                ptFind->data.pfTimerCallback(ptFind->data.pArg);
            }

            if(ptFind->data.bPeriodic) {
                soft_timer_reset(ptFind);
            } else {
                ptNodeFree = ptFind;
                ptFind = ptFind->next;
                soft_timer_delete(ptNodeFree);
                continue;
            }
        }
        ptFind = ptFind->next;
    }
    return true;
}

/*************************** End of file ****************************/
