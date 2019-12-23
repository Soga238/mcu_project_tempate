#ifndef MULTI_HOST_PARAM_H
#define MULTI_HOST_PARAM_H

#include "./multi_host_cfg.h"

#define ADDRESS_MAP_NUM     1
#define MODBUS_CMD_MUM      8    /*! modbus ����, ����Դ����Ŀ�ĵ�����������һ�����ڲ��ӻ�*/

typedef struct {
    bool        bWriteExpiration;       // д�����ݣ���ôӻ���Ӧ������ʧЧ
    bool        bInternalSlave;         // �ڲ��ӻ�
    uint16_t    hwCacheExpirationTime;  // �ڲ�����ʧЧʱ�� 0ms ~ 60s 0��ʾ������
    uint16_t    hwInstructionInterval;  // ָ��ִ�м��
} proxy_setting_t;

typedef struct {
    uint32_t    wBaudRate;              // ������
    uint16_t    hwWordLength;            // ���ݳ���
    uint16_t    hwStopBits;             // ֹͣ
    uint16_t    hwParity;               // ��żУ��λ
} serial_config_t;

typedef enum {
    MODBUS_RTU = 0,                     // Ĭ��RTU
    MODBUS_ASCII,                       // ��֧��
    MODBUS_TCP                          // ��֧��
} modbus_format_t;

typedef enum {
    MASTER = 0,                         // ���� - �Ӵӻ��豸
    SLAVE,                              // �ӻ� - �������豸
    MONITOR                             // ��� - ������������
} port_role_t;

typedef struct {
    uint8_t     chPort;                 // �˿ں�
    modbus_format_t tFormat;            // MODBUS RTU OR ASCII
    uint16_t    hwTimeout;              // �˿�ͨѶ��ʱ���ȴ��ذ����ʱ�䣩
    serial_config_t tConfig;            // �˿ڴ���Ӳ����������
} port_config_t;

typedef struct {
    uint8_t chPort;
    port_role_t     tRole;              // 0 Master 1 Slave 2 Monitor/Listenr
    struct {
        uint8_t chSlaveStart;           // �ӻ���ַ��ʼ    Ĭ��0
        uint8_t chSlaveEnd;             // �ӻ���ַ����    Ĭ��0
        uint8_t chIdStart;              // �ڲ���ַ��ʼ   Ĭ��0
        uint8_t chIdEnd;                // �ڲ���ַ����   Ĭ��0
    } tBuf[ADDRESS_MAP_NUM];            // �ӻ���ַ���ڲ���ַ��ӳ���ϵ��
}port_resource_t;

extern const uint8_t g_chPortBuf[TOTAL_PORT_NUM];

extern void multi_host_param_init(void);

extern const proxy_setting_t *proxy_get_global_setting(void);

extern const port_resource_t *proxy_get_port_resource(uint8_t chPort);

extern const port_config_t *proxy_get_port_config(uint8_t chPort);

extern uint8_t proxy_get_master_port_num(void);

extern uint8_t proxy_get_slave_port_num(void);

extern const port_resource_t *proxy_get_master_port_resource(uint8_t chIndex);

extern const port_resource_t *proxy_get_slave_port_resource(uint8_t chIndex);

#endif
