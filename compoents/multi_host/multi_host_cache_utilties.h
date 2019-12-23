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
*       multi_host_cache_utilities.c *                               *
*                                                                    *
**********************************************************************
*/
#ifndef __MULTI_HOST_CACHE_UTILTIES_H
#define __MULTI_HOST_CACHE_UTILTIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./multi_host_cache.h"
#include "./cache_data_list.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
    uint8_t     chID;
    uint8_t     chCode;
    uint16_t    hwDataAddr;
    uint16_t    hwDataNumber;
} search_cache_data_t;

typedef struct {
    uint8_t     chID;
    uint8_t     chCode;
    uint16_t    hwDataAddr;
    uint16_t    hwDataNumber;
    union {
        uint16_t  *phwDataBuffer;
        uint8_t   *pchDataBuffer;
    };
} update_cache_data_t;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

extern int32_t find_compatible_cache_record_hook(void *res, void *data);

extern int32_t find_equal_cache_record_hook(void *res, void *data);

extern int32_t destroy_cache_data_hook(void *ptCacheData);

extern void decrease_expiration_counter_in_list(cache_data_list_t *ptList);

extern int32_t delete_expired_record_in_list(cache_data_list_t *ptList);

extern int32_t delete_equal_request_in_list(cache_data_list_t *ptList, proxy_request_t *ptRequest);

extern uint8_t calc_storage_space_in_byte(uint8_t chCode, uint16_t hwDataNumber);

extern int32_t cache_data_transform_request(const cache_data_t *ptCache, proxy_request_t *ptRequest);;

extern int32_t refresh_cache_data_object_hook(void *res, void *data);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
