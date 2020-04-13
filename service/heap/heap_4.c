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
*       Heap * Heap_1.c                                              *
*                                                                    *
**********************************************************************
*/
#include ".\heap.h"
#include <stdio.h>

#define HEAP_BITS_PER_BYTE      ((uint32_t)8)
#define HEAP_MINIMUM_BLOCK_SIZE (c_hwHeapStructSize << 1)

typedef struct a_block_link {
    struct a_block_link     *pNextFreeBlock;    // the next free block in the list.
    uint32_t                 wBlockSize;        // the size of the free block.
} block_lint_t;

static const uint16_t c_hwHeapStructSize = ((sizeof(block_lint_t) + (PORT_BYTE_ALIGNMENT - 1)) & ~PORT_BYTE_ALIGNMENT_MASK);

/*! 在分散加载文件定义ram空间给heap使用 */
static uint8_t g_chHeap[CONFIG_TOTAL_HEAP_SIZE] AT_ADDR(0x20009800);

static block_lint_t g_tBlockStart, *g_ptBlockEnd;

uint32_t g_wFreeBytesRemaining = CONFIG_TOTAL_HEAP_SIZE;
static uint32_t g_wMinimunEverFreeBytesRemaining = 0;

static uint32_t g_wBlockAllocatedBit = 0;
static void heap_init(void)
{
    block_lint_t *ptFirstFreeBlock = NULL;
    uint8_t *pchAlignedHeap = NULL;
    uint32_t wAddress = 0;
    uint32_t wTotalHeapSize = CONFIG_TOTAL_HEAP_SIZE;

    //Ensure the heap starts on a correctly aligned boundary.
    wAddress = (uint32_t)g_chHeap;
    if (wAddress & PORT_BYTE_ALIGNMENT_MASK) {
        wAddress = (wAddress + PORT_BYTE_ALIGNMENT - 1) & (~PORT_BYTE_ALIGNMENT_MASK);
        wTotalHeapSize -= (wAddress - (uint32_t)g_chHeap);
    }

    pchAlignedHeap = (uint8_t *)wAddress;
    // g_tBlockStart  is used to hold a pointer to the first item
    // in the list of free blocks.
    // The void cast is used to prevent compiler warnings.
    g_tBlockStart.pNextFreeBlock = (void *)pchAlignedHeap;
    g_tBlockStart.wBlockSize = 0;

    // g_ptBlockEnd is used to mark the end of the list of free blocks
    // and is inserted at;[;; the end of the heap space.
    wAddress = (uint32_t)pchAlignedHeap + wTotalHeapSize;
    wAddress -= c_hwHeapStructSize;
    wAddress &= ~((uint32_t)PORT_BYTE_ALIGNMENT_MASK);

    g_ptBlockEnd = (void *)wAddress;
    g_ptBlockEnd->pNextFreeBlock = NULL;
    g_ptBlockEnd->wBlockSize = 0;

    // to start with there is a single free block that is sized to
    // take up the entire heap space, minus the space taken by g_tBlockEnd.
    ptFirstFreeBlock = (void *)pchAlignedHeap;
    ptFirstFreeBlock->wBlockSize = wAddress - (uint32_t)ptFirstFreeBlock;
    ptFirstFreeBlock->pNextFreeBlock = g_ptBlockEnd;

    // only one block exists - and it covers the entire usable heap space.
    g_wFreeBytesRemaining = ptFirstFreeBlock->wBlockSize;
    g_wMinimunEverFreeBytesRemaining = ptFirstFreeBlock->wBlockSize;

    // work out the position of the top bit ia size_t variable.
    g_wBlockAllocatedBit = ((uint32_t)1) << (sizeof(uint32_t) * HEAP_BITS_PER_BYTE - 1);
    //g_wBlockAllocatedBit = (0x01) << (sizeof(uint32_t) * HEAP_BITS_PER_BYTE - 1);
}


