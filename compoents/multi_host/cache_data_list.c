#include "./cache_data_list.h"

/**
 *      |data|next| -> |data|next| -> |0|NULL|
 */

int32_t cache_data_list_init(cache_data_list_t *ptList, cache_node_t *ptBuffer, int32_t nBufferSize)
{
    if ((NULL == ptList) || (NULL == ptBuffer) || (0 >= nBufferSize)) {
        return -1;
    }

    for (int32_t i = 0; i < nBufferSize; i++) {
        ptBuffer[i].ptNext = ((i + 1) == nBufferSize) ? NULL : ptBuffer + i;
        ptBuffer[i].bIsUsed = false;
    }

    ptList->nBufferSize = nBufferSize;
    ptList->ptBuffer = ptBuffer;

    ptList->ptHead = NULL;
    ptList->ptPeekHead = ptList->ptHead;
    ptList->nPeekedCount = 0;
    ptList->nLength = 0;

    return 0;
}

static cache_node_t *node_malloc(cache_data_list_t *ptList)
{
    cache_node_t *ptNode = NULL;

    for (int32_t i = 0; i < ptList->nBufferSize; i++) {
        if (!ptList->ptBuffer[i].bIsUsed) {
            ptNode = &ptList->ptBuffer[i];
            ptNode->bIsUsed = true;
            break;
        }
    }
    return ptNode;
}

static void node_free(cache_node_t *ptNode)
{
    ptNode->bIsUsed = false;
}

static cache_node_t *get_node(cache_data_list_t *ptList, int32_t nIndex)
{
    cache_node_t *ptNode = ptList->ptHead;

    for (int32_t i = 0; i < nIndex; i++) {
        ptNode = ptNode->ptNext;
    }
    return ptNode;
}

int32_t cache_data_list_size(cache_data_list_t *ptList)
{
    return (NULL != ptList) ? ptList->nLength : -1;
}

int32_t cache_data_list_insert(cache_data_list_t *ptList, int32_t nIndex, cache_data_t *ptData)
{
    cache_node_t *ptNode = NULL;
    cache_node_t *ptFrontNode = NULL;

    if (NULL == ptList) {
        return -1;
    } else if ((0 > nIndex) || (nIndex > ptList->nLength)) {
        return -1;
    }

    ptNode = node_malloc(ptList);
    if (NULL == ptNode) {
        return -1;
    }

    ptNode->tData = *ptData;

    if (0 == nIndex) {
        ptNode->ptNext = (0 == ptList->nLength) ? NULL : ptList->ptHead;
        ptList->ptHead = ptNode;
    } else {
        ptFrontNode = get_node(ptList, nIndex - 1);
        ptNode->ptNext = ptFrontNode->ptNext;
        ptFrontNode->ptNext = ptNode;
    }

    // TODO nLength = 0
    ptList->ptPeekHead = ptList->ptHead;
    ptList->nPeekedCount = 0;
    ptList->nLength++;

    return 0;
}

int32_t cache_data_list_insert_tail(cache_data_list_t *ptList, cache_data_t *ptData)
{
    int32_t nLength = ptList->nLength;

    return 0 == cache_data_list_insert(ptList, nLength, ptData) ? nLength : -1;
}

int32_t cache_data_list_insert_head(cache_data_list_t *ptList, cache_data_t *ptData)
{
    return cache_data_list_insert(ptList, 0, ptData);
}

int32_t cache_data_list_get(cache_data_list_t *ptList, int32_t nIndex, cache_data_t *ptData)
{
    cache_node_t *ptNode = NULL;
    
    if (NULL == ptList) {
        return -1;
    } else if ((0 > nIndex) || (nIndex >= ptList->nLength)) {
        return -1;
    }

    ptNode = get_node(ptList, nIndex);
    *ptData = ptNode->tData;

    return 0;
}

