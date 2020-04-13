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
#include "..\include\at_client.h"
#include "..\include\at_utils.h"
#include "..\include\at_port.h"

#include <string.h>
#include <stdio.h>

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
#define AT_RESP_END_OK                  "OK"
#define AT_RESP_END_ERROR               "ERROR"
#define AT_RESP_END_FAIL                "FAIL"
#define AT_END_CR_LF                    "\r\n"
/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
static osMemoryPoolId_t s_tMemPool;
static osMemoryPoolId_t s_tMemPoolResp;
static osMemoryPoolId_t s_tMemPoolRespBuf;

static osThreadAttr_t thread4_attr = {
    .stack_size = 1024,
    .priority = osPriorityAboveNormal2,
};

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

/**
 * \brief   Create response object.
 * \param[in]
 * \return  resp response object
 */
at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout)
{
    at_response_t resp = NULL;

    if (RESP_BUF_SIZE_MAX < buf_size) {
        ALOG_W("No memory for response object!\r\n");
        return NULL;
    }

    resp = (at_response_t)osMemoryPoolAlloc(s_tMemPoolResp, 0);
    if (NULL == resp) {
        ALOG_W("No memory for response object!\r\n");
        return  NULL;
    }

    resp->buf = osMemoryPoolAlloc(s_tMemPoolRespBuf, 0);
    if (NULL == resp->buf) {
        ALOG_W("No memory for response buffer!\r\n");
        osMemoryPoolFree(s_tMemPoolResp, resp);
        return  NULL;
    }

    resp->buf_size = buf_size;
    resp->line_number = line_num;
    resp->line_counts = 0;
    resp->timeout = timeout;

    return resp;
}

/**
 * \brief Delete and free response object.
 * \param[in] resp response object
 * \return
 */
void at_delete_resp(at_response_t resp)
{
    ASSERT(NULL != resp);

    if (resp->buf) {
        osMemoryPoolFree(s_tMemPoolRespBuf, resp->buf);
    }

    osMemoryPoolFree(s_tMemPoolResp, resp);
}

/**
 * \brief Set response object information
 * \param[in] resp response object
 * \return
 */
at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout)
{
    ASSERT(NULL != resp);

    /*! \note use RTX static memory */
    if (RESP_BUF_SIZE_MAX < buf_size) {
        ALOG_W("No memory for response object!\r\n");
        return NULL;
    }

    resp->line_number = line_num;
    resp->timeout = timeout;

    return resp;
}

/**
 * \brief Get one line AT response buffer by line number
 *      \note resp_line start from 1
 *
 * \param[in] resp response object
 * \return
 */
const char *at_resp_get_line(at_response_t resp, rt_size_t resp_line)
{
    rt_size_t line_num = 0;
    char *resp_buf = NULL;

    ASSERT(NULL != resp);

    if (resp_line > resp->line_counts || resp_line <= 0) {
        ALOG_W("at get %d line data failed\r\n", resp_line);
        return  NULL;
    }

    resp_buf = resp->buf;
    for (line_num = 1; line_num <= resp->line_counts; ++line_num) {
        if (resp_line == line_num) {
            return resp_buf;
        }
        // Each one line data ends with zero.
        // You need add 1 manually, when calculating the length of a line data.
        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * \brief Get one line AT response buffer by keyword
 * \param[in] resp response object
 * \return
 */
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword)
{
    rt_size_t line_num = 0;
    char *resp_buf = NULL;

    ASSERT(NULL != resp);

    resp_buf = resp->buf;
    for (line_num = 1; line_num <= resp->line_counts; ++line_num) {
        if (strstr(resp_buf, keyword)) {
            return resp_buf;
        }
        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * \brief Get and parse AT response buffer arguments by line number
 * \param[in] resp response object
 * \return
 */
int32_t at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...)
{
    va_list args;
    const char *resp_line_buf = NULL;
    int32_t resp_args_num = -1;

    ASSERT(NULL != resp);
    ASSERT(NULL != resp_expr);

    resp_line_buf = at_resp_get_line(resp, resp_line);
    if (NULL == resp_line_buf) {
        return -1;
    }

    va_start(args, resp_expr);
    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);
    va_end(args);

    return resp_args_num;
}

/**
 * \brief Get and parse AT response buffer arguments by keyword
 * \param[in] resp response object
 * \return
 */
int32_t at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...)
{
    va_list args;
    int32_t resp_args_num = 0;
    const char *resp_line_buf = NULL;

    ASSERT(NULL != resp);
    ASSERT(NULL != resp_expr);

    resp_line_buf = at_resp_get_line_by_kw(resp, keyword);
    if (NULL == resp_line_buf) {
        return -1;
    }

    va_start(args, resp_expr);
    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);
    va_end(args);

    return resp_args_num;
}

