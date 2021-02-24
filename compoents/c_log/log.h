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
#ifndef C_LOGGER_H
#define C_LOGGER_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include <stdint.h>
#include "./log_cfg.h"

/* Global define ---------------------------------------------------*/
#define LOG_OPT_FILE_NAME       (1 << 0)
#define LOG_OPT_FUNCTION_NAME   (1 << 1)
#define LOG_OPT_LINE_NO         (1 << 2)
#define LOG_OPT_LEVEL_NAME      (1 << 3)
#define LOG_OPT_DATETIME        (1 << 4)
#define LOG_OPT_COLOR           (1 << 5)

/* Global macro ----------------------------------------------------*/
#if defined(LOG_MODULE_LEVEL_DEBUG)
    #define LOG_MODULE_MINIMUM_LEVEL    L_DEBUG
#elif defined(LOG_MODULE_LEVEL_INFO)
    #define LOG_MODULE_MINIMUM_LEVEL    L_INFO
#elif defined(LOG_MODULE_LEVEL_WARN)
    #define LOG_MODULE_MINIMUM_LEVEL    L_WARN
#elif defined(LOG_MODULE_LEVEL_ERROR)
    #define LOG_MODULE_MINIMUM_LEVEL    L_ERROR
#elif defined(LOG_MODULE_LEVEL_CRITICAL)
    #define LOG_MODULE_MINIMUM_LEVEL    L_CRITICAL
#else
    #define LOG_MODULE_MINIMUM_LEVEL    L_DEBUG
#endif

#ifndef CONNECT2
#define __CONNECT2(__A, __B)    __A##__B
#define CONNECT2(__A, __B)      __CONNECT2(__A, __B)
#endif

#define __LOG_OUTPUT(__LOGGER, __LEVEL, ...)                                    \
    do {                                                                        \
        log_content_t *CONNECT2(ptCtx, __LINE__) = (log_content_t *)(__LOGGER); \
        log_data_t CONNECT2(tData, __LINE__) = {                                \
            .tLevel = __LEVEL,                                                  \
            .tModuleLevel = LOG_MODULE_MINIMUM_LEVEL,                           \
            .nLineNo = __LINE__,                                                \
            .pFuncName = __FUNCTION__ ,                                         \
            .pFileName = __FILE__                                               \
        };                                                                      \
        log_output(CONNECT2(ptCtx, __LINE__),                                   \
                   &(CONNECT2(tData,__LINE__)),                                 \
                   __VA_ARGS__);                                                \
    } while(0)

#define __LOG_DEBUG(__LOGGER, ...)  __LOG_OUTPUT(__LOGGER, L_DEBUG, __VA_ARGS__)
#define LOG_DEBUG(__LOGGER, ...)    __LOG_DEBUG(__LOGGER, __VA_ARGS__)

#define __LOG_INFO(__LOGGER, ...)   __LOG_OUTPUT(__LOGGER, L_INFO, __VA_ARGS__)
#define LOG_INFO(__LOGGER, ...)     __LOG_INFO(__LOGGER, __VA_ARGS__)

#define __LOG_WARN(__LOGGER, ...)   __LOG_OUTPUT(__LOGGER, L_WARN, __VA_ARGS__)
#define LOG_WARN(__LOGGER, ...)     __LOG_WARN(__LOGGER, __VA_ARGS__)

#define __LOG_ERROR(__LOGGER, ...)  __LOG_OUTPUT(__LOGGER, L_ERROR, __VA_ARGS__)
#define LOG_ERROR(__LOGGER, ...)    __LOG_ERROR(__LOGGER, __VA_ARGS__)

#define __LOG_CRITICAL(__LOGGER, ...)  __LOG_OUTPUT(__LOGGER, L_CRITICAL, __VA_ARGS__)
#define LOG_CRITICAL(__LOGGER, ...)    __LOG_CRITICAL(__LOGGER, __VA_ARGS__)

#define LOG_INIT(__LOGGER, __OUTPUT_DESC_PTR, __USER_PTR) \
    do {log_init(__LOGGER, __OUTPUT_DESC_PTR, __USER_PTR);} while(0)

#define LOG_SET_FORMAT(__LOGGER, OPTION) \
    do {log_set_format(__LOGGER, OPTION);} while(0)

/* Global typedef --------------------------------------------------*/
typedef enum {
    L_DEBUG = 1,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_CRITICAL,
} log_level_t;

typedef struct {
    log_level_t tLevel;
    log_level_t tModuleLevel;
    const char *pFuncName;
    const char *pFileName;
    int32_t     nLineNo;
} log_data_t;

typedef struct {
    uint32_t bFileName   : 1;
    uint32_t bFuncName   : 1;
    uint32_t bLineNo     : 1;
    uint32_t bLevelName  : 1;
    uint32_t bDateTime   : 1;
    uint32_t bColor      : 1;
} log_prefix_t;

typedef struct {
    char            *pBuffer;
    uint32_t        wBufferSize;
} log_output_desc_t;

typedef struct {
    uint8_t         bIsInited   : 1;
    log_prefix_t    tPrefix;
    log_output_desc_t tOutDesc;
    void            *pUser;
} log_content_t;

/* Global variables ------------------------------------------------*/
/* Global function prototypes --------------------------------------*/
extern int32_t
log_init(log_content_t *ptCtx, const log_output_desc_t *ptDesc, void *pUser);

extern int32_t
log_set_format(log_content_t *ptCtx, uint32_t wOption);

extern int32_t
log_output(const log_content_t *ptCtx, const log_data_t *ptData,
           const char *format, ...);

#ifdef __cplusplus
}
#endif
#endif

/*************************** End of file ****************************/