int32_t cache_data_list_delete(cache_data_list_t *ptList, int32_t nIndex)
{
    cache_node_t *ptNode = NULL;
    cache_node_t *ptFrontNode = NULL;

    if (NULL == ptList) {
        return -1;
    } else if ((0 > nIndex) || (nIndex >= ptList->nLength)) {
        return -1;
    }

    ptNode = get_node(ptList, nIndex);

    if (0 == nIndex) {
        ptList->ptHead = ptNode->ptNext; // remove head
    } else {
        ptFrontNode = get_node(ptList, nIndex - 1);
        ptFrontNode->ptNext = ptNode->ptNext;
    }

    node_free(ptNode);

    ptList->ptPeekHead = ptList->ptHead;
    ptList->nPeekedCount = 0;
    ptList->nLength--;

    return 0;
}

int32_t cache_data_list_peek(cache_data_list_t *ptList, cache_data_t *ptData)
{
    if ((NULL == ptList) || (NULL == ptData)) {
        return -1;
    }

    if (ptList->nPeekedCount < ptList->nLength) {
        *ptData = ptList->ptPeekHead->tData;
        ptList->ptPeekHead = ptList->ptPeekHead->ptNext;
        ptList->nPeekedCount += 1;
        return 0;
    }

    return -1;
}

int32_t cache_data_list_peek_reset(cache_data_list_t *ptList)
{
    if (NULL != ptList) {
        ptList->nPeekedCount = 0;
        ptList->ptPeekHead = ptList->ptHead;
        return 0;
    }

    return -1;
}

int32_t cache_data_list_delete_all_peeked(cache_data_list_t *ptList)
{
    if (NULL != ptList) {
        ptList->ptHead = ptList->ptPeekHead;
        ptList->nPeekedCount = 0;
        return 0;
    }

    return -1;
}

int32_t cache_data_list_search(cache_data_list_t *ptList, compare_fn *fnCompare, void *pData)
{
    cache_node_t *ptNode = NULL;

    if ((NULL == ptList) || (NULL == fnCompare)) {
        return -1;
    }

    for (int32_t i = 0; i < ptList->nLength; i++) {
        ptNode = get_node(ptList, i);
        if (0 == fnCompare(&ptNode->tData, pData)) {
            return i;
        }
    }

    return -1;
}

cache_data_t *cache_data_list_search_data_ptr(cache_data_list_t *ptList, compare_fn *fnCompare, void *pData)
{
    cache_node_t *ptNode = NULL;

    if ((NULL == ptList) || (NULL == fnCompare)) {
        return NULL;
    }

    for (int32_t i = 0; i < ptList->nLength; i++) {
        ptNode = get_node(ptList, i);
        if (0 == fnCompare(&ptNode->tData, pData)) {
            return &ptNode->tData;
        }
    }

    return NULL;
}

void cache_data_list_travel(cache_data_list_t *ptList, travel_fn *fnHook, void *pData)
{
    cache_node_t *ptNode = NULL;

    if ((NULL == ptList) || (NULL == fnHook)) {
        return ;
    }

    for (int32_t i = 0; i < ptList->nLength; i++) {
        ptNode = get_node(ptList, i);
        fnHook(&ptNode->tData, pData);
    }

}

int32_t cache_data_list_delete_equal(cache_data_list_t *ptList, equal_fn *fnHook, void *pData,
                                     destroy_fn *fnDestroy)
{
    cache_node_t *ptNode = NULL;
    cache_node_t *ptFrontNode = NULL;
    bool bDeleted = false;

    if ((NULL == ptList) || (NULL == fnHook)) {
        return -1;
    }

    ptNode = ptList->ptHead;

    for (volatile int32_t i = 0; i < ptList->nLength; i++) {
        if (0 != fnHook(&ptNode->tData, pData)) { // TODO free tData
            ptNode = ptNode->ptNext;
            continue;
        }

        if (0 == i) {
            ptList->ptHead = ptNode->ptNext;
        } else {
            ptFrontNode = get_node(ptList, i - 1);
            ptFrontNode->ptNext = ptNode->ptNext;
        }

        if (NULL != fnDestroy) {
            fnDestroy(&ptNode->tData);
        }

        node_free(ptNode);
        ptNode = (0 == i) ? ptList->ptHead : ptFrontNode->ptNext;
        ptList->nLength--;
        i--;
        bDeleted = true;
    }

    if (bDeleted) {
        ptList->ptPeekHead = ptList->ptHead;
        ptList->nPeekedCount = 0;
    }
    
    return 0;
}
