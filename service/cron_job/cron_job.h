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
#ifndef CRON_JOB_H
#define CRON_JOB_H

#ifdef __cplusplus
extern "C" {
#endif

// Copy from https://corntab.com/man/crontab.5

// The time and date fields are:

//     field          allowed values
//     -----          --------------
//     second         0-59
//     minute         0-59
//     hour           0-23
//     day of month   1-31
//     month          1-12 (or names, see below)
//     day of week    0-6 (0 is Sunday, or use names, unsupport 7)

// A field may contain an asterisk (*), which always stands for
// "first-last".

// Ranges of numbers are allowed.  Ranges are two numbers separated with
// a hyphen.  The specified range is inclusive.  For example, 8-11 for
// an 'hours' entry specifies execution at hours 8, 9, 10, and 11.

// Step values can be used in conjunction with ranges.  Following a
// range with "/<number>" specifies skips of the number's value through
// the range.  For example, "0-23/2" can be used in the 'hours' field to
// specify command execution for every other hour (the alternative in
// the V7 standard is "0,2,4,6,8,10,12,14,16,18,20,22").  Step values
// are also permitted after an asterisk, so if specifying a job to be
// run every two hours, you can use "*/2".

// Names can also be used for the 'month' and 'day of week' fields.  Use
// the first three letters of the particular day or month (case does not
// matter).  Ranges or lists of names are not allowed.

// The "sixth" field (the rest of the line) specifies the command to be
// run.  The entire command portion of the line, up to a newline or a
// "%" character, will be executed by /bin/sh or by the shell specified
// in the SHELL variable of the cronfile.  A "%" character in the
// command, unless escaped with a backslash (\), will be changed into
// newline characters, and all data after the first % will be sent to
// the command as standard input.

// Note: The day of a command's execution can be specified in the
// following two fields — 'day of month', and 'day of week'.  If both
// fields are restricted (i.e., do not contain the "*" character), the
// command will be run when either field matches the current time.  For
// example,
// "30 4 1,15 * 5" would cause a command to be run at 4:30 am on the 1st
// and 15th of each month, plus every Friday. 
 
/* Includes --------------------------------------------------------*/
#include <stdint.h>

/* Global define ---------------------------------------------------*/
#define CRON_JOB_ID_ALL     0xFFFFFFFF  /*! all job objects */

/* Global macro ----------------------------------------------------*/
/* Global typedef --------------------------------------------------*/
typedef void (cron_job_cb_fn)(void *pUser);

typedef void (cron_job_notifier_fn)(int32_t nId);

/* Global variables ------------------------------------------------*/
/* Global function prototypes --------------------------------------*/

/**
 * \brief   Test cron expression
 * \param[in]   expr    cron expression, it needs to end with '\t'
 * \param[in]   nResultNum  number of output results
 * \return
 */
extern void
cron_expr_test(char *expr, int32_t nResultNum);

/**
 * \brief   Remove cron job
 * \param[in]   expr    cron expression, it needs to end with '\t'
 * \param[in]   pfn job callback function
 * \param[in]   pUser   the parameter passed to the callback function
 * \return      -1 for error
 */
extern int32_t
cron_job_add(char *expr, cron_job_cb_fn *pfn, void *pUser);

/**
 * \brief   Add cron job
 * \param[in]   nId job id
 * \return      -1 for error
 */
extern int32_t
cron_job_remove(int32_t nId);

/**
 * \brief   User job cycle execution function
 * \return
 */
extern void
cron_job_run(void);

/**
 * \brief   User job cycle execution function, the notification function
 *  is called when the cron expression is matched to a point in time
 * \param[in]   pfnNotifier job notification callback function
 * \return
 */
extern void
cron_job_run_as_notifier(cron_job_notifier_fn *pfnNotifier);

/**
 * \brief   Stops the specified cron task
 * \param[in]   nId job id
 * \return
 */
extern void
cron_job_stop(int32_t nId);

/**
 * \brief   Resume the specified cron task
 * \param[in]   nId job id
 * \return
 */
extern void
cron_job_resume(int32_t nId);

extern 
void test(void);

#ifdef __cplusplus
}
#endif
#endif

/*************************** End of file ****************************/
