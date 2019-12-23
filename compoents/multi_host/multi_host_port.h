#ifndef __MULTI_HOST_PORT_H
#define __MULTI_HOST_PORT_H

#include "./multi_host_utilities.h"
#include "../../service/heap/heap.h"

#define OBJECT_MALLOC(__SIZE)       port_malloc_4(__SIZE)
#define OBJECT_FREE(__PTR)          port_free_4(__PTR)

#define RAW_DATA_MALLOC()           raw_data_malloc()
#define RAW_DATA_FREE(__PTR)        do {raw_data_free(__PTR);}while(0)
#define RAW_DATA_FREE_SIZE()        raw_data_free_size()

#define PROXY_REQUEST_MALLOC()      proxy_mb_request_malloc()
#define PROXY_REQUEST_FREE(__PTR)   do {proxy_mb_request_free(__PTR);}while(0)

#define PROXY_RESPONSE_MALLOC()     proxy_mb_response_malloc()
#define PROXY_RESPONSE_FREE(__PTR)  do {proxy_mb_response_free(__PTR);}while(0)

extern uint16_t proxy_send(uint8_t chPort, const uint8_t *pchBuf, uint16_t hwLength);

extern uint16_t proxy_recv(uint8_t chPort, const uint8_t *pchBuf, uint16_t hwLength);

extern int32_t proxy_check_send_complete(uint8_t chPort);

extern raw_data_t *raw_data_malloc(void);

extern uint32_t raw_data_free_size(void);

extern void raw_data_free(raw_data_t *ptRawData);

extern proxy_request_t *proxy_mb_request_malloc(void);

extern void proxy_mb_request_free(proxy_request_t *ptRequest);

extern proxy_response_t *proxy_mb_response_malloc(void);

extern void proxy_mb_response_free(proxy_response_t *ptResponse);

extern void *dlink_malloc(void);

extern void dlink_free(void *pMem);

#endif
