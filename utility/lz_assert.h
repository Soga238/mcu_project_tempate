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
*       lz_assert.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __LZ_ASSERT_H__
#define __LZ_ASSERT_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include ".\lz_types.h"

#if defined(USE_ASSERT)

#define ASSERT(expr)    ((expr) ? (void)0 : assert_handle((uint8_t *)#expr, (uint8_t *)__FILE__, __LINE__))

#define TRACE(expr, ...)   \
        ((expr) ? (void)0 : assert_handle((uint8_t *)#expr, #__VA_ARGS__, (uint8_t *)__FILE__, __LINE__))

#else

#define ASSERT(expr)    ((void)0)

#define TRACE(expr, ...)    ((void)0)
    
#endif
    

extern void assert_handle(uint8_t *expr, uint8_t* file, uint32_t line);

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
