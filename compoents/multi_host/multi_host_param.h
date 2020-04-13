#ifndef MULTI_HOST_PARAM_H
#define MULTI_HOST_PARAM_H

#include "./multi_host_cfg.h"

#define ADDRESS_MAP_NUM     1
#define MODBUS_CMD_MUM      8    /*! modbus 命令, 数据源或者目的地其中至少有一个是内部从机*/

typedef struct {
    bool        bWriteExpiration;       // 写入数据，则该从机对应的数据失效
    bool        bInternalSlave;         // 内部从机
    uint16_t    hwCacheExpirationTime;  // 内部缓存失效时间 0ms ~ 60s 0表示不缓存
    uint16_t    hwInstructionInterval;  // 指令执行间隔
} proxy_setting_t;

typedef struct {
    uint32_t    wBaudRate;              // 波特率
    uint16_t    hwWordLength;            // 数据长度
    uint16_t    hwStopBits;             // 停止
    uint16_t    hwParity;               // 奇偶校验位
} serial_config_t;

typedef enum {
    MODBUS_RTU = 0,                     // 默认RTU
    MODBUS_ASCII,                       // 不支持
    MODBUS_TCP                          // 不支持
} modbus_format_t;

typedef enum {
    MASTER = 0,                         // 主机 - 接从机设备
    SLAVE,                              // 从机 - 接主机设备
    MONITOR                             // 监控 - 并入总线网络
} port_role_t;

typedef struct {
    uint8_t     chPort;                 // 端口号
    modbus_format_t tFormat;            // MODBUS RTU OR ASCII
    uint16_t    hwTimeout;              // 端口通讯超时（等待回包最大时间）
    serial_config_t tConfig;            // 端口串口硬件参数设置
} port_config_t;

typedef struct {
    uint8_t chPort;
    port_role_t     tRole;              // 0 Master 1 Slave 2 Monitor/Listenr
    struct {
        uint8_t chSlaveStart;           // 从机地址起始    默认0
        uint8_t chSlaveEnd;             // 从机地址结束    默认0
        uint8_t chIdStart;              // 内部地址开始   默认0
        uint8_t chIdEnd;                // 内部地址结束   默认0
    } tBuf[ADDRESS_MAP_NUM];            // 从机地址和内部地址的映射关系表
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
