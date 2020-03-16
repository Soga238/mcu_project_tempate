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
*       json_cmd.c *                                                 *
*                                                                    *
**********************************************************************
*/

#include ".\json_cmd.h"
#include "cJSON.h"
#include <string.h>

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

bool json_cmd_add(uartcmd_dev_t *ptDev, const uint8_t *pchName, bool (*fnHandler)(void *))
{
    int32_t nNameLength = 0;

    if ((NULL == ptDev) || (NULL == pchName) || (NULL == fnHandler)) {
        return false;
    }

    if (JSON_CMD_NUMBER <= ptDev->chCount) {
        return false;
    }

    nNameLength = strlen((char *)pchName);
    if ((nNameLength <= 0) || (JSON_CMD_NAME_SIZE <= nNameLength)) {
        return false;
    }

    memcpy(ptDev->tList[ptDev->chCount].chNameArray, pchName, nNameLength);
    ptDev->tList[ptDev->chCount].fnHandler = fnHandler;
    ptDev->chCount += 1;

    return true;
}

static bool __execute(uartcmd_dev_t *ptDev, const uint8_t *pchName, void *pArg)
{
    for (uint8_t i = 0; i < ptDev->chCount; i++) {
        if (0 == strcmp((char *)pchName, (char *)ptDev->tList[i].chNameArray)) {
            if (NULL != ptDev->tList[i].fnHandler) {
                return ptDev->tList[i].fnHandler(pArg);
            }
        }
    }
    return false;
}

bool json_cmd_execute(uartcmd_dev_t *ptDev, uint8_t *pchBuf, uint16_t hwLength)
{
    cJSON *pRoot = NULL;
    cJSON *ptFuncObj = NULL;

    if (NULL == pchBuf || 0 == hwLength) {
        return false;
    }

    if (NULL != (pRoot = cJSON_Parse((char *)pchBuf))) {
        ptFuncObj = cJSON_GetObjectItem(pRoot, "func");
        if (cJSON_IsString(ptFuncObj)) {
            if (__execute(ptDev, (uint8_t *)ptFuncObj->valuestring, pRoot)) {
                cJSON_Delete(pRoot);
                return true;
            }
        }
    }

    cJSON_Delete(pRoot);
    return false;
}

/*************************** End of file ****************************/
