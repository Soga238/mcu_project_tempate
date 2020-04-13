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
#include "..\hal\mcu\plc_board\Drivers\STM32F4xx_HAL_Driver\Inc\stm32f4xx_hal.h"

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
typedef struct {
    uint8_t chSectorNum;
    uint8_t chBank;
    uint32_t wSectorAddr;
    uint32_t wSectorSize;
} flash_sector_t;

/* Private define --------------------------------------------------*/
/* base address of the flash sectors */
#define ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base address of Sector 0, 16 K bytes   */
#define ADDR_FLASH_SECTOR_1      ((uint32_t)0x08004000) /* Base address of Sector 1, 16 K bytes   */
#define ADDR_FLASH_SECTOR_2      ((uint32_t)0x08008000) /* Base address of Sector 2, 16 K bytes   */
#define ADDR_FLASH_SECTOR_3      ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 K bytes   */
#define ADDR_FLASH_SECTOR_4      ((uint32_t)0x08010000) /* Base address of Sector 4, 64 K bytes   */
#define ADDR_FLASH_SECTOR_5      ((uint32_t)0x08020000) /* Base address of Sector 5, 128 K bytes  */
#define ADDR_FLASH_SECTOR_6      ((uint32_t)0x08040000) /* Base address of Sector 6, 128 K bytes  */
#define ADDR_FLASH_SECTOR_7      ((uint32_t)0x08060000) /* Base address of Sector 7, 128 K bytes  */
#define ADDR_FLASH_SECTOR_8      ((uint32_t)0x08080000) /* Base address of Sector 8, 128 K bytes  */
#define ADDR_FLASH_SECTOR_9      ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 K bytes  */
#define ADDR_FLASH_SECTOR_10     ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes */
#define ADDR_FLASH_SECTOR_11     ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base address of Sector 12, 16 K bytes  */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base address of Sector 13, 16 K bytes  */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base address of Sector 14, 16 K bytes  */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base address of Sector 15, 16 K bytes  */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base address of Sector 16, 64 K bytes  */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base address of Sector 17, 128 K bytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base address of Sector 18, 128 K bytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base address of Sector 19, 128 K bytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base address of Sector 20, 128 K bytes */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base address of Sector 21, 128 K bytes */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base address of Sector 22, 128 K bytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base address of Sector 23, 128 K bytes */

/* Private macro ---------------------------------------------------*/
/* Private variables -----------------------------------------------*/
static const flash_sector_t s_tFlashSectorTable[FLASH_SECTOR_TABLE_SIZE] = {
    {FLASH_SECTOR_0,  FLASH_BANK_1, ADDR_FLASH_SECTOR_0,    0x00004000},  // 16K
    {FLASH_SECTOR_1,  FLASH_BANK_1, ADDR_FLASH_SECTOR_1,    0x00004000},  // 16K
    {FLASH_SECTOR_2,  FLASH_BANK_1, ADDR_FLASH_SECTOR_2,    0x00004000},  // 16K
    {FLASH_SECTOR_3,  FLASH_BANK_1, ADDR_FLASH_SECTOR_3,    0x00004000},  // 16K
    {FLASH_SECTOR_4,  FLASH_BANK_1, ADDR_FLASH_SECTOR_4,    0x00010000},  // 64K
    {FLASH_SECTOR_5,  FLASH_BANK_1, ADDR_FLASH_SECTOR_5,    0x00020000},  // 128K
    //{FLASH_SECTOR_6,  ADDR_FLASH_SECTOR_6,    0x00020000},  // 128K
    //{FLASH_SECTOR_7,  ADDR_FLASH_SECTOR_7,    0x00020000},  // 128K
    //{FLASH_SECTOR_8,  ADDR_FLASH_SECTOR_8,    0x00020000},  // 128K
    //{FLASH_SECTOR_9,  ADDR_FLASH_SECTOR_9,    0x00020000},  // 128K
    //{FLASH_SECTOR_10, ADDR_FLASH_SECTOR_10,   0x00020000},  // 128K
    //{FLASH_SECTOR_11, ADDR_FLASH_SECTOR_11,   0x00020000},  // 128K
    //{FLASH_SECTOR_12, ADDR_FLASH_SECTOR_12,   0x00004000},  // 16K
    //{FLASH_SECTOR_13, ADDR_FLASH_SECTOR_13,   0x00004000},  // 16K
    //{FLASH_SECTOR_14, ADDR_FLASH_SECTOR_14,   0x00004000},  // 16K
    //{FLASH_SECTOR_15, ADDR_FLASH_SECTOR_15,   0x00004000},  // 16K
    //{FLASH_SECTOR_16, ADDR_FLASH_SECTOR_16,   0x00010000},  // 64K
    //{FLASH_SECTOR_17, ADDR_FLASH_SECTOR_17,   0x00020000},  // 128K
    //{FLASH_SECTOR_18, ADDR_FLASH_SECTOR_18,   0x00020000},  // 128K
    //{FLASH_SECTOR_19, ADDR_FLASH_SECTOR_19,   0x00020000},  // 128K
    //{FLASH_SECTOR_20, ADDR_FLASH_SECTOR_20,   0x00020000},  // 128K
    //{FLASH_SECTOR_21, ADDR_FLASH_SECTOR_21,   0x00020000},  // 128K
    //{FLASH_SECTOR_22, ADDR_FLASH_SECTOR_22,   0x00020000},  // 128K
    //{FLASH_SECTOR_23, ADDR_FLASH_SECTOR_23,   0x00020000},  // 128K
};

