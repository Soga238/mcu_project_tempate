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
*       json_cmd.h *                                                 *
*                                                                    *
**********************************************************************
*/
#ifndef __JSON_CMD_H__
#define __JSON_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include ".\json_cmd_cfg.h"

#define JSON_OBJECT     void        /*! JSON string */

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    uint8_t chNameArray[JSON_CMD_NAME_SIZE];
    bool    (*fnHandler)(JSON_OBJECT *object);
} function_list_t;

typedef struct {
    uint8_t chCount;            /*! the actual number of serial commands */
    uint8_t chState;
    uint16_t hwTimeCounter;     
    function_list_t tList[JSON_CMD_NUMBER];
} uartcmd_dev_t;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern bool json_cmd_add(uartcmd_dev_t *ptDev, const uint8_t *pchName, bool (*fnHandler)(void *));
extern bool json_cmd_execute(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
