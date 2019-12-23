#ifndef __CRITICAL_H__
#define __CRITICAL_H__

#include "..\service_cfg.h"

#define ENTER_CRITICAL_SECTOR(__CRITICAL)   \
                    (enter_critical_sector(__CRITICAL))
                    
#define LEAVE_CRITICAL_SECTOR(__CRITICAL)   \
                    do{leave_critical_sector(__CRITICAL);}while(0)
                    
#define INIT_CRITICAL_SECTOR(__CRITICAL)    \
                    (init_critical_sector(__CRITICAL))

typedef struct {
    bool bLocked;
} critical_sector_t;
typedef critical_sector_t mutex_t;

extern bool init_critical_sector(mutex_t *ptMutex);

extern bool enter_critical_sector(mutex_t *ptMutex);

extern bool leave_critical_sector(mutex_t *ptMutex);

#endif
