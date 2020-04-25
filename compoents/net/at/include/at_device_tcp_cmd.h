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

#ifndef __AT_DEVICE_TCP_CMD_H__
#define __AT_DEVICE_TCP_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "..\at_cfg.h"
#include "..\include\at_client.h"

typedef struct {
    uint8_t     chCCIDBuf[CCID_BUF_SIZE + 1];   // CCID长度20个字符
    uint8_t     chVerBuf[VERSION_BUF_SIZE + 1];

    uint32_t    wTcpState;     // 数值必须使用4字节，vsscanf中的 %d 按照4字节记算
    uint32_t    wCSQSignal;     // 同上
    uint32_t    wCREGQ;         // 同上
    uint32_t    wCGATT;         // 同上
    
    uint32_t    wCellId;
    uint32_t    wLac;
} gsm_param_t;

extern bool at_cmd_phy_connect(struct at_client *ptClient);

extern bool at_cmd_get_sim_info(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_ip(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_work_state(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_tcp_state(struct at_client* ptClient, gsm_param_t* ptSimInfo);

extern bool at_cmd_connect_to_server(struct at_client* ptClient);

extern bool at_cmd_set_ip_head_mode(struct at_client* ptClient, int32_t nMode);

extern bool at_cmd_set_tcp_echo_mode(struct at_client* ptClient, int32_t nMode);

extern bool at_cmd_close_tcp(struct at_client* ptClient);

#ifdef __cplusplus
    }
#endif
    
#endif

/*************************** End of file ****************************/
