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
*       mailbox.c *                                                  *
*                                                                    *
**********************************************************************
*/
#include "mailbox.h"

bool init_mailbox(mailbox_t *ptMailbox)
{
    if (NULL != ptMailbox) {
        if (INIT_EVENT(&ptMailbox->tSealed, false, RESET_AUTO)) {
            ptMailbox->pTarget = NULL;
            return true;
        }
    }
    return false;
}

bool post_mailbox(mailbox_t *ptMailbox, void *pTarget)
{
    if (NULL != ptMailbox) {
        if (NULL != pTarget) {
            ptMailbox->pTarget = pTarget;
            return SET_EVENT(&ptMailbox->tSealed);
        }
    }
    return false;
}

void* open_mailbox(mailbox_t *ptMailbox)
{
    if (NULL != ptMailbox) {
        if (WAIT_EVENT(&ptMailbox->tSealed)) {
            return ptMailbox->pTarget;
        }
    }
    return NULL;
}

/*************************** End of file ****************************/
