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
*       at_utils.h *                                                 *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_UTILS_H__
#define __AT_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include "..\at_cfg.h"
#include <stdarg.h>

/* Exported constants ----------------------------------------------*/
/* Exported functions --------------------------------------------- */
extern const uint8_t *at_get_last_cmd(int32_t *pnLength);
extern const uint8_t* at_extract_last_cmd(uint8_t* pchBuf, int32_t* pnLength);
extern int32_t at_vprintf(void *_, const char *format, va_list args);

#ifdef __cplusplus
}
#endif
#endif
/*************************** End of file ****************************/
