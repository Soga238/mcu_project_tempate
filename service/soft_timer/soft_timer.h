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

#define MAX_VALUE_32_BIT        (0xFFFFFFFF) /* 32bit���ms��         */
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
    bool                bPeriodic;           /* ���δ���/���ڴ���     */
    uint32_t            wStart;              /* ��ʱ����ʼʱ��        */
    uint32_t            wNow;                /* ��ʱ����ǰʱ��        */
    uint32_t            wElapse;             /* ��ʱ���ѹ�ʱ��        */
    uint32_t            wTimeout;            /* ��ʱ����ʱʱ��        */
    timer_callback_fn   pfTimerCallback;     /* ��ʱ�ص�����          */
    void               *pArg;                /* �ص������Ĳ���        */
} timer_t;

typedef struct _timer_table
{
    timer_t               data;              /* ����ʱ���������      */
    struct _timer_table  *next;              /* ��һ����ʱ������ַ  */
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
