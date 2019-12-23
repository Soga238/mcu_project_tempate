#include "eeprom_24xx.h"
#include "eeprom_cfg.h"
#include "i2c.h"

#ifndef MIN
    #define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

uint8_t ee_check_ok(void)
{
    if (i2c_check_device(EE_DEV_ADDR) == 0) {
        return 1;
    } else {
        // ����ʱ�жϳ������׵���eeprom����
        i2c_crash_release();
        i2c_stop(); // ʧ�ܺ���I2C����ֹͣ�ź�
        return 0;
    }
}

uint8_t ee_read_bytes(uint8_t *pReadBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t i;

    // �����ȡ�����ֽ�

    // 1������I2C��ʼ�ź�
    i2c_start();

    // 2�������豸��ַ �˴�Ϊдָ��
    i2c_send_byte(EE_DEV_ADDR | I2C_WR);

    // 3���ȴ����豸Ӧ��
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    if (EE_ADDR_BYTES == 1) {
        // 4�����Ͷ�ȡ��ַ AT24C02ֻ��256���ֽ�,ֻ��Ҫһ���ֽھ͹���
        i2c_send_byte((uint8_t)usAddress);
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;
        }
    } else if(EE_ADDR_BYTES == 2) {
        i2c_send_byte(usAddress >> 8);
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;
        }

        i2c_send_byte(usAddress);
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;
        }
    }

    // 5�����·���I2C��ʼ�ź�
    i2c_start();

    // 6�������豸��ַ �˴�Ϊ��ָ��
    i2c_send_byte(EE_DEV_ADDR | I2C_RD);

    // 7���ȴ����豸Ӧ��
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    // 8��ѭ����ȡ����
    for (i = 0; i < usSize; i++) {
        *(pReadBuf + i) = i2c_read_byte();
        // ÿ����һ������һ��ACK ���һ���ֽ�NACK
        if (i != usSize - 1) {
            i2c_ack();
        } else {
            i2c_nack();
        }
    }
    i2c_stop();
    return 1;

cmd_fail:
    i2c_stop();
    return 0;
}

 //дһҳ����
uint8_t ee_write_bytes(const uint8_t *pWriteBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t i;
    uint16_t m;
    uint16_t usAddr = usAddress;
    /*
        д����EEPROM�������������������ȡ�ܶ��ֽڣ�ÿ��д����ֻ����ͬһ��page��
        ����24xx02��page size = 8
        �򵥵Ĵ�����Ϊ�����ֽ�д����ģʽ��ÿд1���ֽڣ������͵�ַ
        Ϊ���������д��Ч��: ����������page wirte������
    */

    for (i = 0; i < usSize; i++) {
        /* �����͵�1���ֽڻ���ҳ���׵�ַʱ����Ҫ���·��������źź͵�ַ */
        if ((i == 0) || ((usAddr & (EE_PAGE_SIZE - 1)) == 0)) {
            /*���ڣ�������ֹͣ�źţ������ڲ�д������*/
            i2c_stop();

            /* ͨ���������Ӧ��ķ�ʽ���ж��ڲ�д�����Ƿ����, һ��С�� 10ms
                CLKƵ��Ϊ200KHzʱ����ѯ����Ϊ30������
            */
            for (m = 0; m < 1000; m++) {
                /* ��1��������I2C���������ź� */
                i2c_start();

                /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
                i2c_send_byte(EE_DEV_ADDR | I2C_WR); /* �˴���дָ�� */

                /* ��3��������һ��ʱ�ӣ��ж������Ƿ���ȷӦ�� */
                if (i2c_wait_ack() == 0) {
                    break;
                }
            }
            if (m  == 1000) {
                goto cmd_fail;  /* EEPROM����д��ʱ */
            }

            /* ��4���������ֽڵ�ַ��24C02ֻ��256�ֽڣ����1���ֽھ͹��ˣ������24C04���ϣ���ô�˴���Ҫ���������ַ */
            if (EE_ADDR_BYTES == 1) {
                i2c_send_byte((uint8_t)usAddr);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM������Ӧ�� */
                }
            } else {
                i2c_send_byte(usAddr >> 8);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM������Ӧ�� */
                }

                i2c_send_byte(usAddr);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM������Ӧ�� */
                }
            }
        }

        /* ��6������ʼд������ */
        i2c_send_byte(pWriteBuf[i]);

        /* ��7��������ACK */
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;  /* EEPROM������Ӧ�� */
        }

        usAddr++;   /* ��ַ��1 */
    }

    i2c_stop();
    return 1;
