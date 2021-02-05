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
#include <time.h>
#include <string.h>
#include "k_list.h"
#include "./cron_job_cfg.h"
#include "./cron_job.h"

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
typedef struct CronLine {
    char cl_Dow[7];     /* 0-6, beginning sunday */
    char cl_Mons[12];   /* 0-11 */
    char cl_Hrs[24];    /* 0-23 */
    char cl_Days[32];   /* 1-31 */
    char cl_Mins[60];   /* 0-59 */
#if defined(CRON_SUPPORT_SECOND)
    char cl_Secs[60];   /* 0-59 */
#endif
} CronLine;

typedef struct {
    struct k_list_head  tLink;
    int32_t             nId;
    struct {
        int32_t bIsStop: 1;
    } tBits;

    cron_job_cb_fn      *pfnJob;
    void                *pUser;

    time_t              tNext;
    CronLine            tLine;  // XXX: use bit to save memory space
} cron_job_t;

#ifndef CUSTOM_JOB_MALLOC
    #include <stdlib.h>
    #define JOB_MALLOC(__SIZE)  malloc(__SIZE)
    #define JOB_FREE(__PTR)     free(__PTR)
#endif

#ifndef CUSTOM_JOB_PRINTF
    #include <stdio.h>
    #undef PRINTF
    #define PRINTF  printf
#endif

/* Private define --------------------------------------------------*/
/* Private macro ---------------------------------------------------*/
#undef  MEM_SERO
#define MEM_ZERO(__PTR, __SIZE) memset((__PTR), 0, (__SIZE))
#define SEC_LIMITS              31536000    /*! 365 * 24 * 60 * 60 */

/* Private variables -----------------------------------------------*/
static K_LIST_HEAD(s_tJobHead);

static const char *const DowAry[] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat",
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    NULL
};

static const char *const MonAry[] = {
    "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec",
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    NULL
};

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/
#if defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__)
#define LOCALTIME_S(__TM_P, __TIME_P)   localtime_s(__TM_P, __TIME_P)
#elif defined(__linux__)
#define LOCALTIME_S(__TM_P, __TIME_P)   localtime_r(__TIME_P, __TM_P)
#else
static int _localtime_s(struct tm *tm, time_t *time)
{
    struct tm *_tm;
    // thread lock
    _tm = localtime(time);
    *tm = *_tm;
    // thread unlock
    return 0;
}
#define LOCALTIME_S(__TM_P, __TIME_P)   _localtime_s(__TM_P, __TIME_P)
#endif

/*! reference from busybox/crond.c */
static char *ParseField(char *user, char *ary, int modvalue, int off,
                        const char *const *names, char *ptr)
{
    char *base = ptr;
    int n1 = -1;
    int n2 = -1;

    if (base == NULL) {
        return (NULL);
    }

    while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') {
        int skip = 0;

        /* Handle numeric digit or symbol or '*' */

        if (*ptr == '*') {
            n1 = 0;     /* everything will be filled */
            n2 = modvalue - 1;
            skip = 1;
            ++ptr;
        } else if (*ptr >= '0' && *ptr <= '9') {
            if (n1 < 0) {
                n1 = strtol(ptr, &ptr, 10) + off;
            } else {
                n2 = strtol(ptr, &ptr, 10) + off;
            }
            skip = 1;
        } else if (names) {
            int i;

            for (i = 0; names[i]; ++i) {
                if (strncmp(ptr, names[i], strlen(names[i])) == 0) {
                    break;
                }
            }
            if (names[i]) {
                ptr += strlen(names[i]);
                if (n1 < 0) {
                    n1 = i;
                } else {
                    n2 = i;
                }
                skip = 1;
            }
        }

        /* handle optional range '-' */

        if (skip == 0) {
            PRINTF("  failed user %s parsing %s\n", user, base);
            return (NULL);
        }
        if (*ptr == '-' && n2 < 0) {
            ++ptr;
            continue;
        }

        /*
         * collapse single-value ranges, handle skipmark, and fill
         * in the character array appropriately.
         */

        if (n2 < 0) {
            n2 = n1;
        }
        if (*ptr == '/') {
            skip = strtol(ptr + 1, &ptr, 10);
        }
        /*
         * fill array, using a failsafe is the easiest way to prevent
         * an endless loop
         */

        {
            int s0 = 1;
            int failsafe = 1024;

            --n1;
            do {
                n1 = (n1 + 1) % modvalue;

                if (--s0 == 0) {
                    ary[n1 % modvalue] = 1;
                    s0 = skip;
                }
            } while (n1 != n2 && --failsafe);

            if (failsafe == 0) {
                PRINTF(" failed user %s parsing %s\n", user, base);
                return (NULL);
            }
        }
        if (*ptr != ',') {
            break;
        }
        ++ptr;
        n1 = -1;
        n2 = -1;
    }

    if (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') {
        PRINTF(" failed user %s parsing %s\n", user, base);
        return (NULL);
    }

    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
        ++ptr;
    }

    // {
    //     int i;
    //     PRINTF("%s:", user);
    //     for (i = 0; i < modvalue; ++i) {
    //         PRINTF(" %d ", ary[i]);
    //     }
    //     PRINTF("\n");
    // }

    return (ptr);
}