static void insert_block_into_free_list(block_lint_t *ptBlockToInsert)
{
    block_lint_t *ptIterator;
    uint8_t *pch;

    // iterate through the list until a block is found that has
    // a higher address than the block being inserted.
    for (ptIterator = &g_tBlockStart; ptIterator->pNextFreeBlock < ptBlockToInsert;
         ptIterator = ptIterator->pNextFreeBlock) {
        // nothing to do here, just iterate to the right position.
    }

    // do the block being inserted, and the block it is being inserted
    // after make a contiguous block of memory?

    pch = (uint8_t *)ptIterator;
    if ((pch + ptIterator->wBlockSize) == (uint8_t *)ptBlockToInsert) {
        ptIterator->wBlockSize += ptBlockToInsert->wBlockSize;
        ptBlockToInsert = ptIterator;
    }

    // do the block being inserted, and the block it is being inserted
    // before make a contiguous block of memory?

    pch = (uint8_t *)ptBlockToInsert;
    if ((pch + ptBlockToInsert->wBlockSize) == (uint8_t *) ptIterator->pNextFreeBlock) {
        if (ptIterator->pNextFreeBlock != g_ptBlockEnd) {
            // form one big block from the two blocks.
            ptBlockToInsert->wBlockSize += ptIterator->pNextFreeBlock->wBlockSize;
            ptBlockToInsert->pNextFreeBlock = ptIterator->pNextFreeBlock->pNextFreeBlock;
        } else {
            ptBlockToInsert->pNextFreeBlock = g_ptBlockEnd;
        }
    } else {
        ptBlockToInsert->pNextFreeBlock = ptIterator->pNextFreeBlock;
    }

    // if the block being inserted plugged a gap, so was merged with block
    // before and the block after, then its pNextFreeBlock pointer will have
    // already been set, and should not be set here as that would make it
    // point to itself.

    if (ptIterator != ptBlockToInsert) {
        ptIterator->pNextFreeBlock = ptBlockToInsert;
    }
}


void *port_malloc_4(uint32_t wSize)
{
    block_lint_t *ptBlock, *ptPreBlock, *ptNewBlockLink;
    void *pReturn = NULL;

    // HEAP_LOG_RAW("free: %d, malloc: %d.", port_get_free_heap_size_4(), wSize);
    // -----------------------------------------------------------------
    //                          secdule protect
    // -----------------------------------------------------------------
    ENTER_SECDULE_PROTECT();

    if (NULL == g_ptBlockEnd) {
        heap_init();
    }

    // check the requested block size is not so large that the top bit
    // is set. the top bit of the block size member of the block_link_t
    // structure is used to determine who owns the block - the application
    // or kernel, so it must be free.
    if (wSize & g_wBlockAllocatedBit) {
        return pReturn;
    }

    // the wanted size is increased so it can contain a block_list_t structure
    // in addition to the requested amount of bytes.
    if (wSize) {
        wSize += c_hwHeapStructSize;

        // ensure that blocks are always aligned to the required number
        // of bytes.
        if (wSize & PORT_BYTE_ALIGNMENT_MASK) {
            // byte alignment is required.
            wSize += (PORT_BYTE_ALIGNMENT - (wSize & PORT_BYTE_ALIGNMENT_MASK));
        }
    }

    if (wSize && wSize <= g_wFreeBytesRemaining) {

        // traverse the list from the start (lowest address) block until
        // one of adequate size is found.
        ptPreBlock = &g_tBlockStart;
        ptBlock = g_tBlockStart.pNextFreeBlock;
        while ((ptBlock->wBlockSize < wSize) && (NULL != ptBlock->pNextFreeBlock)) {
            ptPreBlock = ptBlock;
            ptBlock = ptBlock->pNextFreeBlock;
        }

        // if the end marker was reached than adequate suze was not found.
        if (ptBlock != g_ptBlockEnd) {
            pReturn = (void *)((uint8_t *)ptPreBlock->pNextFreeBlock + c_hwHeapStructSize);

            // this block is being returned for use so must be taken out
            // of the list of free blocks.
            ptPreBlock->pNextFreeBlock = ptBlock->pNextFreeBlock;

            // if the block is larger than required it can be split into
            // two.
            if (wSize + HEAP_MINIMUM_BLOCK_SIZE < ptBlock->wBlockSize) {
                // this block is to be split into two. create a new block
                // following the number of bytes requested. thw void cast
                // is used to prevent byte alignment warning from the
                // compiler.

                ptNewBlockLink = (void *)((uint8_t *)ptBlock + wSize);

                // calcualte the sizes of two blocks split from the sigle
                // block.
                ptNewBlockLink->wBlockSize = ptBlock->wBlockSize - wSize;
                ptBlock->wBlockSize = wSize;

                // insert into the new block into the list of free blocks.
                insert_block_into_free_list(ptNewBlockLink);
            }

            g_wFreeBytesRemaining -= ptBlock->wBlockSize;
            if (g_wFreeBytesRemaining < g_wMinimunEverFreeBytesRemaining) {
                g_wMinimunEverFreeBytesRemaining = g_wFreeBytesRemaining;
            }

            // the block is being returned - it is allocated and own by the
            // aplliction and has no "next" block.
            ptBlock->wBlockSize |= g_wBlockAllocatedBit;
            ptBlock->pNextFreeBlock = NULL;
        }
    }

    EXIT_SECDULE_PROTECT();
    // -----------------------------------------------------------------
    //HEAP_LOG_RAW(" ptr: 0x%08x\r\n", pReturn);
    if (NULL == pReturn){
        HEAP_LOG_RAW("no memory to malloc.\r\n");
    }
    
    return pReturn;
}


