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
*       request_queue.h *                                            *
*                                                                    *
**********************************************************************
*/

#ifndef PROXY_REQUEST_QUEUE_H
#define PROXY_REQUEST_QUEUE_H

#include "./multi_host_data_type.h"

typedef struct __proxy_request_queue proxy_request_queue_t;
struct __proxy_request_queue {
    uint8_t chHead;
    uint8_t chTail;
    uint8_t chSize;
    uint8_t chLength;
    
    proxy_request_t *ptBuf;
    
    uint8_t chPeekedCount;
    uint8_t chPeekHead;
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#endif

/*************************** End of file ****************************/
