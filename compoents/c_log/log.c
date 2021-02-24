/****************************************************************************
 * Copyright (c) [2021] [Soga] [core.zhang@outlook.com]                     *
 * [] is licensed under Mulan PSL v2.                                       *
 * You can use this software according to the terms and conditions of       *
 * the Mulan PSL v2.                                                        *
 * You may obtain a copy of Mulan PSL v2 at:                                *
 *          http://license.coscl.org.cn/MulanPSL2                           *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF     *
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO        *
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.       *
 * See the Mulan PSL v2 for more details.                                   *
 *                                                                          *
 ***************************************************************************/
/* Includes --------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "./log.h"
#include "./log_port.h"

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
#define COLOR_NORMAL        "\x1B[0m"
#define COLOR_RED           "\x1B[31m"
#define COLOR_GREEN         "\x1B[32m"
#define COLOR_YELLOW        "\x1B[33m"
#define COLOR_BLUE          "\x1B[34m"
#define COLOR_MAGENTA       "\x1B[35m"
#define COLOR_CYAN          "\x1B[36m"
#define COLOR_WHITE         "\x1B[37m"
#define COLOR_RESET         "\033[0m"

#define DATETIME_FORMAT     "%d-%02d-%02d %02d:%02d:%02d"
/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
#ifndef LOG_MINIMUM_LEVEL
    #define LOG_MINIMUM_LEVEL L_INFO
#endif
#if (LOG_MINIMUM_LEVEL > L_CRITICAL) || (LOG_MINIMUM_LEVEL < L_DEBUG)
    #error "Log level is error."
#endif
static const log_level_t s_tMinimumLevel = LOG_MINIMUM_LEVEL;

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/
static const char *level_name(log_level_t tLevel)
{
    switch (tLevel) {
    case L_DEBUG:
        return "[DEBUG]";
    case L_INFO:
        return "[INFO]";
    case L_WARN:
        return "[WARN]";
    case L_ERROR:
        return "[ERROR]";
    case L_CRITICAL:
        return "[CRITICAL]";
    default:
        return "[NONE]";
    }
}

static const char *level_color(log_level_t tLevel)
{
    switch (tLevel) {
    default:
    case L_DEBUG:
    case L_INFO:
        return COLOR_WHITE;
    case L_WARN:
        return COLOR_YELLOW;
    case L_ERROR:
        return COLOR_RED;
    case L_CRITICAL:
        return COLOR_MAGENTA;
    }
}

static uint32_t add_datetime(char **ppBuf, uint32_t wSize)
{
    struct tm cur;
    int n;

    log_port_current_time(&cur);
    n = snprintf(*ppBuf, wSize, DATETIME_FORMAT" ",
             cur.tm_year + 1900, cur.tm_mon + 1, cur.tm_mday,
             cur.tm_hour, cur.tm_min, cur.tm_sec);
    if (n > 0) {
        *ppBuf += n;
    } else {
        n = 0;
    }

    return n;
}

static uint32_t add_string(char **ppBuf, uint32_t wSize, const char *name)
{
    int n;

    n = snprintf(*ppBuf, wSize, "%s ", name);
    if (n > 0) {
        *ppBuf += n;
    } else {
        n = 0;
    }

    return n;
}

static uint32_t add_line_number(char **ppBuf, uint32_t wSize, int32_t nLineNo)
{
    int n;

    n = snprintf(*ppBuf, wSize, "%d ", nLineNo);
    if (n > 0) {
        *ppBuf += n;
    } else {
        n = 0;
    }

    return n;
}

int32_t log_output(const log_content_t *ptCtx, const log_data_t *ptData,
                   const char *format, ...)
{
    va_list args;
    uint32_t wRemain;
    int n;
    char *buf;

    if ((NULL == ptCtx) || (NULL == ptData)) {
        return -1;
    }
    if (!ptCtx->bIsInited) {
        return -1;
    }
    if ((ptData->tLevel < ptData->tModuleLevel) ||
        (ptData->tLevel < s_tMinimumLevel)) {
        return -1;
    }

    log_port_lock(ptCtx->pUser);

    buf = ptCtx->tOutDesc.pBuffer;
    wRemain = ptCtx->tOutDesc.wBufferSize;

    /*! add prefix */
    if (ptCtx->tPrefix.bColor) {
        wRemain -= add_string(&buf, wRemain, level_color(ptData->tLevel));
    }

    if (ptCtx->tPrefix.bDateTime) {
        wRemain -= add_datetime(&buf, wRemain);
    }

    if (ptCtx->tPrefix.bFileName) {
        wRemain -= add_string(&buf, wRemain, ptData->pFileName);
    }

    if (ptCtx->tPrefix.bFuncName) {
        wRemain -= add_string(&buf, wRemain, ptData->pFuncName);
    }

    if (ptCtx->tPrefix.bLineNo) {
        wRemain -= add_line_number(&buf, wRemain, ptData->nLineNo);
    }

    if (ptCtx->tPrefix.bLevelName) {
        wRemain -= add_string(&buf, wRemain, level_name(ptData->tLevel));
    }

    va_start(args, format);

    n = vsnprintf(buf, wRemain, format, args);
    if (n > 0 ) {
        wRemain -= n;
    }

    wRemain = ptCtx->tOutDesc.wBufferSize - wRemain;
    log_port_output(ptCtx->pUser, ptCtx->tOutDesc.pBuffer, wRemain);

    va_end(args);

    log_port_unlock(ptCtx->pUser);

    return wRemain;
}

int32_t log_init(log_content_t *ptCtx, const log_output_desc_t *ptDesc,
                 void *pUser)
{
    if ((NULL == ptCtx) || (NULL == ptDesc)) {
        return -1;
    }
    if ((ptDesc->pBuffer == NULL) ||
        (ptDesc->wBufferSize == 0)) {
        return -1;
    }

    log_port_init(ptCtx->pUser);

    memset(ptCtx, 0, sizeof(log_content_t));
    ptCtx->tOutDesc = *ptDesc;
    ptCtx->pUser = pUser;
    ptCtx->bIsInited = 1;

    return 0;
}

int32_t log_set_format(log_content_t *ptCtx, uint32_t wOption)
{
    if (NULL == ptCtx) {
        return -1;
    }

    log_port_lock(ptCtx->pUser);

    ptCtx->tPrefix.bFileName = (wOption & LOG_OPT_FILE_NAME) != 0;
    ptCtx->tPrefix.bFuncName = (wOption & LOG_OPT_FUNCTION_NAME) != 0;
    ptCtx->tPrefix.bLineNo = (wOption & LOG_OPT_LINE_NO) != 0;
    ptCtx->tPrefix.bLevelName = (wOption & LOG_OPT_LEVEL_NAME) != 0;
    ptCtx->tPrefix.bDateTime = (wOption & LOG_OPT_DATETIME) != 0;
    ptCtx->tPrefix.bColor = (wOption & LOG_OPT_COLOR) != 0;

    log_port_unlock(ptCtx->pUser);

    return 0;
}

/*************************** End of file ****************************/