static int32_t job_id(void)
{
    static int32_t s_nLastId = 1;
    return ++s_nLastId == __INT32_MAX__ ? s_nLastId = 1 : s_nLastId;
}

static time_t next_time(CronLine *ptline, time_t start, time_t end)
{
    time_t t;
    struct tm tm;
    char add = 0;

    for (t = start; t < end;) {
        LOCALTIME_S(&tm, &t);

        /*! month doesn't match: move to 1st of next month */
        if (0 == ptline->cl_Mons[tm.tm_mon]) {
            if (0 == add++) {
                tm.tm_mday = 1;
                tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
            }
            if (++tm.tm_mon, tm.tm_mon >= 12) {
                tm.tm_year++;
                tm.tm_mon = 0;
            }
            t = mktime(&tm);
            continue;
        }

        /*! day & week */
        if (0 == ptline->cl_Days[tm.tm_mday] ||
            0 == ptline->cl_Dow[tm.tm_wday]) {
            if (0 == add++) {
                tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
                t = mktime(&tm);
            }
            t += 86400; /*! 24 * 60 * 60 */
            continue;
        }

        /*! hour */
        if (0 == ptline->cl_Hrs[tm.tm_hour]) {
            if (0 == add++) {
                tm.tm_min = tm.tm_sec = 0;
                t = mktime(&tm);
            }
            t += 3600; /*! 60 * 60 */
            continue;
        }

        /*! minute */
        if (0 == ptline->cl_Mins[tm.tm_min]) {
            if (0 == add++) {
                tm.tm_sec = 0;
                t = mktime(&tm);
            }
            t += 60; /*! 60 */
            continue;
        }
#if defined(CRON_SUPPORT_SECOND)
        /*! second */
        if (0 == ptline->cl_Secs[tm.tm_sec]) {
            t += 1; /*! 1 */
            continue;
        }
#endif
        return t;
    }

    return -1;
}

/**
 * \brief   Remove cron job
 * \param[in]   expr    cron expression, it needs to end with '\t'
 * \param[in]   pfn job callback function
 * \param[in]   pUser   the parameter passed to the callback function
 * \return      -1 for error
 */
int32_t cron_job_add(char *expr, cron_job_cb_fn *pfn, void *pUser)
{
    char *ptr;
    cron_job_t *ptJob;
    time_t cur;

    ptJob = JOB_MALLOC(sizeof(cron_job_t));
    if (NULL == ptJob) {
        return -1;
    }

    MEM_ZERO(ptJob, sizeof(cron_job_t));
#if defined(CRON_SUPPORT_SECOND)
    ptr = ParseField("Secs", ptJob->tLine.cl_Secs, 60,  0, NULL,   expr);
    ptr = ParseField("Mins", ptJob->tLine.cl_Mins, 60,  0, NULL,   ptr);
#else
    ptr = ParseField("Mins", ptJob->tLine.cl_Mins, 60,  0, NULL,   expr);
#endif
    ptr = ParseField("Hrs",  ptJob->tLine.cl_Hrs,  24,  0, NULL,   ptr);
    ptr = ParseField("Days", ptJob->tLine.cl_Days, 32,  0, NULL,   ptr);
    ptr = ParseField("Mons", ptJob->tLine.cl_Mons, 12, -1, MonAry, ptr);
    ptr = ParseField("Week", ptJob->tLine.cl_Dow,   7,  0, DowAry, ptr);

    if (NULL == ptr) {
        PRINTF("ParseField failed!");
        JOB_FREE(ptJob);
        return -1;
    }

    cur = time(NULL);
    /*! Limit maximum search time depth */
    ptJob->tNext = next_time(&ptJob->tLine, cur, cur + SEC_LIMITS);
    
    ptJob->pfnJob = pfn;
    ptJob->pUser = pUser;
    ptJob->nId = job_id();
    k_list_add_tail(&ptJob->tLink, &s_tJobHead);

    return 0;
}

