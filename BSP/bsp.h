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
*       bsp.h *                                                      *
*                                                                    *
**********************************************************************
*/
#ifndef __TBSP_H__
#define __TBSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include ".\bsp_cfg.h"

/* Exported constants ----------------------------------------------*/
/* Exported functions --------------------------------------------- */
extern bool bsp_init(void);
extern void bsp_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
/*************************** End of file ****************************/