/**
 * \brief Send commands to AT server and wait response
 * \param[in] at client object
 * \param[in] resp response object
 * \return
 */
int32_t at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *expr, ...)
{
    va_list args;
    int32_t result = RT_EOK;
    osStatus_t status;
    int32_t nLength;

    ASSERT(NULL != client);

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
    osMutexAcquire(client->lock, osWaitForever);
#endif

    // thread(client_parser) will start checking the response
    client->resp_status = AT_RESP_OK;
    client->resp = resp;
    if (NULL != resp) {
        resp->line_counts = 0;
    }

    va_start(args, expr);
    at_vprintf(client, expr, args);
    va_end(args);

    if (resp != NULL) {
        status = osSemaphoreAcquire(client->resp_notice, resp->timeout);
        if (osOK == status) {
            if (client->resp_status != AT_RESP_OK) {
                ALOG_W("exec(%s), state=%d!\r\n", at_get_last_cmd(&nLength), client->resp_status);
                result = RT_ERROR;
            }
        } else if (osErrorTimeout == status) {
            ALOG_W("exec(%s) timeout (%d ticks)!\r\n", at_get_last_cmd(&nLength), resp->timeout);
            result = RT_ETIMEOUT;
        } else {
            ALOG_W("os error\r\n");
            result = RT_ERROR;
        }
    }

    client->resp = NULL;

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
    osMutexRelease(client->lock);   // realse multi-client lock
#endif

    return result;
}

/**
 * \brief Send commands to AT server and wait response
 * \param[in] at client object
 * \param[in] resp response object
 * \return
 */
int32_t at_obj_exec_cmd_with_buf(at_client_t client, at_response_t resp, uint8_t *pchBuf, rt_int32_t send_len)
{
    int32_t result = RT_EOK;
    osStatus_t status;
    int32_t nLength;

    ASSERT(NULL != client);
    if (0 == send_len) {
        return  RT_ERROR;
    }

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
    osMutexAcquire(client->lock, osWaitForever);    // acquire multi-client lock
#endif

    // thread(client_parser) will start checking the response
    client->resp_status = AT_RESP_OK;
    client->resp = resp;
    if (resp) {
        resp->line_counts = 0;
    }

    // send AT cmd to AT server
    at_client_obj_send(client, pchBuf, send_len);
    if (resp != NULL) {
        status = osSemaphoreAcquire(client->resp_notice, resp->timeout);
        if (osOK == status) {
            // receive response from AT server
            if (client->resp_status != AT_RESP_OK) {
                ALOG_W("exec(%s), state=%d!\r\n", at_extract_last_cmd(pchBuf, &nLength), client->resp_status);
                result = RT_ERROR;
            }
        } else if (osErrorTimeout == status) {
            ALOG_W("exec(%s) timeout (%d ticks)!\r\n", at_extract_last_cmd(pchBuf, &nLength), resp->timeout);
            result = RT_ETIMEOUT;
        } else {
            ALOG_W("os error\r\n");
            result = RT_ERROR;
        }
    }

    client->resp = NULL;

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
    osMutexRelease(client->lock);   // realse multi-client lock
#endif

    return result;
}

void at_obj_set_urc_table(at_client_t client, const struct at_urc *urc_table, rt_size_t table_sz)
{
    rt_size_t idx = 0;

    ASSERT(NULL != client);
    if (NULL == client) {
        return ;
    }

    for (idx = 0; idx < table_sz; idx++) {
        ASSERT(urc_table[idx].cmd_prefix != NULL);
        //ASSERT(urc_table[idx].cmd_suffix != NULL);  // ?¨¦¨°?¨®D?¡ã¡Áo¦Ì???o¨®¡Áo
    }

    client->urc_table = urc_table;
    client->urc_table_size = table_sz;
}

