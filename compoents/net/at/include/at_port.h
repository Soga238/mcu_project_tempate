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
*       at_port.h *                                                  *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_PORT_H__
#define __AT_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include "..\include\at_client.h"

/* Exported constants ----------------------------------------------*/
/* Exported functions --------------------------------------------- */
extern at_client_t at_client_get(int32_t nPort);
extern int32_t at_client_get_char(at_client_t client, uint8_t *pchByte);
extern int32_t at_client_obj_send(at_client_t client, uint8_t *pchBuf, int32_t nLength);

#ifdef __cplusplus
}
#endif
#endif
/*************************** End of file ****************************/
