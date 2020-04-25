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
#include ".\flashwrite.h"
#include "stm32f10x.h"

// 大容量系列
#if (FLASH_PAGE_SIZE == 0x0800)
#define SECTOR_MASK                 (0xFFFFF800)
#else
#define SECTOR_MASK                 (0xFFFFFC00)
#endif

#define GET_SECTOR_START_ADDR(__ADDR)   (SECTOR_MASK & (__ADDR))    // 获取一个扇区的起始地址

static int32_t write_cpu_flash(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t wTemp, i = 0;
    FLASH_Status tStatus = FLASH_COMPLETE;

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (i = 0; i < (wSize >> 2); i++, wFlashAddr += 4) {
        if (GET_SECTOR_START_ADDR(wFlashAddr) == wFlashAddr) {    // 遇到扇区首地址才擦除
            tStatus = FLASH_ErasePage(wFlashAddr);
            if (tStatus != FLASH_COMPLETE) {
                goto fail;
            }
        }

        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        wTemp |= pchBuf[(i << 2) + 2] << 16;
        wTemp |= pchBuf[(i << 2) + 3] << 24;
        tStatus = FLASH_ProgramWord(wFlashAddr, wTemp);
        if (!(tStatus == FLASH_COMPLETE && cup32(wFlashAddr) == wTemp)) {
            goto fail;
        }
    }

    if (wSize & 0x02) {
        if (GET_SECTOR_START_ADDR(wFlashAddr) == wFlashAddr) {    // 遇到扇区首地址才擦除
            tStatus = FLASH_ErasePage(wFlashAddr);
            if (tStatus != FLASH_COMPLETE) {
                goto fail;
            }
        }

        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        tStatus = FLASH_ProgramHalfWord(wFlashAddr, wTemp);
        if (!(tStatus == FLASH_COMPLETE && cup16(wFlashAddr) == wTemp)) {
            goto fail;
        }
    }

    return 1;
fail:
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
int32_t flash_write(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t wBytes, wPrimaskStatus, wAddress = wFlashAddr;
    const uint8_t *pchSrc = pchBuf;
    int32_t nReturn = 1;
    uint8_t chFlashLocked = 0;

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

    if (FLASH->CR & 0x00000080) {                   // 判断FLASH是否被锁住
        FLASH_Unlock();
        chFlashLocked = 1;                          // 便于后面判断是否要重新锁住
    }

    while (wSize) {
        wBytes = GET_SECTOR_START_ADDR(wAddress) + FLASH_PAGE_SIZE - wAddress;
        wBytes = wSize >= wBytes ? wBytes : wSize;
        if (write_cpu_flash(wAddress, pchSrc, wBytes)) {
            wAddress += wBytes;
            pchSrc += wBytes;
            wSize -= wBytes;
        } else {
            nReturn = 0;
            break;
        }
    }

    if (1 == chFlashLocked) {
        FLASH_Lock();
    }

    __set_PRIMASK(wPrimaskStatus);                 // 避免误开中断

    return nReturn;
}

/*************************** End of file ****************************/
