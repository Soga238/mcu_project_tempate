#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "stdint.h"

extern void iwdg_config(uint8_t prv, uint16_t rlv);
extern void iwdg_feed(void);

#endif
