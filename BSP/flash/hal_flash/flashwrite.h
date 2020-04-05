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
*       Flash * FLASH Write                                          *
*                                                                    *
**********************************************************************
*/
#ifndef FLASH_WRITE_H
#define FLASH_WRITE_H

#include <stdint.h>

extern uint8_t flash_write(uint32_t wFlashAddr, uint8_t *pchBuf, uint32_t wSize);

#endif

/*************************** End of file ****************************/
