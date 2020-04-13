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
*       multi_host_cache_utilities.c *                               *
*                                                                    *
**********************************************************************
*/
#include "./multi_host_cache_utilties.h"
#include "./multi_host_utilities.h"
#include "../modbus/modbus.h"
#include "../../service/heap/heap.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

struct modbus_overlapping_address {
    uint8_t *pchSrcBuffer;
    uint8_t *pchDstBuffer;

    uint16_t hwSrcDataAddr;
    uint16_t hwDstDataAddr;

    uint16_t hwSrcDataNum;
    uint16_t hwDstDataNum;
};

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

int32_t find_compatible_cache_record_hook(void *res, void *data)
{
    cache_data_t *ptCache = (cache_data_t *)res;
    search_cache_data_t *ptSearch = (search_cache_data_t *)data;

    if (ptCache->chID != ptSearch->chID) {
        return -1;
    } else if (ptCache->chCode != ptSearch->chCode) {
        return -1;
    }

    if (ptCache->hwDataAddr <= ptSearch->hwDataAddr) { // TODO 小于等于可以包含 06 和 16
        return 0;
    }

    return -1;
}


/**
 * \brief       寻找和 request 请求匹配的记录
 * \param[in]   res       对象内存地址
 * \param[in]   data      对象内存地址
 * \return      -1(失败)
 */
int32_t find_equal_cache_record_hook(void *res, void *data)
{
    cache_data_t *ptCache = (cache_data_t *)res;
    search_cache_data_t *ptSearch = (search_cache_data_t *)data;

    if (ptCache->chID != ptSearch->chID) {
        return -1;
    } else if (ptCache->chCode != ptSearch->chCode) {
        return -1;
    } else if (ptCache->hwDataAddr != ptSearch->hwDataAddr) {
        return -1;
    } else if (ptCache->hwDataNumber != ptSearch->hwDataNumber) {
        return -1;
    }

    return 0;
}

/**
 * \brief       过期计数器增减钩子函数
 * \param[in]   res       对象内存地址
 * \param[in]   data      对象内存地址
 * \return      -1(失败)
 */
int32_t decrease_counter_hook(void *ptCacheData, void *ptData)
{
    cache_data_t *ptCache = (cache_data_t *)ptCacheData;

    if (0 < ptCache->chExpirationCounter) {
        ptCache->chExpirationCounter--;
    }

    return 0;
}

/**
 * \brief       寻找过期计数器为0的钩子函数
 * \param[in]   res       对象内存地址
 * \param[in]   data      对象内存地址
 * \return      -1(失败)
 */
int32_t find_expired_record_hook(void *ptCacheData, void *ptData)
{
    cache_data_t *ptCache = (cache_data_t *)ptCacheData;
    return (ptCache->chExpirationCounter == 0) ? 0 : -1;
}

/**
 * \brief       释放缓存内存的钩子函数
 * \param[in]   res       对象内存地址
 * \param[in]   data      对象内存地址
 * \return      -1(失败)
 */
int32_t destroy_cache_data_hook(void *ptCacheData)
{
    cache_data_t *ptCache = (cache_data_t *)ptCacheData;

    SYSLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"delete record: ID(%d) code(%d) addr(%04x) num(%d)\r\n",
               ptCache->chID, ptCache->chCode,
               ptCache->hwDataAddr,
               ptCache->hwDataNumber);

    port_free_4(ptCache->pchDataBuffer);
    return 0;
}

/**
 * \brief       删除链表中过期的记录
 * \param[in]   ptList    链表对象内存地址
 * \return      -1(失败)
 */
int32_t delete_expired_record_in_list(cache_data_list_t *ptList)
{
    return cache_data_list_delete_equal(ptList, find_expired_record_hook, NULL, destroy_cache_data_hook);
}

/**
 * \brief       遍历链表，增减过期计数器
 * \param[in]   ptList    链表对象内存地址
 * \return      -1(失败)
 */
