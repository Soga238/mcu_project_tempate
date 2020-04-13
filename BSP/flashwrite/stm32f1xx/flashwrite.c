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

#include ".\flashwrite_cfg.h"
#include ".\flashwrite.h"
#include "..\..\hal\mcu\stm32f103rx\Inc\stm32f1xx_hal_conf.h"

// 大容量系列
#if (FLASH_PAGE_SIZE == 0x0800)
    #define SECTOR_MASK                 (0xFFFFF800)
#else
    #define SECTOR_MASK                 (0xFFFFFC00)
#endif

#define GET_SECTOR_START_ADDR(_ADDR)           (SECTOR_MASK & _ADDR)        // 获取一个扇区的起始地址

static int32_t write_cpu_flash(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t i = 0;
    uint32_t wTemp = 0;
    uint32_t wPageError = 0;
    HAL_StatusTypeDef tStatus = HAL_OK;

    FLASH_EraseInitTypeDef tEraseInfo = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .NbPages = 1,
        .Banks = FLASH_BANK_1
    };

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    for (i = 0; i < (wSize >> 2); i++) {
        if (0 == (wFlashAddr & (FLASH_PAGE_SIZE - 1))) {
            tEraseInfo.PageAddress = GET_SECTOR_START_ADDR(wFlashAddr);
            if (HAL_OK != HAL_FLASHEx_Erase(&tEraseInfo, &wPageError)) {
                goto fail;
            }
        }

        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        wTemp |= pchBuf[(i << 2) + 2] << 16;
        wTemp |= pchBuf[(i << 2) + 3] << 24;

        tStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, wFlashAddr, wTemp);
        if (!(tStatus == HAL_OK && cup32(wFlashAddr) == wTemp)) {
            goto fail;
        }

        wFlashAddr += 4;
    }

    if (wSize & 0x02) {
        if (0 == (wFlashAddr & (FLASH_PAGE_SIZE - 1))) {
            tEraseInfo.PageAddress = GET_SECTOR_START_ADDR(wFlashAddr);
            if (HAL_OK != HAL_FLASHEx_Erase(&tEraseInfo, &wPageError)) {
                goto fail;
            }
        }

        wTemp = pchBuf[i << 2];
        wTemp |= pchBuf[(i << 2) + 1] << 8;
        tStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, wFlashAddr, wTemp);
        if (!(tStatus == HAL_OK && cup16(wFlashAddr) == wTemp)) {
            goto fail;
        }
    }

    return 1;
fail:
    return 0;
}

/**
 * \brief FLASH写入函数
 * \param wFlashAddr    32位写入地址
 * \param pchBuf        字节数组指针
 * \param wSize         字节数组长度
 *
 */
int32_t flash_write(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t wPrimaskStatus, wBytes, wAddress = wFlashAddr;
    const uint8_t *pchSrc = pchBuf;
    int32_t  nReturn = 1;
    uint8_t chOriginLocked = 0;

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
        HAL_FLASH_Unlock();
        chOriginLocked = 1;                         // 便于后面判断是否要重新锁住
    }

    while (wSize) {
        wBytes = wFlashAddr - GET_SECTOR_START_ADDR(wFlashAddr);
        wBytes = wSize >= wBytes ? wBytes : wSize;
        if (write_cpu_flash(wAddress, pchSrc, hwByteToWrite)) {
            wAddress += wBytes;
            pchSrc += wBytes;
            wSize -= wBytes;
        } else {
            nReturn = 0;
            break;
        }
    }

    if (1 == chOriginLocked) {
        HAL_FLASH_Lock();
    }

    __set_PRIMASK(wPrimaskStatus);                  // 避免误开中断

    return nReturn;
}

/*************************** End of file ****************************/
