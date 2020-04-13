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
*       at_utils.c *                                                 *
*                                                                    *
**********************************************************************
*/

/* Includes --------------------------------------------------------*/
#include "..\include\at_utils.h"
#include "..\include\at_port.h"
#include <stdio.h>

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
/* Private macro ---------------------------------------------------*/
#define PRINT_BUF_SIZE  32

/* Private variables -----------------------------------------------*/
static uint8_t s_chSendBuf[PRINT_BUF_SIZE];
static int32_t s_nLastCmdLength = 0;

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

const uint8_t *at_get_last_cmd(int32_t *pnLength)
{
    if (strstr((char *)s_chSendBuf, "\r\n")) {
        s_nLastCmdLength -= 2;
        s_chSendBuf[s_nLastCmdLength] = 0;
    }
    *pnLength = s_nLastCmdLength;
    return s_chSendBuf;
}

const uint8_t *at_extract_last_cmd(uint8_t *pchBuf, int32_t *pnLength)
{
    int32_t i;

    for (i = 0; i < sizeof(s_chSendBuf); i++) {
        if (0 == pchBuf[i]) {
            break;
        } else if ((pchBuf[i] != '\r') && (pchBuf[i] != '\n')) {
            s_chSendBuf[i] = pchBuf[i];
        } else {
            s_chSendBuf[i] = 0;
        }
    }

    *pnLength = i;
    return s_chSendBuf;
}

int32_t at_vprintf(void *client, const char *format, va_list args)
{
    s_nLastCmdLength = vsnprintf((char *)s_chSendBuf, sizeof(s_chSendBuf), format, args);
    at_client_obj_send(client, s_chSendBuf, s_nLastCmdLength);
    return s_nLastCmdLength;
}

/*************************** End of file ****************************/
