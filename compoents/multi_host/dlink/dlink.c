#include ".\dlink.h"

#ifndef ABS
    #define ABS(n)      ((n) >= 0 ? (n) : (-n))
#endif // !ABS

#ifndef NULL
    #define NULL ((void *)0)
#endif // !NULL

static dlink_node_t *dlink_node_new(void *data)
{
    dlink_node_t *ptNode = (dlink_node_t *)MALLOC(sizeof(dlink_node_t));

    if (NULL != ptNode) {
        ptNode->ptPre = ptNode->ptNext = ptNode;    /* link to it self !*/
        ptNode->pData = data;
    }
    return ptNode;
}

static void dlink_node_free(dlink_node_t *ptNode)
{
    FREE(ptNode);
}

int32_t dlink_list_init(dlink_list_t *ptList)
{
    if (NULL != ptList) {
        ptList->nCount = 0;
        ptList->ptHead = NULL;
        return 0;
    }
    return -1;
}

static int32_t transform_index(dlink_list_t *ptList, int32_t nIndex)
{
    if (ABS(nIndex) >= ptList->nCount) {
        nIndex = 0;
    }
    return nIndex < 0 ? (nIndex + ptList->nCount) : nIndex;
}

static dlink_node_t *get_dlink_node(dlink_list_t *ptList, int32_t nIndex)
{
    dlink_node_t *ptNode = ptList->ptHead;
    int32_t nNewIndex = transform_index(ptList, nIndex);

    if (nNewIndex < (ptList->nCount >> 1)) {
        for (int32_t i = 0; i < nIndex; i++) {
            ptNode = ptNode->ptNext;
        }
    } else {
        for (int32_t i = nIndex; i < ptList->nCount; i++) {
            ptNode = ptNode->ptPre;
        }
    }
    return ptNode;
}

int32_t dlink_list_insert(dlink_list_t *ptList, int32_t nIndex, void *pdata)
{
    dlink_node_t *ptNode = NULL;
    dlink_node_t *ptNodeInsert = NULL;

    if (NULL == ptList) {
        return -1;
    }

    ptNodeInsert = dlink_node_new(pdata);
    if (NULL == ptNodeInsert) {
        return -1;
    }

    ptNode = get_dlink_node(ptList, nIndex);

    if (NULL != ptNode) {
        ptNode->ptPre->ptNext = ptNodeInsert;   /*insert pre */
        ptNodeInsert->ptPre = ptNode->ptPre;

        ptNode->ptPre = ptNodeInsert;
        ptNodeInsert->ptNext = ptNode;

        if ((0 == nIndex) || (nIndex < (-ptList->nCount))) {
            ptList->ptHead = ptNodeInsert;
        }
    } else {
        ptList->ptHead = ptNodeInsert;
    }

    ptList->nCount += 1;
    return 0;
}

void *dlink_list_get(dlink_list_t *ptList, int32_t nIndex)
{
    dlink_node_t *ptNode = NULL;

    if ((NULL == ptList) || (0 == ptList->nCount)) {
        return NULL;
    }

    if (ABS(nIndex) > ptList->nCount) {
        return NULL;
    }

    ptNode = get_dlink_node(ptList, nIndex);

    return ptNode->pData;
}

int32_t dlink_list_free(dlink_list_t *ptList, int32_t nIndex)
{
    dlink_node_t *ptNode = NULL;

    if ((NULL == ptList) || (0 == ptList->nCount)) {
        return -1;
    }

    if (ABS(nIndex) > ptList->nCount) {
        return -1;
    }

    ptNode = get_dlink_node(ptList, nIndex);

    if (1 == ptList->nCount) {
        ptList->ptHead = NULL;
    } else {
        ptNode->ptPre->ptNext = ptNode->ptNext;
        ptNode->ptNext->ptPre = ptNode->ptPre;
        if (ptNode == ptList->ptHead) {
            ptList->ptHead = ptNode->ptNext;
        }
    }

    dlink_node_free(ptNode);
    ptList->nCount -= 1;

    return 0;
}

void *dlink_list_pop(dlink_list_t *ptList, int32_t nIndex)
{
    void *pData = dlink_list_get(ptList, nIndex);

    dlink_list_free(ptList, nIndex);
    return pData;
}

int32_t dlink_list_delete(dlink_list_t *ptList)
{
    dlink_node_t *ptNode = NULL;

    if (NULL == ptList) {
        return -1;
    }

    while (ptList->nCount--) {
        ptNode = ptList->ptHead;
        ptList->ptHead = ptList->ptHead->ptNext;
        FREE(ptNode);
    }

    ptList->ptHead = NULL;
    ptList->nCount = 0;

    return 0;
}

int32_t dlink_list_is_empty(dlink_list_t *ptList)
{
    return NULL != ptList ? (ptList->nCount == 0) : -1;
}

int32_t dlink_list_size(dlink_list_t *ptList)
{
    return NULL != ptList ? ptList->nCount : -1;
}

int32_t dlink_list_search(dlink_list_t *ptList, compare_fn *fnCompare, void *pData)
{
    dlink_node_t *ptNode = NULL;

    if ((NULL == ptList) || (NULL == fnCompare)) {
        return -1;
    }

    ptNode = ptList->ptHead;

    for (int32_t i = 0; i < ptList->nCount; i++) {
        if (0 == (*fnCompare)(ptNode->pData, pData)) {
            return i;
        }
        ptNode = ptNode->ptNext;
    }

    return -1;
}

void dlink_list_travel(dlink_list_t *ptList, travel_fn *fnHook, void *pData)
{
    dlink_node_t *ptNode = NULL;

    if ((NULL == ptList) || (NULL == fnHook)) {
        return ;
    }

    ptNode = ptList->ptHead;

    for (int32_t i = 0; i < ptList->nCount; i++) {
        (*fnHook)(ptNode->pData, pData);
        ptNode = ptNode->ptNext;
    }

}

void dlink_list_delete_equal(dlink_list_t *ptList, travel_fn *fnHook,  void *pData,
                             destroy_fn *fnDestroy)
{
    dlink_node_t *ptNode = NULL;
    dlink_node_t *ptNextNode = NULL;

    if ((NULL == ptList) || (NULL == fnHook)) {
        return;
    }

    ptNode = ptList->ptHead;

    for (int32_t i = 0; i < ptList->nCount; i++, ptNode = ptNextNode) {
        ptNextNode = ptNode->ptNext;
        if (0 != fnHook(ptNode->pData, pData)) {
            continue;
        }

        if (1 == ptList->nCount) {
            ptList->ptHead = NULL;
        } else {
            ptNode->ptPre->ptNext = ptNextNode;
            ptNextNode->ptPre = ptNode->ptPre;
            if (0 == i) {
                ptList->ptHead = ptNextNode;
            }
        }

        if (NULL != fnDestroy) {
            (*fnDestroy)(ptNode->pData);
        }

        dlink_node_free(ptNode);
        i--;
        ptList->nCount--;
    }

}
