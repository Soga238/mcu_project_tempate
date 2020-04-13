#ifndef __CACHE_DATA_LIST_H__
#define __CACHE_DATA_LIST_H__

#include "./multi_host_cache.h"

typedef struct cache_node cache_node_t;
struct cache_node {
    cache_node_t *ptNext;
    cache_data_t tData;
    bool bIsUsed;
};

typedef struct cache_data_list_t cache_data_list_t;
struct cache_data_list_t {
    cache_node_t *ptBuffer;         // 指向静态节点数组
    int32_t nBufferSize;            // 数组长度

    int32_t nLength;
    cache_node_t *ptHead;           // 头指针

    cache_node_t *ptPeekHead;
    int32_t nPeekedCount;
};

extern int32_t cache_data_list_init(cache_data_list_t *ptList, cache_node_t *ptBuffer, int32_t nBufferSize);

extern int32_t cache_data_list_size(cache_data_list_t *ptList);

extern int32_t cache_data_list_insert(cache_data_list_t *ptList, int32_t nIndex, cache_data_t *ptData);

extern int32_t cache_data_list_insert_tail(cache_data_list_t *ptList, cache_data_t *ptData);

extern int32_t cache_data_list_insert_head(cache_data_list_t *ptList, cache_data_t *ptData);

extern int32_t cache_data_list_get(cache_data_list_t *ptList, int32_t nIndex, cache_data_t *ptData);

extern int32_t cache_data_list_peek(cache_data_list_t *ptList, cache_data_t *ptData);

extern int32_t cache_data_list_peek_reset(cache_data_list_t *ptList);

extern int32_t cache_data_list_search(cache_data_list_t *ptList, compare_fn *fnCompare, void *pData);

extern cache_data_t *cache_data_list_search_data_ptr(cache_data_list_t *ptList, compare_fn *fnCompare, void *pData);

extern void cache_data_list_travel(cache_data_list_t *ptList, travel_fn *fnHook, void *pData);

extern int32_t cache_data_list_delete_equal(cache_data_list_t *ptList, equal_fn *fnHook, void *pData,
        destroy_fn *fnDestroy);

#endif