cmd_fail:
    i2c_stop();
    return 0;

}

/*
uint8_t ee_read_bytes(uint8_t *pReadBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t i;
    uint16_t hwWordAddress = usAddress & 0x00FF;
    uint16_t hwDeviceAddress = EE_DEV_ADDR + ((usAddress >> 8) << 1);

    i2c_start();
    i2c_send_byte(hwDeviceAddress | I2C_WR);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

#if EE_ADDR_BYTES == 1
    i2c_send_byte((uint8_t)hwWordAddress);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }
#else
    i2c_send_byte((uint8_t)(hwWordAddress >> 8));
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    i2c_send_byte((uint8_t)hwWordAddress);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }
#endif

    i2c_start();
    i2c_send_byte(hwDeviceAddress | I2C_RD);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    for (i = 0; i < usSize; i++) {
        *(pReadBuf + i) = i2c_read_byte();
        if (i != usSize - 1) {
            i2c_ack();
        } else {
            i2c_nack();
        }
    }
    i2c_stop();
    return 1;

cmd_fail:
    i2c_stop();
    return 0;
}

static uint8_t ee_write_one_page_bytes(const uint8_t *pWriteBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t m;
    uint16_t usAddr = usAddress;
    uint16_t DeviceAddress = EE_DEV_ADDR;

#if EE_ADDR_BYTES == 1
    uint16_t Page= 0;
    uint16_t hwWordAddress = usAddress;
    
    Page = usAddr / EE_PAGE_SIZE;
    hwWordAddress=(usAddr % EE_PAGE_SIZE) & 0x0F;
    DeviceAddress |= (((Page<<1) & 0xE0)>>4);     //High 3 bits
    hwWordAddress |= (Page & 0x0F) << 4;            //Low 4 bits
#endif

    i2c_stop();
    for (m = 0; m < 1000; m++) {
        i2c_start();
        i2c_send_byte(DeviceAddress);
        if (i2c_wait_ack() == 0) {
            break;
        }
    }

    if (m  == 1000) {
        goto cmd_fail;
    }

#if EE_ADDR_BYTES == 1
    i2c_send_byte(hwWordAddress);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }    
#else 
    i2c_send_byte((uint8_t)(usAddr >> 8));
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }
    
    i2c_send_byte((uint8_t)usAddr);
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    } 
#endif

    for (m = 0; m < usSize; m++) {
        i2c_send_byte(pWriteBuf[m]);
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;
        }
    }

    i2c_stop();
    return 1;
cmd_fail:
    i2c_stop();
    return 0;
}

*/

//static uint8_t ee_check_ok_retry(uint8_t chRetry)
//{
//    uint8_t chReturn = 0;

//    while (chRetry) {
//        if (0 == i2c_check_device(EE_DEV_ADDR)) {
//            chReturn = 1;
//            break;
//        }
//        chRetry -= 1;
//    }

//    return chReturn;
//}

//uint8_t ee_write_bytes(const uint8_t* pchBuf, uint16_t hwAddress, uint16_t hwWriteBytes)
//{
//    uint8_t chWriteToLength, chPageOffest;
//    uint8_t chError = 1;

//    while(hwWriteBytes > 0) {
//        chPageOffest = EE_PAGE_SIZE - (hwAddress & (EE_PAGE_SIZE - 1));
//        chWriteToLength = hwWriteBytes > chPageOffest ? chPageOffest : hwWriteBytes;

//        if (0 == ee_write_one_page_bytes(pchBuf, hwAddress, chWriteToLength)) {
//            chError = 0;
//            break;
//        }

//        hwWriteBytes = hwWriteBytes - chWriteToLength;
//        if(hwWriteBytes > 0) {
//            pchBuf = pchBuf + chWriteToLength;
//            hwAddress = hwAddress + chWriteToLength;

//            if (0 == ee_check_ok_retry(16)) {
//                chError = 0;
//                break;
//            }
//        }
//    }
//    return chError;
//}
