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
*       at_assert.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_ASSERT_H__
#define __AT_ASSERT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\at_cfg.h"

#undef ASSERT
#if AT_FULL_ASSERT
#define ASSERT(expr)  ((expr) ? (void)0 : at_assert_failed((uint8_t *)#expr, (uint8_t *)__FILE__, __LINE__))
extern void at_assert_failed(uint8_t *expr, uint8_t *file, uint32_t line);
#else
#define ASSERT(expr)    ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
