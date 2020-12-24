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
*       mailbox.h *                                                  *
*                                                                    *
**********************************************************************
*/
#ifndef MAILBOX_H
#define MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\service\event\event.h"

#define INIT_MAIL(_MAIL)                        init_mailbox(_MAIL)
#define OPEN_MAIL(_MAIL)                        open_mailbox(_MAIL)
#define POST_MAIL(_MAIL, _TARGET)               post_mailbox(_MAIL, _TARGET)

typedef struct {
    event_t tSealed;
    void *pTarget;
} mailbox_t;

extern bool init_mailbox(mailbox_t *ptMailbox);
extern bool post_mailbox(mailbox_t *ptMailbox, void *pTarget);
extern void* open_mailbox(mailbox_t *ptMailbox);

#ifdef __cplusplus
    }
#endif
#endif

/*************************** End of file ****************************/