static const struct at_urc *get_urc_obj(at_client_t client)
{
    rt_size_t i;
    rt_size_t prefix_len;
    rt_size_t suffix_len;
    rt_size_t buf_sz;
    char *buffer = NULL;
    const char *prefix = NULL;
    const char *suffix = NULL;

    ASSERT(NULL != client);
    if (NULL == client || NULL == client->urc_table) {
        return NULL;
    }

    buffer = client->recv_buffer;
    buf_sz = client->cur_recv_len;
    for (i = 0; i < client->urc_table_size; i++) {
        prefix = client->urc_table[i].cmd_prefix;
        suffix = client->urc_table[i].cmd_suffix;

        prefix_len = (NULL != prefix) ? strlen(prefix) : 0;
        suffix_len = (NULL != suffix) ? strlen(suffix) : 0;
        if (buf_sz < prefix_len + suffix_len) {
            continue;
        }

        // match prefix and suffix
        if ((prefix_len ? !strncmp(buffer, prefix, prefix_len) : 1) &&
            (suffix_len ? !strncmp(buffer + buf_sz - suffix_len, suffix, suffix_len) : 1)) {
            return &client->urc_table[i];
        }
    }
    return  NULL;
}

static int at_recv_readline(at_client_t client)
{
    char ch = 0;
    char last_ch = 0;
    rt_size_t read_len = 0;

    // clean buffer
    memset(client->recv_buffer, 0, client->recv_bufsz);
    client->cur_recv_len = 0;

    do {
        // block until receive a character
        if (0 == at_client_get_char(client, (uint8_t *)&ch)) {
            continue;
        }

        if (read_len < client->recv_bufsz) {
            client->recv_buffer[read_len++] = ch;
            client->cur_recv_len = read_len;
        } else {
            // memset(client->recv_buffer, 0, client->recv_bufsz);
            client->cur_recv_len = 0;
            ALOG_W("Read line failed\r\n");
            return RT_EFULL;
        }

        // is newline
        if ((ch == '\n' && last_ch == '\r') ||
            (client->end_sign != 0 && ch == client->end_sign)) {
            break;
        }

        // is URC data
        if (get_urc_obj(client)) {
            break;
        }

        last_ch = ch;
    } while (1);

    return  read_len;
}

void client_parser(at_client_t client)
{
    const struct at_urc *urc = NULL;
    int32_t resp_buf_len = 0;
    rt_size_t line_counts = 0;

    ASSERT(NULL != client);
    ALOG_D("start client parser thread\r\n");

    while (1) {
        if (0 >= at_recv_readline(client)) {
            continue;
        }

        urc = get_urc_obj(client);
        if (NULL != urc) {
            // handle URC data
            if (NULL != urc->func) {
                urc->func(client, client->recv_buffer, client->cur_recv_len);
            }
        } else if (NULL != client->resp) {
            // handle normal AT response
            client->recv_buffer[client->cur_recv_len - 1] = '\0';
            if (resp_buf_len + client->cur_recv_len < client->resp->buf_size) {
                memcpy(client->resp->buf + resp_buf_len, client->recv_buffer, client->cur_recv_len);
                resp_buf_len += client->cur_recv_len;
                line_counts += 1;
            } else {
                client->resp_status = AT_RESP_BUFF_FULL;
                ALOG_W("The Response buffer size is out of buffer size\r\n");
            }

            // check response result
            if (0 == client->resp->line_number &&
                0 == memcmp(client->recv_buffer, AT_RESP_END_OK, strlen(AT_RESP_END_OK))) {
                client->resp_status = AT_RESP_OK;
            } else if (strstr(client->recv_buffer, AT_RESP_END_ERROR) ||
                       0 == memcmp(client->recv_buffer, AT_RESP_END_FAIL, strlen(AT_RESP_END_FAIL))) {
                client->resp_status = AT_RESP_ERROR;

            } else if (client->resp->line_number &&
                       line_counts == client->resp->line_number) {
                client->resp_status = AT_RESP_OK;
            } else if (line_counts > client->resp->line_number) {
                // get more line counts.
                client->resp_status = AT_RESP_MORE_LINE;
            } else {
                continue;
            }

            client->resp->line_counts = line_counts;

            // Do not use resp by the client at the front desk.
            client->resp = NULL;

            // notice response.
            osSemaphoreRelease(client->resp_notice);
            resp_buf_len = 0;
            line_counts = 0;
        }
    }
}

