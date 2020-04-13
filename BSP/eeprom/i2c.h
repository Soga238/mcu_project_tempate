#ifndef I2C_H
#define I2C_H

#include "stdint.h"

#define I2C_WR              0       /* 写控制bit */
#define I2C_RD              1       /* 读控制bit */

extern void     i2c_start(void);
extern void     i2c_stop(void);
extern void     i2c_send_byte(uint8_t ucByte);
extern uint8_t  i2c_read_byte(void);

extern void     i2c_ack(void);
extern void     i2c_nack(void);
extern uint8_t  i2c_wait_ack(void);
extern uint8_t  i2c_check_device(uint8_t addr);
extern uint8_t  i2c_crash_release(void);
#endif
