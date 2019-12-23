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
*       event.h *                                                    *
*                                                                    *
**********************************************************************
*/
#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "..\service_cfg.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define RESET_AUTO     false
#define RESET_MANUAL   true
    
#define SET_EVENT(__EVENT)      set_event(__EVENT)
#define WAIT_EVENT(_EVENT)      wait_event(_EVENT)
#define RESET_EVENT(_EVENT)     reset_event(_EVENT)

#define INIT_EVENT(_EVENT, _INIT_VALUE, _MANUAL)    \
                                init_event(_EVENT, _INIT_VALUE, _MANUAL)
                                    
/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
    bool bAutoReset;
    bool bIsSet;
} event_t;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern bool wait_event(event_t *ptEvent);
extern bool reset_event(event_t *ptEvent);
extern bool init_event(event_t *ptEvent, bool bValue, bool bManual);
extern bool set_event(event_t *ptEvent);

#ifdef __cplusplus
    }
#endif

#endif
/*************************** End of file ****************************/
