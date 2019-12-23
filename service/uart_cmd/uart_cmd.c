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
*       uartcmd.c *                                                  *
*                                                                    *
**********************************************************************
*/

#include ".\uart_cmd.h"

#include "cJSON.h"
#include <string.h>

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

bool uartcmd_add_handler(uartcmd_dev_t *ptDev, const uint8_t *pchName, bool (*fnHandler)(void *))
{
    uint8_t chNameLenght = 0;

    if ((NULL == ptDev) || (NULL == pchName) || (NULL == fnHandler)) {
        return false;
    }

    if (UARTCMD_NUMBER <= ptDev->chCount) {
        return false;
    }

    chNameLenght = strlen((char *)pchName);
    if (UARTCMD_NAME_SIZE <= chNameLenght) {
        return false;
    }

    memcpy(ptDev->tList[ptDev->chCount].chNameArray, pchName, chNameLenght);
    ptDev->tList[ptDev->chCount].fnHandler = fnHandler;
    ptDev->chCount += 1;

    return true;
}

static bool handler_list_process(uartcmd_dev_t *ptDev, const uint8_t *pchName, void *pArg)
{
    for (uint8_t i = 0; i < ptDev->chCount; i++) {
        if (NULL != strstr((char *)pchName, (char *)ptDev->tList[i].chNameArray)) {
            if (NULL != ptDev->tList[i].fnHandler) {
                return ptDev->tList[i].fnHandler(pArg);
            }
        }
    }
    return false;
}

bool uartcmd_execute(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength)
{
    cJSON *pRoot = NULL;
    cJSON *ptFuncObj = NULL;

    if (NULL == pchBuf || 0 == hwLength) {
        return false;
    }

    if (NULL != (pRoot = cJSON_Parse((char *)pchBuf))) {
        ptFuncObj = cJSON_GetObjectItem(pRoot, "func");
        if (NULL != ptFuncObj && cJSON_IsString(ptFuncObj)) {
            if (handler_list_process(ptDev, (uint8_t *)ptFuncObj->valuestring, pRoot)) {
                cJSON_Delete(pRoot);
                return true;
            }
        }
    }

    cJSON_Delete(pRoot);
    return false;
}

/*
#define USERCMD_SET_RESET_FSM() do{ptThis->chState = START;}while(0)
fsm_rt_t uartcmd_fsm(uartcmd_dev_t *ptThis)
{
    enum {
        START = 0,
        CHECK_QUEUE,
        EXECUTE_CMD
    };

    uartcmd_msg_t tMsg;

    if (NULL == ptThis) {
        return fsm_rt_err;
    }

    switch (ptThis->chState) {
    case START:
        uartcmd_queue_clr(&ptThis->tQueue);
        ptThis->chState = CHECK_QUEUE;

    // fall through
    case CHECK_QUEUE:
        if (uartcmd_dequeue(&ptThis->tQueue, &tMsg)) {
            ptThis->hwTimeCounter = 0;
            ptThis->chState = EXECUTE_CMD;
        } else {
            USERCMD_SET_RESET_FSM();
            return fsm_rt_cpl;
        }

    // fall through
    case EXECUTE_CMD:
        handle_cmds(ptThis, tMsg.pchMsgBuf, tMsg.hwMsgSize);
        ptThis->chState = CHECK_QUEUE;
        break;

    default:
        return fsm_rt_err;
    }

    return fsm_rt_on_going;
}

bool uartcmd_dev_init(uartcmd_dev_t *ptDev, uartcmd_msg_t *ptBuffer, uint8_t chBufferSize)
{
    enum {
      START = 0,  
    };
    
    if (NULL != ptDev) {
        memset(ptDev, 0x00, sizeof(uartcmd_dev_t));
        ptDev->chState = START;
        if (uartcmd_queue_init(&ptDev->tQueue, ptBuffer, chBufferSize)) { 
            return true;
        }
    }
    return false;
}

void uartcmd_received_notice(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength)
{
    uartcmd_msg_t tMsg = {
        .pchMsgBuf = pchBuf,
        .hwMsgSize = hwLength
    };
    
    uartcmd_enqueue(&ptDev->tQueue, &tMsg);
}

*/

/*************************** End of file ****************************/
