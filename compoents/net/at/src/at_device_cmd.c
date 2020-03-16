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
*       at_device_cmd.c *                                            *
*                                                                    *
**********************************************************************
*/

#include "..\include\at_device_cmd.h"
#include "..\include\at_client.h"
#include "..\..\service\heap\heap.h"

#include <stdio.h>
#include <string.h>

/*********************************************************************
*
*       Configuration, default values
*
**********************************************************************
*/

#define ATCMD_ECHO_OFF              "ATE0\r\n"

#define ATCMD_SIM_CCID              "AT+CCID\r\n"
#define ATCMD_SIM_CGMR              "AT+CGMR\r\n"

#define ATCMD_CLOSE_SHUT            "AT+CIPSHUT\r\n"
#define ATCMD_SET_CMNET             "AT+CSTT=\"CMNET\"\r\n"
#define ATCMD_GET_IP                "AT+CIICR\r\n"
#define ATCMD_CHECK_IP              "AT+CIFSR\r\n"

#define ATCMD_CHECK_CSQ             "AT+CSQ\r\n"
#define ATCMD_CHECK_CGATT           "AT+CGATT?\r\n"
#define ATCMD_CHECK_MQTT_STATE      "AT+MQTTSTATU\r\n"

#define ATCMD_TCP                   "AT+MIPSTART=\"mqtt.heclouds.com\",\"6002\"\r\n"
#define ATCMD_ONENET                "AT+MCONNECT=1,60\r\n"

#define ATCMD_CHECK_NPTM            "AT%NTPM=1\r\n"

#define __AT_EXECUTE(__client, __resp, __macro_str) \
        at_obj_exec_cmd_with_buf(__client, __resp, (uint8_t *)__macro_str, strlen(__macro_str))
#define AT_EXECUTE(__macro_str) __AT_EXECUTE(ptClient, ptResp, __macro_str)

#define CONNECT_COUNT_MAX           8u

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
bool at_cmd_phy_connect(struct at_client *ptClient)
{
    int8_t i = 0;
    struct at_response *ptResp = at_create_resp(128, 2, 1000);

    if (NULL == ptResp) {
        goto __exit;
    }

    for (i = 0; i < CONNECT_COUNT_MAX; i++) {
        if (RT_EOK != AT_EXECUTE(ATCMD_ECHO_OFF)) {
            break;
        }
    }

__exit:
    at_delete_resp(ptResp);
    return CONNECT_COUNT_MAX == i;
}

bool at_cmd_get_sim_info(struct at_client *ptClient, gsm_param_t *ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 4, 1000);

    if (NULL == ptResp) {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_SIM_CGMR)) {
        if (1 != at_resp_parse_line_args(ptResp, 2, "%20s", ptSimInfo->chVerBuf)) {
            goto __exit;
        }
    } else {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_SIM_CCID)) {
        if (1 != at_resp_parse_line_args(ptResp, 2, "%20s", ptSimInfo->chCCIDBuf)) {
            goto __exit;
        }

        if (18 >= strlen((char *)ptSimInfo->chCCIDBuf)) {
            goto __exit;
        }

    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_get_ip(struct at_client *ptClient, gsm_param_t *ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 2, 1000);

    if (NULL == ptResp) {
        goto __exit;
    }

    if (RT_EOK != AT_EXECUTE(ATCMD_CLOSE_SHUT)) {
        goto __exit;
    }

    if (RT_EOK != AT_EXECUTE(ATCMD_SET_CMNET)) {
        goto __exit;
    }

    at_resp_set_info(ptResp, 128, 2, 5000);

    if (RT_EOK != AT_EXECUTE(ATCMD_GET_IP)) {
        ALOG_I("%s", at_resp_get_line(ptResp, 2));
        goto __exit;
    }

    at_resp_set_info(ptResp, 128, 2, 1000);

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_IP)) {
        ALOG_I("%s", at_resp_get_line(ptResp, 2));
    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_get_work_state(struct at_client *ptClient, gsm_param_t *ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 4, 1000);

    if (NULL == ptResp) {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_CSQ)) {
        if (1 != at_resp_parse_line_args(ptResp, 2, "+CSQ: %d, %*d", &ptSimInfo->wCSQSignal)) {
            goto __exit;
        }
    } else {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_CGATT)) {
        if (1 != at_resp_parse_line_args(ptResp, 2, "+CGATT: %d", &ptSimInfo->wCGATT)) {
            goto __exit;
        }
    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_get_mqttstate(struct at_client *ptClient, uint32_t *pchState)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 2, 2000);

    if (NULL == ptResp) {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_MQTT_STATE)) {
        if (1 != at_resp_parse_line_args(ptResp, 2, "+MQTTSTATU :%d", pchState)) {
            goto __exit;
        }
    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_connect_onenet(struct at_client *ptClient, const uint8_t *pchUser, const uint8_t *pchClientId, const uint8_t *pchPwd)
{
    bool bRet = false;
    uint8_t *pchBuf = NULL;
    int32_t nBytes = 0;
    struct at_response *ptResp = NULL;

    if ((NULL == ptClient) || (NULL == pchUser) || (NULL == pchClientId) ||
        (NULL == pchPwd)) {
        return false;
    }

    if (NULL == (ptResp = at_create_resp(128, 2, 1000))) {
        goto __exit;
    }

    if (NULL == (pchBuf = port_malloc_4(128))) {
        goto __exit;
    }

    nBytes = snprintf((char *)pchBuf, 128, "AT+MCONFIG=\"%s\",\"%s\",\"%s\"\r\n",
                      (char *)pchClientId,
                      (char *)pchUser,
                      (char *)pchPwd);
    if (nBytes <= 0) {
        goto __exit;
    }

    if (RT_EOK != AT_EXECUTE((char *)pchBuf)) {
        goto __exit;
    }

    at_resp_set_info(ptResp, 128, 3, 6000);

    if (RT_EOK != AT_EXECUTE(ATCMD_TCP)) {
        goto __exit;
    }

    if (RT_EOK != AT_EXECUTE(ATCMD_ONENET)) {
        goto __exit;
    }

    bRet = true;
__exit:
    port_free_4(pchBuf);
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_get_nptm(struct at_client *ptClient, gsm_param_t *ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 3, 2000);

    if (NULL == ptResp) {
        goto __exit;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_NPTM)) {
        if (2 != at_resp_parse_line_args(ptResp, 2, "%*5s:%*d,%*d,%*d,%d,%d", &ptSimInfo->wCellId, &ptSimInfo->wLac)) {
            goto __exit;
        }
    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

/*************************** End of file ****************************/
