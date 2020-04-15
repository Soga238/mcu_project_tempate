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
*       heap.h *                                                     *
*                                                                    *
**********************************************************************
*/
#ifndef __HEAP_H__
#define __HEAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\service_cfg.h"

#include "cmsis_os2.h"
    
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define HEAP_LOG_RAW(fmt)

#define CONFIG_TOTAL_HEAP_SIZE          (GLOBAL_HEAP_SIZE)

#define PORT_BYTE_ALIGNMENT             (8)
#if PORT_BYTE_ALIGNMENT == 8
#   define PORT_BYTE_ALIGNMENT_MASK     (0x0007)
#elif PORT_BYTE_ALIGNMENT == 4
#   define PORT_BYTE_ALIGNMENT_MASK     (0x0003)
#endif

#define ENTER_SECDULE_PROTECT           osKernelLock
#define EXIT_SECDULE_PROTECT            osKernelUnlock

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern void     *port_malloc_4(uint32_t wSize);
extern void      port_free_4(void *pMemory);
extern uint32_t  port_get_free_heap_size_4(void);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
