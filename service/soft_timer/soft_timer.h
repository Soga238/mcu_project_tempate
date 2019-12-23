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
#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include "..\service_cfg.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define MAX_VALUE_32_BIT        (0xFFFFFFFF) /* 32bit最大ms数         */
#define MAX_VALUE_16_BIT        (0xFFFF)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int32_t  (*timer_callback_fn)(void *pArg);
typedef uint32_t (*timer_tick_fn)(void);

typedef struct _timer
{
    bool                bPeriodic;           /* 单次触发/周期触发     */
    uint32_t            wStart;              /* 计时器起始时间        */
    uint32_t            wNow;                /* 计时器当前时间        */
    uint32_t            wElapse;             /* 计时器已过时间        */
    uint32_t            wTimeout;            /* 计时器计时时间        */
    timer_callback_fn   pfTimerCallback;     /* 计时回掉函数          */
    void               *pArg;                /* 回调函数的参数        */
} timer_t;

typedef struct _timer_table
{
    timer_t               data;              /* 本计时器结点数据      */
    struct _timer_table  *next;              /* 下一个定时器结点地址  */
} timer_table_t;

typedef struct _malloc_hook
{
      void *(*malloc_fn)(uint32_t sz);      // pointer of malloc
      void (*free_fn)(void *ptr);           // pointer of free
} timer_hook_t;

/*********************************************************************
*
*       Extern API
*
**********************************************************************
*/
extern bool soft_timer_init(timer_tick_fn pfGetSysTick, uint32_t wMaxTime);
extern bool soft_timer_init_hook(timer_hook_t *pHooks);
extern timer_table_t* soft_timer_create(uint32_t wTimeout, bool bPeriodic, timer_callback_fn pFunc, void *pArg);
extern bool soft_timer_delete(timer_table_t* ptNode);
extern bool soft_timer_reset(timer_table_t* ptNode);
extern bool soft_timer_process(void);

#endif

/* end of file*/