/**
 * \brief   Add cron job
 * \param[in]   nId job id
 * \return      -1 for error
 */
int32_t cron_job_remove(int32_t nId)
{
    cron_job_t *ptJob, *ptTmp;

    k_list_for_each_entry_safe(ptJob, ptTmp, &s_tJobHead, tLink) {
        if ((ptJob->nId == nId) || (CRON_JOB_ID_ALL == nId)) {
            k_list_del(&ptJob->tLink);
            JOB_FREE(ptJob);
            return nId;
        }
    }
    return -1;
}

static int32_t match_line(CronLine *ptline, struct tm *tm)
{
    if((ptline->cl_Mins[tm->tm_min] == 1)  &&
       (ptline->cl_Hrs[tm->tm_hour] == 1)  &&
       (ptline->cl_Days[tm->tm_mday] == 1) &&
       (ptline->cl_Mons[tm->tm_mon] == 1)  &&
       (ptline->cl_Dow[tm->tm_wday] == 1)) {
#if defined(CRON_SUPPORT_SECOND)
        if (ptline->cl_Secs[tm->tm_sec] == 1) {
            return 1;
        }
#else
        return 1;
#endif
    }
    return 0;
}

/**
 * \brief   User job cycle execution function, the notification function
 *  is called when the cron expression is matched to a point in time
 * \param[in]   pfnNotifier job notification callback function
 * \return
 */
void cron_job_run_as_notifier(cron_job_notifier_fn *pfnNotifier)
{
    cron_job_t *ptJob, *ptTmp;
    time_t cur;
    struct tm tm;

    if (k_list_empty(&s_tJobHead) || (NULL == pfnNotifier)) {
        return ;
    }

    cur = time(NULL);
    LOCALTIME_S(&tm, &cur);

    k_list_for_each_entry_safe(ptJob, ptTmp, &s_tJobHead, tLink) {
        if (!ptJob->tBits.bIsStop &&
            (cur >= ptJob->tNext)) {
            if (match_line(&ptJob->tLine, &tm)) {
#if defined(CRON_SUPPORT_SECOND)
                cur += 1;   /*! enter the next timing cycle */
#else
                cur += 60;   /*! enter the next timing cycle */
#endif
                ptJob->tNext = next_time(&ptJob->tLine, cur, cur + SEC_LIMITS);
                pfnNotifier(ptJob->nId);
            }
        }
    }

}

/**
 * \brief   User job cycle execution function
 * \return
 */
void cron_job_run(void)
{
    cron_job_t *ptJob, *ptTmp;
    time_t cur;
    struct tm tm;

    if (k_list_empty(&s_tJobHead)) {
        return ;
    }

    cur = time(NULL);
    LOCALTIME_S(&tm, &cur);

    k_list_for_each_entry_safe(ptJob, ptTmp, &s_tJobHead, tLink) {
        if (!ptJob->tBits.bIsStop && (cur >= ptJob->tNext)) {
            if (match_line(&ptJob->tLine, &tm)) {
#if defined(CRON_SUPPORT_SECOND)
                cur += 1;   /*! enter the next timing cycle */
#else
                cur += 60;   /*! enter the next timing cycle */
#endif
                ptJob->tNext = next_time(&ptJob->tLine, cur, cur + SEC_LIMITS);
                ptJob->pUser != NULL ? ptJob->pfnJob(ptJob->pUser) : 0;
            }
        }
    }

}

/**
 * \brief   Stops the specified cron task
 * \param[in]   nId job id
 * \return
 */
void cron_job_stop(int32_t nId)
{
    cron_job_t *ptJob, *ptTmp;

    k_list_for_each_entry_safe(ptJob, ptTmp, &s_tJobHead, tLink) {
        if ((ptJob->nId == nId) || (CRON_JOB_ID_ALL == nId)) {
            ptJob->tBits.bIsStop = 1;
        }
    }
}

/**
 * \brief   Resume the specified cron task
 * \param[in]   nId job id
 * \return
 */
