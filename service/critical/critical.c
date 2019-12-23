#include ".\critical.h"

bool init_critical_sector(mutex_t *ptMutex)
{
    if (NULL == ptMutex) {
        return false;
    }

    ptMutex->bLocked = false;
    return true;
}

/*! \brief can set true if bLocked is false
 *! \return fsm_rt_t
 */
bool enter_critical_sector(mutex_t *ptMutex)
{    
    if (NULL == ptMutex) {
        return false;
    }
    
    if (ptMutex->bLocked){
        return false;
    }

    ptMutex->bLocked = true;
    return true;
}

bool leave_critical_sector(mutex_t *ptMutex)
{
    if (NULL == ptMutex) {
        return false;
    }

    ptMutex->bLocked = false;
    return true;
}
