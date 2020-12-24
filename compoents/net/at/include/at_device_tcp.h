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
*       at_device_tcp.h *                                            *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_DEVICE_TCP_H__
#define __AT_DEVICE_TCP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\at_cfg.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    const uint8_t   *pchCCID;
    const uint8_t   *pchVer;

    uint8_t  chTcpState;
    uint8_t  chCSQSignal;

    uint32_t wCid;
    uint32_t wLoc;
} at_gsm_param_t;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern bool at_device_init(void);

extern bool at_device_open_tcp(void);

extern bool at_device_close_tcp(void);

extern bool at_device_check_status(void);

extern int32_t at_device_tcp_send(const uint8_t* pchBuf, uint16_t hwLength, uint32_t wTimeout);

extern int32_t at_device_tcp_recv(uint8_t* pchBuf, uint16_t hwLength, uint32_t wTimeout);

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