void cron_job_resume(int32_t nId)
{
    cron_job_t *ptJob, *ptTmp;

    k_list_for_each_entry_safe(ptJob, ptTmp, &s_tJobHead, tLink) {
        if ((ptJob->nId == nId) || (CRON_JOB_ID_ALL == nId)) {
            ptJob->tBits.bIsStop = 0;
        }
    }
}

/**
 * \brief   Test cron expression
 * \param[in]   expr        cron expression, it needs to end with '\t'
 * \param[in]   nResultNum  number of output results
 * \return
 */
void cron_expr_test(char *expr, int32_t nResultNum)
{
    char *ptr;
    CronLine line;
    time_t cur, next;
    struct tm tm;
    int32_t i;

    MEM_ZERO(&line, sizeof(line));
    PRINTF("expr: %s\n", expr);

#if defined(CRON_SUPPORT_SECOND)
    ptr = ParseField("Secs", line.cl_Secs, 60,  0, NULL,   expr);
    ptr = ParseField("Mins", line.cl_Mins, 60,  0, NULL,   ptr);
#else
    ptr = ParseField("Mins", line.cl_Mins, 60,  0, NULL,   expr);
#endif
    ptr = ParseField("Hrs",  line.cl_Hrs,  24,  0, NULL,   ptr);
    ptr = ParseField("Days", line.cl_Days, 32,  0, NULL,   ptr);
    ptr = ParseField("Mons", line.cl_Mons, 12, -1, MonAry, ptr);
    ptr = ParseField("Week", line.cl_Dow,   7,  0, DowAry, ptr);
    if (NULL == ptr) {
        return;
    }

    cur = time(NULL);

    for (i = 0; i < nResultNum; i++) {
        next = next_time(&line, cur, cur + SEC_LIMITS);
#if defined(CRON_SUPPORT_SECOND)
        cur = next + 1;   /*! enter the next timing cycle */
#else
        cur = next + 60;
#endif
        LOCALTIME_S(&tm, &next);
        PRINTF("%02d-%02d-%02d %02d:%02d:%02d\n",
               tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
               tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
}

void test(void)
{    
#if defined(CRON_SUPPORT_SECOND)
    cron_expr_test("*/2 * * * * *\t", 5);
    cron_expr_test("0 */2 * * * *\t", 5);
    cron_expr_test("0 0 2 1 * *\t", 5);
    cron_expr_test("0 15 10 * * mon-fri\t", 5);
    cron_expr_test("0/2 * * * * *\t", 5);
    cron_expr_test("0 0 10,14,16 * * *\t", 5);
    cron_expr_test("0 */30 9-17 * * *\t", 5);
    cron_expr_test("0 0 12 * * wed\t", 5);
    cron_expr_test("0 0 12 * * *\t", 5);
    cron_expr_test("0 15 10 * * *\t", 5);
    cron_expr_test("0 * 14 * * *\t", 5);
    cron_expr_test("0 */5 14 * * *\t", 5);
    cron_expr_test("0 */5 14,18 * * *\t", 5);
    cron_expr_test("0 0-5 14 * * *\t", 5);
    cron_expr_test("0 10,44 14 * 3 wed\t", 5);
    cron_expr_test("0 15 10 * * mon-fri\t", 5);
    cron_expr_test("0 15 10 15 * *\t", 5);
#else
    /*! online test tool https://tool.lu/crontab/ */
    cron_expr_test("*/2 * * * *\t", 5);
    cron_expr_test("0 2 1 * *\t", 5);
    cron_expr_test("15 10 * * mon-fri\t", 5);
    cron_expr_test("0 10,14,16 * * *\t", 5);
    cron_expr_test("*/30 9-17 * * *\t", 5);
    cron_expr_test("0 12 * * wed\t", 5);
    cron_expr_test("0 12 * * *\t", 5);
    cron_expr_test("15 10 * * *\t", 5);
    cron_expr_test("0 14 * * *\t", 5);
    cron_expr_test("*/5 14 * * *\t", 5);
    cron_expr_test("*/5 14,18 * * *\t", 5);
    cron_expr_test("0-5 14 * * *\t", 5);
    cron_expr_test("10,44 14 * 3 wed\t", 5);
    cron_expr_test("15 10 * * mon-fri\t", 5);
    cron_expr_test("15 10 15 * *\t", 5);
#endif

}

/*************************** End of file ****************************/
