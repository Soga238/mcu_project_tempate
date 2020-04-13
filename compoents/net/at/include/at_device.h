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
*       at_device.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_DEVICE_H__
#define __AT_DEVICE_H__

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
    const uint8_t   *pchClientID;   // 产品名称存储数组
    const uint8_t   *pchUserName;   // 设备名称存储数组
    const uint8_t   *pchPassword;   // 设备密钥存储数组
} at_mqtt_info_t;

typedef struct {
    const uint8_t   *pchCCID;
    const uint8_t   *pchVer;

    uint8_t  chMqttState;
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

extern bool at_device_connect_server(at_mqtt_info_t *ptMqttInfo);

extern int32_t at_device_check_mqtt(void);

extern bool at_device_check_online(void);

extern int32_t at_device_pub_with_buf(uint8_t *pchBuf, uint16_t hwLength, uint32_t wTimeout);

extern bool at_device_get_gsm_param(at_gsm_param_t *ptGsm);

extern_simple_fsm(init_at_device,
                  def_params(
                      uint8_t chErrorCount;
                  ))
extern_fsm_implementation(init_at_device);
extern_fsm_initialiser(init_at_device);

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
