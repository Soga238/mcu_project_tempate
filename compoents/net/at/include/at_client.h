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
*       AT CLIENT * AT FRAMEWORK                                     *
*                                                                    *
**********************************************************************
*/
#ifndef  __AT_CLIENT_H__
#define  __AT_CLIENT_H__

#include "..\at_cfg.h"
#include ".\at_types.h"
#include "cmsis_os2.h"

/* RT-Thread error code definitions */
#define RT_EOK                           0               /**< There is no error */
#define RT_ERROR                        -1               /**< A generic error happens */
#define RT_ETIMEOUT                     -2               /**< Timed out */
#define RT_EFULL                        -3               /**< The resource is full */
#define RT_EEMPTY                       -4               /**< The resource is empty */
#define RT_ENOMEM                       -5               /**< No memory */
#define RT_ENOSYS                       -6               /**< No system */
#define RT_EBUSY                        -7               /**< Busy */
#define RT_EIO                          -8               /**< IO error */
#define RT_EINTR                        -9               /**< Interrupted system call */
#define RT_EINVAL                       -10              /**< Invalid argument */

typedef  struct at_response *at_response_t;
struct at_response {
    char *buf;                                          /*response buffer*/
    rt_size_t buf_size;                                 /*the maximum response buffer size*/
    rt_size_t line_number;                              /* the number of setting response lines */
    rt_size_t line_counts;                              /* the count of received response lines*/
    rt_size_t timeout;                                  /* the maxium response time*/
};

typedef  struct at_client *at_client_t;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY'*/
typedef  struct at_urc *at_urc_t;
struct at_urc {
    const char* cmd_prefix;
    const char* cmd_suffix;
    void (*func)(struct at_client * client, const char *data, rt_size_t size);
};

//extern struct at_urc URC_TABLE[URC_TABLE_SIZE];

typedef  enum at_status {
    AT_STATUS_UNINITIALIZED = 0,
    AT_STATUS_INITIALIZED,
    AT_STATUS_BUSY,
} at_status_t;

typedef  enum at_resp_status {
    AT_RESP_OK =             0,
    AT_RESP_ERROR =         -1,
    AT_RESP_TIMEOUT =       -2,
    AT_RESP_BUFF_FULL =     -3,
    AT_RESP_MORE_LINE =     -4,
} at_resp_status_t;


struct at_client {
    at_status_t     status;
    char            end_sign;
    char           *recv_buffer;
    rt_size_t       recv_bufsz;
    rt_size_t       cur_recv_len;

    osSemaphoreId_t rx_notice;
    osMutexId_t     lock;

    at_response_t   resp;
    osSemaphoreId_t resp_notice;
    at_resp_status_t resp_status;

    const struct at_urc* urc_table;
    rt_size_t       urc_table_size;

    osThreadId_t    parser;
};

extern int32_t at_client_init(rt_int32_t port, rt_size_t recv_buffsz);

extern void client_parser(at_client_t client);

extern at_client_t at_client_get_first(void);

extern at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);

extern at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);

extern void at_delete_resp(at_response_t resp);

extern int32_t at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *expr, ...);

extern int32_t at_obj_exec_cmd_with_buf(at_client_t client, at_response_t resp, uint8_t *pchBuf, rt_int32_t send_len);

extern const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);

extern int32_t at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);

extern int32_t at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...);

extern const char *at_resp_get_line(at_response_t resp, rt_size_t resp_line);

extern void at_obj_set_urc_table(at_client_t client, const struct at_urc *urc_table, rt_size_t table_sz);

#endif