/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

static const flash_sector_t *__get_sector(uint32_t wAddr)
{
    for (uint8_t i = 0; i < countof(s_tFlashSectorTable); ++i) {
        if ((s_tFlashSectorTable[i].wSectorAddr <= wAddr) &&
            (wAddr < (s_tFlashSectorTable[i].wSectorAddr + s_tFlashSectorTable[i].wSectorSize))) {
            return &s_tFlashSectorTable[i];
        }
    }
    return NULL;
}

/**
 * \brief 检查扇区是否已经擦除
 * \param pchBuf        字节数组指针
 * \param wSize         字节数组长度
 *
 */
static bool __is_sector_erased(const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t i = 0;
    for (i = 0; i < wSize; ++i) {
        if (pchBuf[0] != 0xFF) {
            break;
        }
    }
    return i == wSize;
}

static int32_t write_cpu_flash(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    uint32_t i = 0;
    uint32_t wTemp = 0;
    uint32_t wSectorError = 0;
    HAL_StatusTypeDef tStatus = HAL_OK;

    FLASH_EraseInitTypeDef tEraseInfo = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
        .NbSectors = 1
    };

    const flash_sector_t *ptSector = __get_sector(wFlashAddr);

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    // word write
    for (i = 0; i < (wSize >> 2); i++) {
        if ((ptSector->wSectorAddr == wFlashAddr) &&
            !(__is_sector_erased((uint8_t *)ptSector->wSectorAddr, ptSector->wSectorSize))) {
            tEraseInfo.Banks = ptSector->chBank;
            tEraseInfo.Sector = ptSector->chSectorNum;
            if (HAL_OK != HAL_FLASHEx_Erase(&tEraseInfo, &wSectorError)) {
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

    // half word write
    if (wSize & 0x02) {
        if ((ptSector->wSectorAddr == wFlashAddr) &&
            !(__is_sector_erased((uint8_t *)ptSector->wSectorAddr, ptSector->wSectorSize))) {
            tEraseInfo.Banks = ptSector->chBank;
            tEraseInfo.Sector = ptSector->chSectorNum;
            if (HAL_OK != HAL_FLASHEx_Erase(&tEraseInfo, &wSectorError)) {
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
 * \brief FLASH写入函数, 当写入范围包含扇区首地址时，会擦除首地址扇区
 *
 * \param wFlashAddr    32位写入地址
 * \param pchBuf        字节数组指针
 * \param wSize         字节数组长度
 *
 */
int32_t flash_write(uint32_t wFlashAddr, const uint8_t *pchBuf, uint32_t wSize)
{
    const uint8_t *pchSrc = pchBuf;
    uint32_t wAddress = wFlashAddr;
    uint32_t wBytes, wPrimaskStatus;
    int32_t nReturn = 1;
    uint8_t chLocked = 0;

    const flash_sector_t *ptSector = NULL;

    if ((wFlashAddr < FLASH_BASE_ADDRESS) ||
        (wFlashAddr + wSize) > (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)) {
        return 0;
    }

    if ((wFlashAddr & 0x01) || (wSize & 0x01)) {    // 写入地址2字节对齐，写入字节个数为2的倍数
        return 0;
    }

    ptSector = __get_sector(wFlashAddr);            // STM32F4 以块为最小擦除单元
    if (NULL == ptSector) {
        return 0;
    }

    wPrimaskStatus = __get_PRIMASK();               // 获取中断状态
    __set_PRIMASK(1);                               // 关闭总中断

    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != RESET) {
        HAL_FLASH_Unlock();
        chLocked = 1;                               // 判断FLASH后面是否要重新锁住
    }

    while (wSize) {
        wBytes = MIN((ptSector->wSectorAddr + ptSector->wSectorSize - wFlashAddr), wSize);
        if (write_cpu_flash(wAddress, pchSrc, wBytes)) {
            wAddress += wBytes;
            pchSrc += wBytes;
            wSize -= wBytes;
        } else {
            nReturn = 0;
            break;
        }
    }

    if (1 == chLocked) {
        HAL_FLASH_Lock();
    }

    __set_PRIMASK(wPrimaskStatus);                 // 避免误开中断

    return nReturn;
}

/*************************** End of file ****************************/
