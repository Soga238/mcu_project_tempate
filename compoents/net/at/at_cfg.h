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
*       at_cfg.h *                                                   *
*                                                                    *
**********************************************************************
*/
#ifndef __AT_CFG_H__
#define __AT_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\compoents_cfg.h"

//#define __log(format, ...)                                              \
//        SEGGER_RTT_printf(0u, RTT_CTRL_RESET"%ld "format"",  get_systick_count(), ##__VA_ARGS__);

//#define __log(format, ...) SEGGER_RTT_printf(0u, RTT_CTRL_RESET""format"", ##__VA_ARGS__);

#define __log(format, ...)  ((void *)0)

#define ALOG_D                  __log
#define ALOG_I                  __log
#define ALOG_W                  __log

/*! the maximum number of supported At clients */
#define AT_CLIENT_NUM_MAX               1
#if (AT_CLIENT_NUM_MAX > 1)
#define AT_CLIENT_MUTEX_LOCK_ENABLE
#endif // (AT_CLIENT_NUM_MAX > 1)
    
/*! number of memory pool objects */
#define MEMPOOL_OBJECTS                 (AT_CLIENT_NUM_MAX)
#define RESP_MEMPOOL_OBJECTS            2u
#define RESP_BUF_SIZE_MAX               256u

#define AT_FULL_ASSERT                  0

#define CCID_BUF_SIZE                   20
#define VERSION_BUF_SIZE                20

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
