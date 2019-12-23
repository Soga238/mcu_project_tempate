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

#define USR_PORT_NUM            4                   // �û�ʹ�õĶ˿���

#define TOTAL_PORT_NUM          (USR_PORT_NUM + 1)  // ȫ���Ķ˿���
#define CH_INTERNAL_CACHE_PORT  0xA0                // �ڲ�����˿�
#define RAW_DATA_BUF_SIZE       253

/**
 *  \brief �˿ڶ�ȡ���泬ʱʱ��
 */
#define MINMIUM_READ_CACHE_WAIT_TIME        75

/**
 *  \brief ��Ϊ���ö�ȡ���洦��ʱ�䣨��λ���룩������ظ����졣
 */
#define MINMIUM_READ_CACHE_PROCESS_TIME     25

/**
 *  \brief ��־��ӡ���أ�0��ʾ�رմ�ӡ����
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
