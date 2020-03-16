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
*       at_assert.c *                                                *
*                                                                    *
**********************************************************************
*/

#include "..\include\at_assert.h"

#include "cmsis_os2.h"
/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

void at_assert_failed(uint8_t *expr, uint8_t *file, uint32_t line)
{
    ALOG_F("art(%s): %s line: %d", expr, file, line);

    osDelay(200);   /*! delay for display more*/

    while(1) {
        ;
    }
}

/*************************** End of file ****************************/
