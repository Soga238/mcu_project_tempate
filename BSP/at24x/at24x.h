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
*       at24x.h *                                                    *
*                                                                    *
**********************************************************************
*/
#ifndef __BSP_AT24X_H__
#define __BSP_AT24X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\bsp_cfg.h"

extern bool at24x_init(void);
extern bool at24x_write_bytes(uint16_t hwAddress, uint8_t *pchData, uint16_t hwSize);
extern bool at24x_read_bytes(uint16_t hwAddress, uint8_t *pchDst, uint16_t hwSize);
extern bool at24x_device_poll(void);

#ifdef __cplusplus
    }
#endif

#endif

/*************************** End of file ****************************/
