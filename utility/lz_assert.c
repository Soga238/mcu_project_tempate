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
*       lz_assert.c *                                                *
*                                                                    *
**********************************************************************
*/

#include "./lz_assert.h"
#include "../usr_app/usr_app_cfg.h"

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

void assert_handle(uint8_t *expr, uint8_t *file, uint32_t line)
{
    SYSLOG_F("assert(%s): %s line: %d", expr, file, line);
        
    while(1) {
        ;
    }
}

/*************************** End of file ****************************/
