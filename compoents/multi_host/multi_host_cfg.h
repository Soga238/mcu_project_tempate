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
*       multi_host_cfg.h *                                           *
*                                                                    *
**********************************************************************
*/

#ifndef MULTI_HOST_CFG_H
#define MULTI_HOST_CFG_H

#include "../../usr_app/usr_app_cfg.h"

#define USR_PORT_NUM            4                   // 用户使用的端口数

#define TOTAL_PORT_NUM          (USR_PORT_NUM + 1)  // 全部的端口数
#define CH_INTERNAL_CACHE_PORT  0xA0                // 内部缓存端口
#define RAW_DATA_BUF_SIZE       253

/**
 *  \brief 端口读取缓存超时时间
 */
#define MINMIUM_READ_CACHE_WAIT_TIME        75

/**
 *  \brief 人为设置读取缓存处理时间（单位毫秒），避免回复过快。
 */
#define MINMIUM_READ_CACHE_PROCESS_TIME     25

/**
 *  \brief 日志打印开关，0表示关闭打印功能
 */

#define MULTI_HOST_LOG_ENABLE   0

#if MULTI_HOST_LOG_ENABLE

#include "cmsis_os2.h"
#define MLOG_RAW(format, ...)   \
        do {                    \
            SEGGER_RTT_printf(0u, RTT_CTRL_RESET"%d ", osKernelGetTickCount()); \
            SEGGER_RTT_printf(0u, RTT_CTRL_RESET""format, ##__VA_ARGS__);       \
        }while(0)

#define MLOG_LN(format, ...)    \
        SEGGER_RTT_printf(0u, RTT_CTRL_RESET""format, ##__VA_ARGS__);
#else
#define MLOG_RAW(format, ...)
#define MLOG_LN(FORMAT, ...)
#endif

#endif

/*************************** End of file ****************************/