int32_t at_client_para_init(at_client_t client)
{
    int32_t result = RT_EOK;

    if (NULL == client) {
        return RT_ERROR;
    }

    client->status = AT_STATUS_UNINITIALIZED;
    client->cur_recv_len = 0;

    client->recv_buffer = osMemoryPoolAlloc(s_tMemPool, 0);
    if (NULL == client->recv_buffer) {
        result = RT_ENOMEM;
        goto __exit;
    }

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
    client->lock = osMutexNew(NULL);
    if (NULL == client->lock) {
        result = RT_ENOMEM;
        goto __exit;
    }
#endif

    client->rx_notice = osSemaphoreNew(1, 0, NULL);
    if (NULL == client->rx_notice) {
        result = RT_ENOMEM;
        goto __exit;
    }

    client->resp_notice = osSemaphoreNew(1, 0, NULL);
    if (NULL == client->resp_notice) {
        result = RT_ENOMEM;
        goto __exit;
    }

    client->parser = osThreadNew((osThreadFunc_t)client_parser, client, &thread4_attr);
    if (NULL == client->parser) {
        result = RT_ENOMEM;
        goto __exit;
    }

__exit:
    //  release resource to system
    if (result != RT_EOK) {
        if (client->recv_buffer) {
            osMemoryPoolFree(s_tMemPool, client->recv_buffer);
        }

#if defined(AT_CLIENT_MUTEX_LOCK_ENABLE)
        if (client->lock) {
            osMutexDelete(client->lock);
        }
#endif

        if (client->rx_notice) {
            osSemaphoreDelete(client->rx_notice);
        }

        if (client->resp_notice) {
            osSemaphoreDelete(client->resp_notice);
        }

        if (client->parser) {
            osThreadDetach(client->parser);
        }
    }

    return result;
}

/**
 * @param recv_buffsz the maximum of receive buffer length
 * @return 0 success
 */
int32_t at_client_init(rt_int32_t port, rt_size_t recv_buffsz)
{
    int32_t idx;
    int32_t result = RT_EOK;
    at_client_t client;

    ASSERT(0 < recv_buffsz);

    s_tMemPool = osMemoryPoolNew(MEMPOOL_OBJECTS, recv_buffsz, NULL);
    if (NULL == s_tMemPool) {
        result = RT_ENOMEM;
        goto __exit;
    }

    s_tMemPoolResp = osMemoryPoolNew(RESP_MEMPOOL_OBJECTS, sizeof(struct at_response), NULL);
    if (NULL == s_tMemPoolResp) {
        result = RT_ENOMEM;
        goto __exit;
    }

    s_tMemPoolRespBuf = osMemoryPoolNew(RESP_MEMPOOL_OBJECTS, RESP_BUF_SIZE_MAX, NULL);
    if (NULL == s_tMemPoolRespBuf) {
        result = RT_ENOMEM;
        goto __exit;
    }

    client = at_client_get(port);
    if (NULL != client) {
        client->recv_bufsz = recv_buffsz;
    }  else {
        result = RT_ERROR;
        goto __exit;
    }

    result = at_client_para_init(client);
    if (RT_EOK != result) {
        goto __exit;
    }

__exit:
    if (RT_ENOMEM == result) {
        if (s_tMemPool) {
            osMemoryPoolDelete(s_tMemPool);
        }
        if (s_tMemPoolResp) {
            osMemoryPoolDelete(s_tMemPoolResp);
        }
        if (s_tMemPoolRespBuf) {
            osMemoryPoolDelete(s_tMemPoolRespBuf);
        }
    }

    return result;
}

/*************************** End of file ****************************/
