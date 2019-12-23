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
*       multi_host_cache.h *                                         *
*                                                                    *
**********************************************************************
*/
#ifndef MULTI_HOST_CACHE_H
#define MULTI_HOST_CACHE_H

#include "./multi_host_request_response.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct __cache_data cache_data_t;
struct __cache_data {
    uint8_t     chExpirationCounter;    // 过期计数器
    uint8_t     chPort;                 // ID对应从机所在端口

    uint8_t     chID;                   // 独一无二的内部从机ID， 避免有重复（站号）的设备
    uint8_t     chCode;                 // 功能码
    uint16_t    hwDataAddr;
    uint16_t    hwDataNumber;
    union {
        uint16_t   *phwDataBuffer;
        uint8_t    *pchDataBuffer;
    };
};

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/
extern_simple_fsm(update_cache_record,
def_params(
    uint32_t wTimeout;
    uint8_t chFailedCounter;
    cache_data_t tCacheData;
    raw_data_t *ptRawData;
    proxy_request_t *ptRequest;
    proxy_response_t *ptResponse;
    fsm(safe_read_and_write_operation) tOperation;
))
extern_fsm_initialiser(update_cache_record)
extern_fsm_implementation(update_cache_record)

extern_simple_fsm(read_cache_data,
def_params(
    proxy_request_t *ptRequest;
    proxy_response_t *ptResponse;
    raw_data_t *ptRaw;
    uint32_t wDelayCounter;
))

extern_fsm_initialiser(read_cache_data, args(proxy_request_t *ptRequest))
extern_fsm_implementation(read_cache_data, args(proxy_response_t **pptResponse))

extern_simple_fsm(delete_expired_cache_record,
def_params(
    cache_data_t tCacheData;
    uint16_t hwTimeCounter;
))

extern_fsm_initialiser(delete_expired_cache_record)
extern_fsm_implementation(delete_expired_cache_record)

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

extern int32_t cache_record_init(void);

extern int32_t create_cache_record(const proxy_request_t *ptRequest);

extern int32_t delete_cache_record_by_request(const proxy_request_t *ptRequest);

extern int32_t refresh_cache_record(const proxy_request_t *ptRequest, const proxy_response_t *ptResponse);

#endif

/*************************** End of file ****************************/