void decrease_expiration_counter_in_list(cache_data_list_t *ptList)
{
    cache_data_list_travel(ptList, decrease_counter_hook, NULL);
}

/**
 * \brief       遍历链表，删除跟request匹配上的记录
 * \param[in]   ptList    链表对象内存地址
 * \return      -1(失败)
 */
int32_t delete_equal_request_in_list(cache_data_list_t *ptList, proxy_request_t *ptRequest)
{
    return cache_data_list_delete_equal(ptList, find_equal_cache_record_hook, ptRequest, destroy_cache_data_hook);
}

/**
 * \brief       计算字节占用空间
 * \param[in]   chCode          MODBUS 功能码
 * \param[in]   hwDataNumber    MODBUS 数据读取个数
 * \return      -1(失败)
 */
uint8_t calc_storage_space_in_byte(uint8_t chCode, uint16_t hwDataNumber)
{
    volatile uint8_t chLength = 0;

    switch (chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
            chLength = CALC_READ_COILS_REQUIRED_BYTES(hwDataNumber);
            break;

        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            chLength = CALC_READ_REGISTERS_REQUIRED_BYTES(hwDataNumber);
            break;
        
        default:
            break;
    }

    return chLength;
}

/**
 * \brief       将缓存记录转换为相应的 request 请求
 * \param[in]   ptCache     缓存记录对象内存地址
 * \param[in]   ptRequest   request请求对象内存地址
 * \return      -1(失败)
 */
int32_t cache_data_transform_request(const cache_data_t *ptCache, proxy_request_t *ptRequest)
{
    uint8_t chSlave = 0;
    int32_t nLength;

    if ((NULL == ptCache) || (NULL == ptRequest)) {
        return -1;
    } else if (NULL == ptRequest->ptRaw) {
        return -1;
    }

    if (0 != id_to_address(ptCache->chPort, ptCache->chID, &chSlave)) {
        return -1;
    }

    ptRequest->tBodyCfg.chSlave = chSlave;
    ptRequest->tBodyCfg.chCode = ptCache->chCode;
    ptRequest->tBodyCfg.hwDataAddr = ptCache->hwDataAddr;
    ptRequest->tBodyCfg.hwDataNumber = ptCache->hwDataNumber;

    nLength = make_request_body(ptRequest->ptRaw->chBuf, RAW_DATA_BUF_SIZE,
                                &ptRequest->tBodyCfg);

    if (0 > nLength) {
        return -1;
    }

    ptRequest->chPortSrc = CH_INTERNAL_CACHE_PORT;    // 内部缓存端口
    ptRequest->chID = ptCache->chID;
    ptRequest->chPortDstBuf[0] = ptCache->chPort;
    ptRequest->chPortDstNum = 1;

    ptRequest->ptRaw->chPort = ptCache->chPort;
    ptRequest->ptRaw->hwBufSize = (uint16_t)nLength;

    return 0;
}

/**
* \brief       更新重叠区域的寄存器数据，16位长度
* \return      -1(失败)
*/
static int32_t refresh_overlapping_area_registers(struct modbus_overlapping_address *ptObejct)
{
    uint16_t *phwSrcBuffer = NULL;
    uint16_t *phwDstBuffer = NULL;

    overlapping_area_input_t tIn;
    overlapping_area_output_t tOut;

    tIn.nFirstAreaStart     = ptObejct->hwSrcDataAddr;
    tIn.nFirstAreaSize      = ptObejct->hwSrcDataNum;

    tIn.nSecondAreaStart    = ptObejct->hwDstDataAddr;
    tIn.nSecondAreaSize     = ptObejct->hwDstDataNum;

    if (0 != calc_overlapping_area(&tIn, &tOut)) {
        return -1;
    }

    //SYSLOG_RAW("start %d size %d start %d size %d\r\n", tIn.nFirstAreaStart, tIn.nFirstAreaSize, tIn.nSecondAreaStart, tIn.nSecondAreaSize);

    phwSrcBuffer = (uint16_t *)ptObejct->pchSrcBuffer + (tOut.nAreaStart - tIn.nFirstAreaStart);
    phwDstBuffer = (uint16_t *)ptObejct->pchDstBuffer + (tOut.nAreaStart - tIn.nSecondAreaStart);

    short_copy_xch(phwDstBuffer, phwSrcBuffer, tOut.nAreaSize, true);

    return -1;
}

