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

#ifndef __RINGBUF_H_
#define __RINGBUF_H_

#include "..\service_cfg.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef  struct char_ring_t char_ring_t;
struct char_ring_t {
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint8_t *pchBuf;
};

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
extern uint8_t  ringbuf_init(char_ring_t *ptRing, uint8_t *pchBuf, uint32_t wSize);
extern uint16_t ringbuf_put(char_ring_t *ptRing, uint8_t *pchSrc, uint16_t wSize);
extern uint16_t ringbuf_get(char_ring_t *ptRing, uint8_t *pchDst, uint16_t wReadSize);
extern uint16_t ringbuf_get_len(char_ring_t *ptRing);
extern uint16_t ringbuf_get_empty_len(char_ring_t *ptRing);
extern uint8_t ringbuf_get_one_char(char_ring_t *ptRing, uint8_t *pchByte);

#endif

/*************************** End of file ****************************/