void port_free_4(void *pMemory)
{
    uint8_t *pMem = (uint8_t *)pMemory;
    block_lint_t *ptLink = NULL;

    if (NULL == pMem) {
        return ;
    }

    pMem -= c_hwHeapStructSize;
    ptLink = (void *)pMem;
    if (ptLink->wBlockSize & g_wBlockAllocatedBit) {
        ENTER_SECDULE_PROTECT();
        if (NULL == ptLink->pNextFreeBlock) {
            ptLink->wBlockSize &= ~g_wBlockAllocatedBit;
            g_wFreeBytesRemaining += ptLink->wBlockSize;
            insert_block_into_free_list(ptLink);
        }
        EXIT_SECDULE_PROTECT();
    }
}

uint32_t port_get_free_heap_size_4(void)
{
    return g_wFreeBytesRemaining;
}

/*
void heap4_test(void)
{
    uint8_t *pMemBlock = NULL;
    uint8_t *pMemBlock1 = NULL;
    uint8_t *pMemBlock2 = NULL;
    uint8_t *pMemBlock3 = NULL;
    uint8_t *pMemBlock4 = NULL;

    printf("start address of g_chHeap is 0x%08x\r\n", (uint32_t)g_chHeap);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    pMemBlock1 = (uint8_t *)port_malloc_4(10);
    printf("malloc: 10, m_1: 0x%08x\r\n", (uint32_t)pMemBlock1);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    pMemBlock2 = (uint8_t *)port_malloc_4(45);
    printf("malloc: 45, m_2: 0x%08x\r\n", (uint32_t)pMemBlock2);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    pMemBlock3 = (uint8_t *)port_malloc_4(10);
    printf("malloc: 10, m_3: 0x%08x\r\n", (uint32_t)pMemBlock3);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    pMemBlock4 = (uint8_t *)port_malloc_4(20);
    printf("malloc: 20, m_3: 0x%08x\r\n", (uint32_t)pMemBlock4);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    printf("\r\n");

    printf("free heap 0x%08x\r\n", (uint32_t)pMemBlock2);
    port_free_4(pMemBlock2);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());

    pMemBlock = (uint8_t *)port_malloc_4(1);
    printf("malloc: 1, m_3: 0x%08x\r\n", (uint32_t)pMemBlock);
    printf("free bytes: %d\r\n", port_get_free_heap_size_4());
}
*/
