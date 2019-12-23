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
*       multi_host_port.h *                                          *
*                                                                    *
**********************************************************************
*/

#include "./multi_host_utilities.h"
#include "./dlink/dlink.h"

#include "cmsis_os2.h"

/*********************************************************************
*
*       Configuation, default
*
**********************************************************************
*/

#define RAW_DATA_MAXIMUM_NUMBER     24
#define REQUEST_MAXIMUM_NUMBER      16
#define RESPONSE_MAXIMUN_NUMBER     16
#define LINK_NODE_MAIMUM_NUMBER     16

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

osMemoryPoolId_t s_tMemPoolRawDataId;
osMemoryPoolAttr_t s_tMemPoolRawDataAttr = {
    .name = "raw data"
};

osMemoryPoolId_t s_tMemPoolProxyRequestId;
osMemoryPoolAttr_t s_tMemPoolRawRequestAttr = {
    .name = "request"
};

osMemoryPoolId_t s_tMemPoolProxyResponseId;
osMemoryPoolAttr_t s_tMemPoolResponseAttr = {
    .name = "response"
};

osMemoryPoolId_t s_tMemPoolDlinkId;
static osMemoryPoolAttr_t s_tDlinkAttr = {
    .name = "dlink"
};

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

WEAK uint16_t proxy_send(uint8_t chPort, const uint8_t *pchBuf, uint16_t hwLength)
{
    return 0;
}

WEAK int32_t proxy_check_send_complete(uint8_t chPort)
{
    return 0;
}

WEAK uint16_t proxy_recv(uint8_t chPort, uint8_t *pchBuf, uint16_t hwLength)
{
    return 0;
}

raw_data_t *raw_data_malloc(void)
{
    raw_data_t *ptRawData = NULL;
    
    if (NULL == s_tMemPoolRawDataId) {
        s_tMemPoolRawDataId = osMemoryPoolNew(RAW_DATA_MAXIMUM_NUMBER, sizeof(raw_data_t), &s_tMemPoolRawDataAttr);
    }
    
    ptRawData = osMemoryPoolAlloc(s_tMemPoolRawDataId, 0);
        
    return ptRawData;
}

uint32_t raw_data_free_size(void)
{
    return RAW_DATA_MAXIMUM_NUMBER - osMemoryPoolGetCount(s_tMemPoolRawDataId);
}

void raw_data_free(raw_data_t *ptRawData)
{    
    osMemoryPoolFree(s_tMemPoolRawDataId, ptRawData);
}

proxy_request_t *proxy_mb_request_malloc(void)
{
    if (NULL == s_tMemPoolProxyRequestId) {
        s_tMemPoolProxyRequestId = osMemoryPoolNew(REQUEST_MAXIMUM_NUMBER, sizeof(proxy_request_t), &s_tMemPoolRawRequestAttr);
    }

    return osMemoryPoolAlloc(s_tMemPoolProxyRequestId, 0);
}

void proxy_mb_request_free(proxy_request_t *ptRequest)
{
    osMemoryPoolFree(s_tMemPoolProxyRequestId, ptRequest);
}

proxy_response_t *proxy_mb_response_malloc(void)
{
    if (NULL == s_tMemPoolProxyResponseId) {
        s_tMemPoolProxyResponseId = osMemoryPoolNew(RESPONSE_MAXIMUN_NUMBER, sizeof(proxy_response_t), &s_tMemPoolResponseAttr);
    }

    return osMemoryPoolAlloc(s_tMemPoolProxyResponseId, 0);
}

void proxy_mb_response_free(proxy_response_t *ptResponse)
{
    osMemoryPoolFree(s_tMemPoolProxyResponseId, ptResponse);
}

void *dlink_malloc(void)
{
    if (NULL == s_tMemPoolDlinkId) {
        s_tMemPoolDlinkId = osMemoryPoolNew(LINK_NODE_MAIMUM_NUMBER, sizeof(dlink_node_t), &s_tDlinkAttr);
    }

    return osMemoryPoolAlloc(s_tMemPoolDlinkId, 0);
}

void dlink_free(void *pMem)
{
    osMemoryPoolFree(s_tMemPoolDlinkId, pMem);
}

/*************************** End of file ****************************/
