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
*       uartcmd.h *                                                  *
*                                                                    *
**********************************************************************
*/
#ifndef __UARTCMD_H__
#define __UARTCMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include ".\uart_cmd_cfg.h"

#define JSON_OBJECT     void        /*! JSON string */

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    uint8_t chNameArray[UARTCMD_NAME_SIZE];
    bool    (*fnHandler)(JSON_OBJECT *object);
} function_list_t;

typedef struct {
    uint8_t chCount;            /*! the actual number of serial commands */
    uint8_t chState;
    
    uint16_t hwTimeCounter;     

    function_list_t tList[UARTCMD_NUMBER];
    
} uartcmd_dev_t;


/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern bool uartcmd_add_handler(uartcmd_dev_t *ptDev, const uint8_t *pchName, bool (*fnHandler)(void *));
extern bool uartcmd_execute(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength);

/*
extern fsm_rt_t uartcmd_fsm(uartcmd_dev_t *ptThis);
extern void uartcmd_received_notice(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength);
extern bool uartcmd_dev_init(uartcmd_dev_t *ptDev, uartcmd_msg_t *ptBuffer, uint8_t chBufferSize);
*/

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
