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
#ifndef LOG_PORT_H
#define LOG_PORT_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------*/
#include <time.h>

/* Global define ---------------------------------------------------*/
/* Global macro ----------------------------------------------------*/
/* Global typedef --------------------------------------------------*/
/* Global variables ------------------------------------------------*/
/* Global function prototypes --------------------------------------*/
extern
void log_port_init(void *pUser);

extern
void log_port_lock(void *pUser);

extern
void log_port_unlock(void *pUser);

extern
void log_port_current_time(struct tm *tm);

extern
void log_port_output(void *pUser, void *pBuffer, uint32_t wSize);

#ifdef __cplusplus
extern "C" {
#endif
#endif
/*************************** End of file ****************************/
