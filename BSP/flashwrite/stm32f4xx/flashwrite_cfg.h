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
*       flashwrite_cfg.h *                                           *
*                                                                    *
**********************************************************************
*/

#include "..\bsp_cfg.h"

#if defined(STM32F401xC)
#define FLASH_SECTOR_TABLE_SIZE 6   // Number of flash sectors, defined by user
#else
#define FLASH_SECTOR_TABLE_SIZE 0
#endif

/*************************** End of file ****************************/