/**
* \brief       更新重叠区域的线圈值，单位bit
* \return      -1(失败)
*/
static int32_t refresh_overlapping_area_coils(struct modbus_overlapping_address *ptObejct)
{
    uint8_t *pchSrcBuffer = NULL;
    uint8_t *pchDstBuffer = NULL;

    uint8_t chSrcCoilOffest;
    uint8_t chDstCoilOffest;

    overlapping_area_input_t tIn;
    overlapping_area_output_t tOut;

    tIn.nFirstAreaStart     = ptObejct->hwSrcDataAddr;
    tIn.nFirstAreaSize      = ptObejct->hwSrcDataNum;

    tIn.nSecondAreaStart    = ptObejct->hwDstDataAddr;
    tIn.nSecondAreaSize     = ptObejct->hwDstDataNum;

    if (0 != calc_overlapping_area(&tIn, &tOut)) {
        return -1;
    }

    for (int32_t i = tOut.nAreaStart; i < (tOut.nAreaStart + tOut.nAreaSize); ++i) {

        chSrcCoilOffest = (i - tIn.nFirstAreaStart) & 0x07;
        pchSrcBuffer = (uint8_t *)ptObejct->pchSrcBuffer + ((i - tIn.nFirstAreaStart) >> 3);

        chDstCoilOffest = (i - tIn.nSecondAreaStart) & 0x07;
        pchDstBuffer = (uint8_t *)ptObejct->pchDstBuffer + ((i - tIn.nSecondAreaStart) >> 3);

        if (LZ_BIT_GET(*pchSrcBuffer, chSrcCoilOffest)) {
            LZ_BIT_SET(*pchDstBuffer, chDstCoilOffest);
        } else {
            LZ_BIT_CLR(*pchDstBuffer, chDstCoilOffest);
        }
    }

    return 0;
}

/**
* \brief       更新设备数据和缓存数据的重叠部分
* \return      -1(失败)
*/
int32_t refresh_cache_data_object_hook(void *res, void *data)
{
    int32_t nRetval = -1;
    cache_data_t *ptCache = (cache_data_t *)res;
    update_cache_data_t *ptData = (update_cache_data_t *)data;
    struct modbus_overlapping_address tObject;

    tObject.hwDstDataAddr = ptCache->hwDataAddr;
    tObject.hwDstDataNum = ptCache->hwDataNumber;
    tObject.pchDstBuffer = ptCache->pchDataBuffer;

    tObject.hwSrcDataAddr = ptData->hwDataAddr;
    tObject.hwSrcDataNum = ptData->hwDataNumber;
    tObject.pchSrcBuffer = ptData->pchDataBuffer;

    switch (ptCache->chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
            if ((MB_FUNC_READ_COILS           == ptData->chCode) ||
                (MB_FUNC_READ_DISCRETE_INPUTS == ptData->chCode) ||
                (MB_FUNC_WRITE_SINGLE_COIL    == ptData->chCode)) {
                nRetval = refresh_overlapping_area_coils(&tObject);
            }
            break;

        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            if ((MB_FUNC_READ_INPUT_REGISTERS   == ptData->chCode) ||
                (MB_FUNC_READ_HOLDING_REGISTERS == ptData->chCode) ||
                (MB_FUNC_WRITE_REGISTER         == ptData->chCode)) {
                nRetval = refresh_overlapping_area_registers(&tObject);
            }
            break;

        default:
            break;
    }

    return nRetval;
}

/*************************** End of file ****************************/
