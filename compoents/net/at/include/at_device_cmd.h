#ifndef __AT_DEVICE_CMD_H__
#define __AT_DEVICE_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "..\at_cfg.h"
#include "..\include\at_client.h"

typedef struct {
    uint8_t     chCCIDBuf[CCID_BUF_SIZE + 1];   // CCID����20���ַ�
    uint8_t     chVerBuf[VERSION_BUF_SIZE + 1];

    uint32_t    wMqttState;     // ��ֵ����ʹ��4�ֽڣ�vsscanf�е� %d ����4�ֽڼ���
    uint32_t    wCSQSignal;     // ͬ��
    uint32_t    wCREGQ;         // ͬ��
    uint32_t    wCGATT;         // ͬ��
    
    uint32_t    wCellId;
    uint32_t    wLac;
} gsm_param_t;

extern bool at_cmd_phy_connect(struct at_client *ptClient);

extern bool at_cmd_get_sim_info(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_ip(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_work_state(struct at_client *ptClient, gsm_param_t *ptSimInfo);

extern bool at_cmd_get_mqttstate(struct at_client *ptClient, uint32_t *pchState);

extern bool at_cmd_connect_onenet(struct at_client *ptClient,
                                  const uint8_t *pchUser,
                                  const uint8_t *pchClientId,
                                  const uint8_t *pchPwd);
                                  
extern bool at_cmd_get_nptm(struct at_client *ptClient, gsm_param_t *ptSimInfo);

#ifdef __cplusplus
    }
#endif
    
#endif