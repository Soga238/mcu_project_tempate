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
*       proxy_request_queue.c *                                      *
*                                                                    *
**********************************************************************
*/


#include "./proxy_request_queue.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/


bool byte_queue_init(proxy_request_queue_t* ptQueue, proxy_request_t* pchBuffer, uint8_t chSize)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if ((ptQ == NULL) || (pchBuffer == NULL) || (chSize == 0)) {
        return false;
    }

    ptQ->ptBuf = pchBuffer;
    ptQ->chSize = chSize;
    ptQ->chHead = 0;
    ptQ->chTail = 0;
    ptQ->chLength = 0;

    ptQ->chPeekedCount = 0;
    ptQ->chPeekedCount = ptQ->chHead;

    return true;
}

bool byte_queue_in(proxy_request_queue_t* ptQueue, proxy_request_t* pchVal)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if ((ptQ == NULL) || (pchVal == NULL)) {
        return false;
    }

    if (ptQ->chLength < ptQ->chSize) {
        ptQ->chLength++;
        ptQ->ptBuf[ptQ->chTail] = *pchVal;
        if (ptQ->chTail == (ptQ->chSize - 1)) {
            ptQ->chTail = 0;
        } else {
            ptQ->chTail++;
        }
        return true;
    }

    return false;
}

bool byte_queue_out(proxy_request_queue_t* ptQueue, proxy_request_t* pchVal)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if ((ptQ == NULL) || (pchVal == NULL)) {
        return false;
    }

    if (ptQ->chLength) {
        ptQ->chLength--;

        *pchVal = ptQ->ptBuf[ptQ->chHead];
        if (ptQ->chHead == (ptQ->chSize - 1)) {
            ptQ->chHead = 0;
        } else {
            ptQ->chHead++;
        }

        // reset peek
        ptQ->chPeekHead = ptQ->chHead;
        ptQ->chPeekedCount = 0;

        return true;
    }

    return false;
}

bool byte_queue_is_empty(proxy_request_queue_t* ptQueue)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if (ptQ == NULL) {
        return false;
    }

    return 0 == ptQ->chLength;
}

bool byte_queue_peek(proxy_request_queue_t* ptQueue, proxy_request_t* pchVal)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if ((ptQ == NULL) || (pchVal == NULL)) {
        return false;
    }

    if (ptQ->chPeekedCount < ptQ->chLength) {
        *pchVal = ptQ->ptBuf[ptQ->chPeekHead];

        if (++ptQ->chPeekHead >= ptQ->chSize) {
            ptQ->chPeekHead = 0;
        }

        ptQ->chPeekedCount += 1;

        return true;
    }

    return false;
}

bool byte_queue_reset_peek(proxy_request_queue_t* ptQueue)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if (ptQ != NULL) {
        // reset peek
        ptQ->chPeekHead = ptQ->chHead;
        ptQ->chPeekedCount = 0;
        return true;
    }
    
    return false;
}

bool byte_queue_get_all_peeked(proxy_request_queue_t* ptQueue)
{
    proxy_request_queue_t* ptQ = (proxy_request_queue_t*)ptQueue;

    if (ptQ != NULL) {
        if (0 < ptQ->chPeekedCount) {
            ptQ->chHead = ptQ->chPeekHead;
            ptQ->chLength -= ptQ->chPeekedCount;
            ptQ->chPeekedCount = 0;
        }
        return true;
    }
    
    return false;  
}

uint8_t byte_queue_size(proxy_request_queue_t* ptQueue)
{
    return (NULL != ptQueue) ? ptQueue->chLength : 0;
}

/*************************** End of file ****************************/
