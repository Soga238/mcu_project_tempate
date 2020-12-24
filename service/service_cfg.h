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
*       service_cfg.h *                                              *
*                                                                    *
**********************************************************************
*/
#ifndef __SERVICE_CFG_H__
#define __SERVICE_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\usr_app\usr_app_cfg.h"

//#include "cmsis_os2.h"

//#define ENTER_ATOM_PROTECT()                    \
//    do{                                         \
//        uint32_t wIrqState = __get_PRIMASK();   \
//        __set_PRIMASK(1u);}

//#define EXIT_ATOM_PROTECT() do{__set_PRIMASK(wIrqState);}while(0)

//#define SAFE_OPERATON(code)                     \
//    do{                                         \
//        uint32_t wIrqState = __get_PRIMASK();   \
//        __set_PRIMASK(1u);                      \
//        #code;                                  \
//        __set_PRIMASK(wIrqState);               \
//    }while(0)

//#define RTOS_ATOM_PROTECT(code) do{             \
//    uint32_t wLock = osKernelLock();            \
//    #code;                                      \
//    osKernelRestoreLock(wLock);
//    
//#define RTX_ENTER_ATOM_PROTECT()                \
//        uint32_t wLock = osKernelLock();
//        
//#define RTX_EXIT_ATOM_PROTECT()                 \
//    do{                                         \
//        osKernelRestoreLock(wLock);             \
//    }while(0)
    
#define ENTER_ATOM_PROTECT() ((void)0)
#define EXIT_ATOM_PROTECT() ((void)0)
        
#define RTX_ENTER_ATOM_PROTECT() ((void)0)
#define RTX_EXIT_ATOM_PROTECT() ((void)0)

#ifdef __cplusplus
    }
#endif

#endif

/*************************** End of file ****************************/
