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
*       at_device_tcp_cmd.c *                                        *
*                                                                    *
**********************************************************************
*/

#include "..\include\at_device_tcp_cmd.h"
#include "..\include\at_client.h"
#include "..\service\heap\heap.h"

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
#define ATCMD_CHECK_TCP_STATE       "AT+CIPSTATUS\r\n"

#define ATCMD_TCP                   "AT+CIPSTART=\"TCP\",\"www.cniot.ltd\",\"8888\"\r\n"
#define ATCMD_CHECK_NPTM            "AT%NTPM=1\r\n"

#define __AT_EXECUTE(__client, __resp, __macro_str) \
        at_obj_exec_cmd_with_buf(__client, __resp, (uint8_t *)__macro_str, strlen(__macro_str))
#define AT_EXECUTE(__macro_str) __AT_EXECUTE(ptClient, ptResp, __macro_str)

#define PHY_CONNECT_COUNT_MAX           8
#define PHY_CONNECT_COUNT_SET           3

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
bool at_cmd_phy_connect(struct at_client *ptClient)
{
    int8_t i, j;
    struct at_response *ptResp = at_create_resp(128, 2, 1000);

    if (NULL == ptResp) {
        return false;
    }

    /*! 连续发送8个ATE0 确保与AT设备的通讯连接正常*/
    for (i = 0, j = 0; i < PHY_CONNECT_COUNT_MAX; i++) {
        if (RT_EOK == AT_EXECUTE(ATCMD_ECHO_OFF)) {
            ++j;
        } else {
            j = 0;
        }
    }

__exit:
    at_delete_resp(ptResp);
    return j > PHY_CONNECT_COUNT_SET;
}

bool at_cmd_get_sim_info(struct at_client *ptClient, gsm_param_t *ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 4, 1000);

    if (NULL == ptResp) {
        return false;
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
        return false;
    }

    if (RT_EOK != AT_EXECUTE(ATCMD_CLOSE_SHUT)) {
        goto __exit;
    }

    if (RT_EOK != AT_EXECUTE(ATCMD_SET_CMNET)) {
        goto __exit;
    }

    at_resp_set_info(ptResp, 128, 2, 5000);

    if (RT_EOK != AT_EXECUTE(ATCMD_GET_IP)) {
        goto __exit;
    }

//    at_resp_set_info(ptResp, 128, 2, 2000);

//    if (RT_EOK != AT_EXECUTE(ATCMD_CHECK_IP)) {
//        ALOG_I("%s\r\n", at_resp_get_line(ptResp, 2));
//    } else {
//        goto __exit;
//    }

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
        return false;
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

bool at_cmd_get_tcp_state(struct at_client *ptClient, gsm_param_t* ptSimInfo)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 4, 1000);

    if (NULL == ptResp) {
        return false;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_CHECK_TCP_STATE)) {
        bRet = true;
        if (NULL != at_resp_get_line_by_kw(ptResp, "CONNECT OK")) {
            ptSimInfo->wTcpState = 1;
        } else {
            ptSimInfo->wTcpState = 0;
        }
    } else {
        goto __exit;
    }

__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_set_ip_head_mode(struct at_client* ptClient, int32_t nMode)
{
    bool bRet = false;
    struct at_response* ptResp = at_create_resp(128, 2, 1000);

    if (NULL == ptResp) {
        return false;
    }

    // 显示IP头 +IPD:,data length:
    if (RT_EOK != at_obj_exec_cmd(ptClient, ptResp, "AT+CIPHEAD=%d\r\n", nMode)) {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_set_tcp_echo_mode(struct at_client* ptClient, int32_t nMode)
{
    bool bRet = false;
    struct at_response* ptResp = at_create_resp(128, 2, 1000);

    if (NULL == ptResp) {
        return false;
    }

    // 关闭 > 返回
    if (RT_EOK != at_obj_exec_cmd(ptClient, ptResp, "AT+CIPSPRT=%d\r\n", nMode)) {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_connect_to_server(struct at_client *ptClient)
{
    bool bRet = false;
    struct at_response *ptResp = at_create_resp(128, 4, 6000);

    if (NULL == ptResp) {
        return false;
    }

    if (RT_EOK == AT_EXECUTE(ATCMD_TCP)) {
        if (NULL == at_resp_get_line_by_kw(ptResp, "CONNECT OK")) {
            if (NULL == at_resp_get_line_by_kw(ptResp, "ALREADY CONNECT")) {
                goto __exit;
            }
        }
    } else {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

bool at_cmd_close_tcp(struct at_client* ptClient)
{
    bool bRet = false;
    struct at_response* ptResp = at_create_resp(128, 2, 2000);

    if (NULL == ptResp) {
        return false;
    }

    if (RT_EOK != AT_EXECUTE("AT+CIPCLOSE\r\n")) {
        goto __exit;
    }

    bRet = true;
__exit:
    at_delete_resp(ptResp);
    return bRet;
}

/*************************** End of file ****************************/
