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
        // 调试时中断程序容易导致eeprom死机
        i2c_crash_release();
        i2c_stop(); // 失败后发送I2C总线停止信号
        return 0;
    }
}

uint8_t ee_read_bytes(uint8_t *pReadBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t i;

    // 随机读取若干字节

    // 1、发生I2C开始信号
    i2c_start();

    // 2、发送设备地址 此处为写指令
    i2c_send_byte(EE_DEV_ADDR | I2C_WR);

    // 3、等待从设备应答
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    if (EE_ADDR_BYTES == 1) {
        // 4、发送读取地址 AT24C02只有256个字节,只需要一个字节就够了
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

    // 5、重新发生I2C开始信号
    i2c_start();

    // 6、发送设备地址 此处为读指令
    i2c_send_byte(EE_DEV_ADDR | I2C_RD);

    // 7、等待从设备应答
    if (i2c_wait_ack() != 0) {
        goto cmd_fail;
    }

    // 8、循环读取数据
    for (i = 0; i < usSize; i++) {
        *(pReadBuf + i) = i2c_read_byte();
        // 每读完一个发送一个ACK 最后一个字节NACK
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

 //写一页函数
uint8_t ee_write_bytes(const uint8_t *pWriteBuf, uint16_t usAddress, uint16_t usSize)
{
    uint16_t i;
    uint16_t m;
    uint16_t usAddr = usAddress;
    /*
        写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
        对于24xx02，page size = 8
        简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
        为了提高连续写的效率: 本函数采用page wirte操作。
    */

    for (i = 0; i < usSize; i++) {
        /* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
        if ((i == 0) || ((usAddr & (EE_PAGE_SIZE - 1)) == 0)) {
            /*　第０步：发停止信号，启动内部写操作　*/
            i2c_stop();

            /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
                CLK频率为200KHz时，查询次数为30次左右
            */
            for (m = 0; m < 1000; m++) {
                /* 第1步：发起I2C总线启动信号 */
                i2c_start();

                /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
                i2c_send_byte(EE_DEV_ADDR | I2C_WR); /* 此处是写指令 */

                /* 第3步：发送一个时钟，判断器件是否正确应答 */
                if (i2c_wait_ack() == 0) {
                    break;
                }
            }
            if (m  == 1000) {
                goto cmd_fail;  /* EEPROM器件写超时 */
            }

            /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
            if (EE_ADDR_BYTES == 1) {
                i2c_send_byte((uint8_t)usAddr);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM器件无应答 */
                }
            } else {
                i2c_send_byte(usAddr >> 8);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM器件无应答 */
                }

                i2c_send_byte(usAddr);
                if (i2c_wait_ack() != 0) {
                    goto cmd_fail;  /* EEPROM器件无应答 */
                }
            }
        }

        /* 第6步：开始写入数据 */
        i2c_send_byte(pWriteBuf[i]);

        /* 第7步：发送ACK */
        if (i2c_wait_ack() != 0) {
            goto cmd_fail;  /* EEPROM器件无应答 */
        }

        usAddr++;   /* 地址增1 */
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
