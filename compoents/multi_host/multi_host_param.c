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
*       multi_host_param.h *                                         *
*                                                                    *
**********************************************************************
*/

#include "./multi_host_param.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

const uint8_t g_chPortBuf[TOTAL_PORT_NUM] = {
    CH_INTERNAL_CACHE_PORT, CH_PORT1, CH_PORT2, CH_PORT3, CH_PORT4
};

proxy_setting_t s_tGboalSetting = {
    0,
    0,
    .hwCacheExpirationTime = 1,
    0
};

port_config_t s_tPortConfigBuf[TOTAL_PORT_NUM] = {
    {
        .chPort = CH_INTERNAL_CACHE_PORT,
        .tFormat = MODBUS_RTU,
        .hwTimeout = 1000,
    },
    {
        .chPort = CH_PORT1,
        .tFormat = MODBUS_RTU,
        .hwTimeout = 1000,
        .tConfig = {
            .wBaudRate = CH1_PORT_BAUDRATE,
        }
    },
    {
        .chPort = CH_PORT2,
        .tFormat = MODBUS_RTU,
        .hwTimeout = 1000,
        .tConfig = {
            .wBaudRate = CH2_PORT_BAUDRATE,
        }
    },
    {
        .chPort = CH_PORT3,
        .tFormat = MODBUS_RTU,
        .hwTimeout = 1000,
        .tConfig = {
            .wBaudRate = CH3_PORT_BAUDRATE,
        }
    },
    {
        .chPort = CH_PORT4,
        .tFormat = MODBUS_RTU,
        .hwTimeout = 1000,
        .tConfig = {
            .wBaudRate = CH4_PORT_BAUDRATE,
        }
    }
};       // 端口配置实例数组

port_resource_t s_tPortResource[TOTAL_PORT_NUM] = {
    {
        .chPort = CH_INTERNAL_CACHE_PORT,
        .tRole = MASTER,
        .tBuf = {
            {0, 253, 0, 253}
        },
    },
    {
        .chPort = CH_PORT1,
        .tRole = MASTER,
        .tBuf = {
            {1, 253, 1, 253}
        }
    },
    {
        .chPort = CH_PORT2,
        .tRole = MASTER,
        .tBuf = {
            {1, 252, 1, 252}
        },
    },
    {
        .chPort = CH_PORT3,
        .tRole = MASTER,
        .tBuf = {
            {1, 253, 1, 253}
        },
    },
    {
        .chPort = CH_PORT4,
        .tRole = SLAVE,
        .tBuf = {
            {1, 253, 1, 253}
        },
    },
};

static uint8_t s_chMasterPortNum = 0;
static uint8_t s_chSlavePortNum = 0;

void multi_host_param_init(void)
{
    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (MASTER == s_tPortResource[i].tRole) {
            s_chMasterPortNum += 1;
        } else  if (SLAVE == s_tPortResource[i].tRole) {
            s_chSlavePortNum += 1;
        }
    }
}

const proxy_setting_t *proxy_get_global_setting(void)
{
    return &s_tGboalSetting;
}

const port_resource_t *proxy_get_port_resource(uint8_t chPort)
{
    int8_t i = 0;

    for (i = 0; (i < TOTAL_PORT_NUM) && (s_tPortResource[i].chPort != chPort); i++);
    return i == TOTAL_PORT_NUM ? NULL : &s_tPortResource[i];
}

const port_config_t *proxy_get_port_config(uint8_t chPort)
{
    int8_t i = 0;

    for (i = 0; (i < TOTAL_PORT_NUM) && (s_tPortConfigBuf[i].chPort != chPort); i++);
    return i == TOTAL_PORT_NUM ? NULL : &s_tPortConfigBuf[i];
}

uint8_t proxy_get_master_port_num(void)
{
    return s_chMasterPortNum;
}

uint8_t proxy_get_slave_port_num(void)
{
    return s_chSlavePortNum;
}

const port_resource_t *proxy_get_master_port_resource(uint8_t chIndex)
{
    uint8_t j = 0;

    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (MASTER == s_tPortResource[i].tRole) {
            if (chIndex == (j++)) {
                return &s_tPortResource[i];
            }
        }
    }

    return NULL;
}

const port_resource_t *proxy_get_slave_port_resource(uint8_t chIndex)
{
    uint8_t j = 0;

    for (uint8_t i = 0; i < TOTAL_PORT_NUM; i++) {
        if (SLAVE == s_tPortResource[i].tRole) {
            if (chIndex == (j++)) {
                return &s_tPortResource[i];
            }
        }
    }

    return NULL;
}

/*************************** End of file ****************************/
