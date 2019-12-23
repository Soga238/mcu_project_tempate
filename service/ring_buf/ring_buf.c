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
*       RINGBUF * Ring Buffer Quueue                                 *
*                                                                    *
**********************************************************************
*/

#include "./ring_buf.h"
#include <stdlib.h>
#include <string.h>

/*********************************************************************
*
*       Configuration, default values
*
**********************************************************************
*/
#define IS_POWER_OF_2(size) ((size) & (size - 1))

/**
 * /brief Initialize the circular queue
 * /param ptRing Pointer to the queue control block.
 * /param pchBuf Pointer to character array.
 * /param wSize  Size of character array.
 * /return  1:  Success
 *          0:  Fail
 */
uint8_t ringbuf_init(char_ring_t *ptRing, uint8_t *pchBuf, uint32_t wSize)
{
    if (NULL == ptRing || NULL == pchBuf || 0 == wSize) {
        return 0;
    }

    if (IS_POWER_OF_2(wSize)) {
        return 0;
    }

    ptRing->head = 0;
    ptRing->tail = 0;
    ptRing->pchBuf = pchBuf;
    ptRing->size = wSize;

    return 1;
}

/**
 * /brief Put data to the circular queue
 * /param ptRing Pointer to the queue control block.
 * /param pchSrc Pointer to source character array.
 * /param wSize Number of character.
 * /return The number of characters actually insert into the queue.
 */
uint16_t ringbuf_put(char_ring_t *ptRing, uint8_t *pchSrc, uint16_t wSize)
{
    uint16_t wBytes = 0;
    uint16_t wTailToEnd = 0;
    uint16_t wLeftSpace = 0;

    if (NULL == ptRing || NULL == pchSrc || 0 == wSize) {
        return 0;
    }

    wTailToEnd = ptRing->size - ptRing->tail;

    // free one byte difference between empty and full.
    // the location where tail is located does not store data.
    // "tail == head" mean empty.

    wLeftSpace = (ptRing->head + wTailToEnd) & (ptRing->size - 1);
    wLeftSpace = wLeftSpace ? wLeftSpace - 1 : ptRing->size - 1;

    wBytes = MIN(wSize, wLeftSpace);

    if (ptRing->head > ptRing->tail) {
        memcpy(ptRing->pchBuf + ptRing->tail, pchSrc, wBytes);
    } else {
        if (wTailToEnd >= wBytes) {
            memcpy(ptRing->pchBuf + ptRing->tail, pchSrc, wBytes);
        } else {
            memcpy(ptRing->pchBuf + ptRing->tail, pchSrc, wTailToEnd);
            memcpy(ptRing->pchBuf, pchSrc + wTailToEnd, wBytes - wTailToEnd);
        }
    }
    ptRing->tail = (ptRing->tail + wBytes) & (ptRing->size - 1);

    return wBytes;
}

/**
 * /brief Read data from a circular queue
 * /param ptRing Pointer to the queue control block.
 * /param pchDst Pointer to destination character array.
 * /param wReadSize The number of character to read
 * /return The number of characters actually read from the queue.
 */
uint16_t ringbuf_get(char_ring_t *ptRing, uint8_t *pchDst, uint16_t wReadSize)
{
    uint16_t wBytes = 0;
    uint16_t wValidBytes = 0;
    uint16_t wTail = 0;
    uint16_t wHeadToEnd = 0;

    if (NULL == ptRing || NULL == pchDst || 0 == wReadSize) {
        return 0;
    }

    wValidBytes = (ptRing->tail - ptRing->head + ptRing->size) & (ptRing->size - 1);
    if (wValidBytes) {
        wBytes = MIN(wValidBytes, wReadSize);
        wTail = (ptRing->head + wBytes) & (ptRing->size - 1);

        if (wTail > ptRing->head) {
            memcpy(pchDst, ptRing->pchBuf + ptRing->head, wBytes);
        } else {
            wHeadToEnd = ptRing->size - ptRing->head;
            memcpy(pchDst, ptRing->pchBuf + ptRing->head, wHeadToEnd);
            memcpy(pchDst + wHeadToEnd, ptRing->pchBuf, wBytes - wHeadToEnd);
        }

        ptRing->head = (ptRing->head + wBytes) & (ptRing->size - 1);
    }

    return wBytes;
}

/**
 * /brief Get the length of characters in the queue.
 * /param ptRing Pointer to the queue control block.
 * /return
 */
uint16_t ringbuf_get_len(char_ring_t *ptRing)
{
    uint16_t wValidLen = 0;
    if (NULL == ptRing) {
        return 0;
    }
    
    wValidLen = (ptRing->tail - ptRing->head + ptRing->size) & (ptRing->size - 1);

    return wValidLen;
}

/**
 * /brief Get hte ramaning space in the queue.
 * /param ptRing Pointer to the queue control block.
 * /return
 */
uint16_t ringbuf_get_empty_len(char_ring_t *ptRing)
{
    uint16_t wLeftSpace = 0;

    if (NULL == ptRing) {
        return 0;
    }

    wLeftSpace = (ptRing->head + ptRing->size - ptRing->tail) & (ptRing->size - 1);
    wLeftSpace = wLeftSpace ? wLeftSpace - 1 : ptRing->size - 1;

    return wLeftSpace;
}

uint8_t ringbuf_get_one_char(char_ring_t *ptRing, uint8_t *pchByte)
{
    if (NULL == ptRing) {
        return 0;
    }

    if (ptRing->tail != ptRing->head) {
        *pchByte = ptRing->pchBuf[ptRing->head];
        ptRing->head = (ptRing->head + 1) & (ptRing->size - 1);
        return 1;
    }

    return 0;
}

/*************************** End of file ****************************/
