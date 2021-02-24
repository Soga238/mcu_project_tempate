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
#include <stdint.h>
#include <time.h>
#include <stdio.h>

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/
void log_port_init(void *pUser)
{
    /*! add user code */
}

void log_port_lock(void* pUser)
{
    /*! add user code */
}

void log_port_unlock(void* pUser)
{
    /*! add user code */
}

void log_port_output(void *pUser, void *pBuffer, uint32_t wSize)
{
    for (uint32_t i = 0; i < wSize; ++i) {
        printf("%c", *(char *)pBuffer);
        (char *)pBuffer++;
    }
}

void log_port_current_time(struct tm *tm)
{
    time_t cur;
    time(&cur);
    localtime_s(tm, &cur);
}

/*************************** End of file ****************************/
