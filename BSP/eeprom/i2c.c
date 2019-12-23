#include "i2c.h"
#include "stm32f10x.h"
#include "bsp_gpio.h"

#define I2C_SCL_1()           EEP_SCL_HIGH()
#define I2C_SCL_0()           EEP_SCL_LOW()

#define I2C_SDA_1()           EEP_SDA_HIGH()
#define I2C_SDA_0()           EEP_SDA_LOW()

#define I2C_SDA_READ()        EEP_SDA_READ()
#define I2C_SCL_READ()        EEP_SCL_READ()

static void i2c_delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 30; i++) {
        // do nothing
    }
}

void i2c_start(void)
{
    /* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź� */
    I2C_SDA_1();
    I2C_SCL_1();
    i2c_delay();
    I2C_SDA_0();
    i2c_delay();

    I2C_SCL_0();
    i2c_delay();
}

void i2c_stop(void)
{
    /* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź� */
    I2C_SDA_0();
    I2C_SCL_1();
    i2c_delay();

    I2C_SDA_1();
    i2c_delay();
}

void i2c_send_byte(uint8_t ucByte)
{
    uint8_t i;

    // i2c_start���ʱSCL���ڵ͵�ƽ
    for (i = 0; i < 8; i++) {
        if (ucByte & (0x80 >> i)) {
            I2C_SDA_1();
        } else {
            I2C_SDA_0();
        }
        i2c_delay();
        I2C_SCL_1();
        i2c_delay();
        I2C_SCL_0();
    }

    i2c_delay();
    I2C_SDA_1();    // �ͷ�����
    i2c_delay();
}


uint8_t i2c_read_byte(void)
{
    uint8_t i;
    uint8_t value = 0;

    for (i = 0; i < 8; i++) {
        value <<= 1;
        I2C_SCL_1();    // �ͷ�����
        i2c_delay();

        if (I2C_SDA_READ()) {
            value += 1;
        }
        I2C_SCL_0();
        i2c_delay();
    }
    return value;
}

uint8_t i2c_wait_ack(void)
{
    uint8_t rc = 0;
    // �ȴ��ӻ�����SDA ������ʱSDA�����ͣ���9������SCL��SDA�ָ��ߵ�ƽ
    I2C_SDA_1();
    i2c_delay();
    I2C_SCL_1();
    i2c_delay();

    if (I2C_SDA_READ()) {
        rc = 1; // �豸����Ӧ
    }

    I2C_SCL_0();
    i2c_delay();

    return rc;
}

void i2c_ack(void)
{
    // �ڵ�9��ʱ�����壬����SDA
    I2C_SDA_0();
    i2c_delay();
    I2C_SCL_1();
    i2c_delay();
    I2C_SCL_0();
    i2c_delay();
    I2C_SDA_1();
    i2c_delay();
}

void i2c_nack(void)
{
    // �ڵ�9��ʱ�����壬SDA���ָߵ�ƽ
    I2C_SDA_1();
    i2c_delay();
    I2C_SCL_1();
    i2c_delay();
    I2C_SCL_0();
    i2c_delay();
}

uint8_t i2c_check_device(uint8_t addr)
{
    uint8_t ucAck = 0;
    if (I2C_SCL_READ() && I2C_SDA_READ()) {
        i2c_start();
        i2c_send_byte(addr | I2C_WR);
        ucAck = i2c_wait_ack();
        i2c_stop();
        return ucAck;   //����0��ʾ����Ӧ
    }
    return 1;
}

uint8_t i2c_crash_release(void)
{
    uint8_t i = 0;

    if (I2C_SDA_READ()) {
        return 1;
    }

    for (i = 0; i < 9; i++) {
        I2C_SCL_0();
        if (I2C_SDA_READ()) {
            return 1;
        } else {
            I2C_SCL_1();
        }
    }
    return 0;
}

