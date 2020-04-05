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
*       Flash * FLASH Write                                          *
*                                                                    *
**********************************************************************
*/
#include "iap_config.h"
#include "config.h"
#include "log.h"

#include "stm32f10x.h"

// 大容量系列
#if (FLASH_PAGE_SIZE == 0x0800)
    #define SECTOR_MASK                 (0xFFFFF800)
#else
    #define SECTOR_MASK                 (0xFFFFFC00)
#endif

#define GET_SECTOR_START_ADDR(_ADDR)           (SECTOR_MASK & _ADDR)        // 获取一个扇区的起始地址

static uint8_t write_cpu_flash(uint32_t wFlashAddr, uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t i = 0;
    uint32_t wTemp = 0;
    uint8_t chOriginLocked = 0;
    FLASH_Status tStatus = FLASH_COMPLETE;

    if (FLASH->CR & 0x00000080) {           // 判断FLASH是否被锁住
        FLASH_Unlock();
        chOriginLocked = 1;                 // 便于后面判断是否要重新锁住
    }
        
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (i = 0; i < (wSize >> 2); i++) {
        if (0 == (wFlashAddr & (FLASH_PAGE_SIZE - 1))) {
            tStatus = FLASH_ErasePage(GET_SECTOR_START_ADDR(wFlashAddr));   // 页擦除
            if (tStatus != FLASH_COMPLETE) {
                LOG_ERROR("erase fail");
                goto fail;
            }
        }
        
        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        wTemp |= pchBuf[(i << 2) + 2] << 16;
        wTemp |= pchBuf[(i << 2) + 3] << 24;

        tStatus = FLASH_ProgramWord(wFlashAddr, wTemp);
        if (!(tStatus == FLASH_COMPLETE && cup32(wFlashAddr) == wTemp)) {
            LOG_ERROR("program fail");
            goto fail;
        }

        wFlashAddr += 4;
    }

    if (wSize & 0x02) {
        
        if (0 == (wFlashAddr & (FLASH_PAGE_SIZE - 1))) {
            tStatus = FLASH_ErasePage(GET_SECTOR_START_ADDR(wFlashAddr));   // 页擦除
            if (tStatus != FLASH_COMPLETE) {
                LOG_ERROR("erase fail");
                goto fail;
            }
        }
        
        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        tStatus = FLASH_ProgramHalfWord(wFlashAddr, wTemp);
        if (!(tStatus == FLASH_COMPLETE && cup16(wFlashAddr) == wTemp)) {
            LOG_ERROR("program fail");
            goto fail;
        }
    }

    if (1 == chOriginLocked) {
        FLASH_Lock();
    }
    return 1;
    
fail:
    if (1 == chOriginLocked) {
        FLASH_Lock();
    }
    return 0;
}

/**
 * \brief FLASH写入函数
 *
 * \param wFlashAddr    32位写入地址
 * \param pchBuf        字节数组指针
 * \param wSize         字节数组长度
 *
 */
uint8_t flash_write(uint32_t wFlashAddr, uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t wAddress = wFlashAddr;
    uint16_t hwByteToWrite = 0;
    uint8_t *pchSrc = pchBuf;
    uint8_t chReturn = 0;
    uint32_t wPrimaskStatus = 0;
    
    if ((wFlashAddr < FLASH_BASE_ADDRESS) ||
        (wFlashAddr + wSize) > (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)) {
        return 0;
    }
    
    // 写入地址2字节对齐，写入字节个数为2的倍数
    if ((wFlashAddr & 0x01) || (wSize & 0x01)) {    // FALSH半字写入
        return 0;
    }

    wPrimaskStatus = __get_PRIMASK();               // 获取中断状态
    __set_PRIMASK(1);                               // 关闭总中断

    while (wSize) {
        hwByteToWrite = wSize >= FLASH_PAGE_SIZE ? FLASH_PAGE_SIZE : wSize;
        chReturn = write_cpu_flash(wAddress, pchSrc, hwByteToWrite);
        if (0 == chReturn) {
            break;
        }
        wAddress += hwByteToWrite;
        pchSrc += hwByteToWrite;
        wSize -= hwByteToWrite;
    }
    __set_PRIMASK(wPrimaskStatus);                 // 避免误开中断

    return chReturn;
}

/*************************** End of file ****************************/
