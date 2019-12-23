#ifndef EEPROM_24XX_H
#define EEPROM_24XX_H

#include "stdint.h"

extern uint8_t ee_check_ok(void);
extern uint8_t ee_read_bytes(uint8_t *pReadBuf, uint16_t usAddress, uint16_t usSize);
extern uint8_t ee_write_bytes(const uint8_t *pWriteBuf, uint16_t usAddress, uint16_t usSize);
#endif
