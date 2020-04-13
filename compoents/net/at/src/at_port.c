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
*       at_port.c *                                                  *
*                                                                    *
**********************************************************************
*/

/* Includes --------------------------------------------------------*/
#include "..\include\at_port.h"
#include "..\service\ring_buf\ring_buf.h"
#include "..\bsp\uart\uart.h"
#include <stdio.h>

/* Global variables ------------------------------------------------*/
extern char_ring_t g_tRingBuf;
extern uart_dev_t g_tATUart;

/* Private typedef -------------------------------------------------*/
typedef struct {
    int32_t nPort;
    struct at_client tClient;
} at_client_map_t;

/* Private define --------------------------------------------------*/
/* Private macro ---------------------------------------------------*/

/* Private variables -----------------------------------------------*/
static at_client_map_t s_AtClientTable[AT_CLIENT_NUM_MAX] = {
    {.nPort = 0}
};

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

int32_t at_client_get_char(at_client_t client, uint8_t* pchByte)
{
    if (ringbuf_get(&g_tRingBuf, pchByte, 1)) {
        return 1;
    } else {
        osDelay(5);
    }
    return 0;
}

int32_t at_client_obj_send(at_client_t client, uint8_t* pchBuf, int32_t nLength)
{
    if (0 < nLength) {
        xhal_uart_send_in_dma_mode(&g_tATUart, pchBuf, nLength, 1000);
    }
    return 0;
}

at_client_t at_client_get(int32_t nPort)
{
    for (int32_t i = 0; i < AT_CLIENT_NUM_MAX; i++) {
        if (s_AtClientTable[i].nPort == nPort) {
            return &s_AtClientTable[i].tClient;
        }
    }
    return NULL;
}

at_client_t at_client_get_first(void)
{
    return (0 < AT_CLIENT_NUM_MAX) ? &s_AtClientTable[0].tClient : NULL;
}

/*************************** End of file ****************************/
