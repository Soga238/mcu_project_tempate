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
*       multi_host_utilities.c *                                     *
*                                                                    *
**********************************************************************
*/

#include "./multi_host_utilities.h"
#include "./multi_host_port.h"
#include "./multi_host_param.h"
#include "../modbus/modbus.h"

/*********************************************************************
*
*       Default
*
**********************************************************************
*/

#define MB_READ_REGCNT_MIN             (0x0001)      /*! 读取保持寄存器的最小长度 */
#define MB_READ_REGCNT_MAX             (0x007D)      /*! 读取保持寄存器的最大长度 */

#define MB_READ_COILCNT_MIN            (0x0001)      /*! 读取线圈的最小长度      */
#define MB_READ_COILCNT_MAX            (0x07D0)      /*! 读取线圈的最大长度      */

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define PROCESS_TIME                    150         /*! 预估远端设备处理 modbus 指令所需的时间，单位毫秒 */

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

/**
 * \brief       ID号转MODBUS从机地址
 * \param[in]   chPort     端口号
 * \param[in]   chId       ID值
 * \param[out]  pchSlave   MODBUS从机地址
 * \return      -1(失败)
 */
int8_t id_to_address(uint8_t chPort, uint8_t chId, uint8_t *pchSlave)
{
    const port_resource_t *ptRes = NULL;

    ptRes = proxy_get_port_resource(chPort);
    if (NULL == ptRes) {
        return -1;
    }

    for (uint8_t i = 0; i < ADDRESS_MAP_NUM; i++) {
        if ((chId >= ptRes->tBuf[i].chIdStart) &&
            (chId <= ptRes->tBuf[i].chIdEnd)) {
            *pchSlave = ptRes->tBuf[i].chSlaveStart + chId - ptRes->tBuf[i].chIdStart;
            return 0;
        }
    }

    return -1;
}

/**
 * \brief       MODBUS从机地址转ID号
 * \param[in]   chPort     端口号
 * \param[in]   chSlave    MODBUS从机地址
 * \param[out]  *pchID     指向ID变量的指针
 * \return      -1(失败)
 */
static int8_t address_to_id(uint8_t chPort, uint8_t chSlave, uint8_t *pchID)
{
    const port_resource_t *ptRes = NULL;

    ptRes = proxy_get_port_resource(chPort);
    if (NULL == ptRes) {
        return -1;
    }

    for (uint8_t i = 0; i < ADDRESS_MAP_NUM; i++) {
        if (ptRes->tBuf[i].chSlaveStart <= chSlave &&
            ptRes->tBuf[i].chSlaveEnd >= chSlave) {
            *pchID = chSlave + ptRes->tBuf[i].chIdStart - ptRes->tBuf[i].chSlaveStart;
            return 0;
        }
    }

    return -1;
}

/**
 * \brief       为 request 请求添加转发目的端口号
 * \param[in]   ptRequest     指向request结构体的指针
 * \return      -1(失败)
 */
static int32_t add_dst_port_to_request(proxy_request_t *ptRequest)
{
    const port_resource_t *ptRes = NULL;

    ptRes = proxy_get_port_resource(ptRequest->chPortSrc);
    if (NULL == ptRes) {
        return -1;
    }

    ptRequest->chPortDstNum = 0;

    for (uint8_t i = 0; i < proxy_get_slave_port_num(); i++) {
        ptRes = proxy_get_slave_port_resource(i);
        if (NULL == ptRes) {
            continue;
        }

        for (uint8_t j = 0; j < ADDRESS_MAP_NUM; j++) {
            if ((ptRes->tBuf[j].chIdStart <= ptRequest->chID) &&
                (ptRes->tBuf[j].chIdEnd >= ptRequest->chID)) {
                ptRequest->chPortDstBuf[ptRequest->chPortDstNum++] = ptRes->chPort;
            }
        }
    }

    return 0 == ptRequest->chPortDstNum ? -1 : 0;
}

/**
 * \brief       计算 request_cfg 请应的 modbus 协议占用的字节长度
 * \param[in]   ptCfg     指向 request_cfg_t 结构体的指针
 * \return      -1(失败)
 */
static int32_t calc_request_body_size(const request_cfg_t *ptCfg)
{
    volatile int32_t nSize = -1;

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_REGISTER:
            nSize = 8;
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS:
            nSize = CALC_READ_COILS_REQUIRED_BYTES(ptCfg->hwDataNumber) + 9;
            break;

        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            nSize = CALC_READ_REGISTERS_REQUIRED_BYTES(ptCfg->hwDataNumber) + 9;
            break;

        default:
            break;
    }

    return nSize;
}

/**
 * \brief       计算 response_cfg 对应的 modbus 协议占用的字节长度
 * \param[in]   ptCfg     指向 resonse_cfg_t 结构体的指针
 * \return      -1(失败)
 */
static int32_t calc_response_body_size(const response_cfg_t *pCfg)
{
    volatile int32_t nSize = -1;

    switch (pCfg->chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            nSize = 5 + pCfg->chByteCount;
            break;

        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_REGISTER:
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            nSize = 8;
            break;

        default:
            break;
    }

    return nSize;
}

/**
 * \brief       根据一组符合 MODBUS 协议的字节数据，生成 request_cfg_t 类型的结构体
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 request_cfg_t 结构体的指针
 * \return      -1(失败)
 */
static int32_t make_request_cfg(uint8_t *pchBuffer, uint16_t hwBufferSize, request_cfg_t *ptCfg)
{
    if ((NULL == pchBuffer) || (4 > hwBufferSize)) {
        return -1;
    }

    ptCfg->chSlave = pchBuffer[0];
    ptCfg->chCode = pchBuffer[1];

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_REGISTER:
            if (8 > hwBufferSize) {
                return -1;
            }
            break;

        default:
            break;
    }

    ptCfg->hwDataAddr = CHAR_HL_SHORT(pchBuffer[2], pchBuffer[3]);

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
            ptCfg->hwDataNumber = CHAR_HL_SHORT(pchBuffer[4], pchBuffer[5]);
            if ((MB_READ_REGCNT_MIN > ptCfg->hwDataNumber) ||
                (MB_READ_REGCNT_MAX < ptCfg->hwDataNumber)) {
                return -1;
            }
            break;

        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_COILS:
            ptCfg->hwDataNumber = CHAR_HL_SHORT(pchBuffer[4], pchBuffer[5]);
            if ((MB_READ_COILCNT_MIN > ptCfg->hwDataNumber) ||
                (MB_READ_COILCNT_MAX < ptCfg->hwDataNumber)) {
                return -1;
            }
            break;

        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_REGISTER:
            ptCfg->hwDataNumber = 1;
            ptCfg->pchWR = &pchBuffer[4];
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            ptCfg->hwDataNumber = CHAR_HL_SHORT(pchBuffer[4], pchBuffer[5]);
            ptCfg->pchWR = &pchBuffer[7];
            break;

        default:
            return -1;
    }

    return 0;
}

/**
 * \brief       根据一组 MODBUS 字节数据，生成 response_cfg_t 类型的结构体
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 response_cfg_t 结构体的指针
 * \return      -1(失败)
 */
static int32_t make_response_cfg(uint8_t *pchBuffer, uint16_t hwBufferSize, response_cfg_t *ptCfg)
{
    volatile uint16_t hwCoilValue = 0;
    int32_t nRetval = 0;

    if ((NULL == pchBuffer) || (4 > hwBufferSize)) {
        return -1;
    }

    ptCfg->chSlave = pchBuffer[0];
    ptCfg->chCode = pchBuffer[1];

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_COILS:
            ptCfg->chByteCount = pchBuffer[2];
            ptCfg->pchHR = &pchBuffer[3];
            if ((ptCfg->chByteCount >> 1) > MB_READ_REGCNT_MAX) {
                nRetval = -1;
            }
            break;

        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_REGISTER:
            ptCfg->hwOutputNumber = 1;
            ptCfg->hwDataAddr = CHAR_HL_SHORT(pchBuffer[2], pchBuffer[3]);
            ptCfg->pchHR = &pchBuffer[4];

            // 使用 volatile 修饰 hwCoilValue 避免优化此段代码
            if (MB_FUNC_WRITE_SINGLE_COIL == ptCfg->chCode) {
                hwCoilValue = CHAR_HL_SHORT(pchBuffer[4], pchBuffer[5]);
                if ((COIL_VALUE_OFF == hwCoilValue) || (COIL_VALUE_ON == hwCoilValue)) {
                    break;
                } else {
                    nRetval = -1;
                }
            }
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            ptCfg->hwDataAddr = CHAR_HL_SHORT(pchBuffer[2], pchBuffer[3]);
            ptCfg->hwOutputNumber = CHAR_HL_SHORT(pchBuffer[4], pchBuffer[5]);
            break;

        // 异常代码
        case (0x80 + MB_FUNC_READ_INPUT_REGISTERS):
        case (0x80 + MB_FUNC_READ_HOLDING_REGISTERS):
        case (0x80 + MB_FUNC_READ_DISCRETE_INPUTS):
        case (0x80 + MB_FUNC_READ_COILS):
        case (0x80 + MB_FUNC_WRITE_SINGLE_COIL):
        case (0x80 + MB_FUNC_WRITE_REGISTER):
            ptCfg->chExceptionCode = pchBuffer[2];
            break;

        default:
            nRetval = -1;
            break;
    }

    return nRetval;
}


/**
 * \brief       将从端口接收到的类型为 raw_data_t 的数据，转换成 response 请求格式
 * \param[in]   ptRawData     指向 raw_data_t 类型的结构体的数据
 * \param[out]  ptCfg         指向 response_cfg_t 类型结构体的指针
 * \return      -1(失败)
 */
int32_t raw_data_transform_response(raw_data_t *ptRawData, proxy_response_t *ptResponse)
{
    response_cfg_t *ptCfg = NULL;

    if ((NULL == ptRawData) || (NULL == ptResponse)) {
        return -1;
    }

    ptCfg = &ptResponse->tBodyCfg;
    if (0 != make_response_cfg(ptRawData->chBuf, ptRawData->hwBufSize, ptCfg)) {
        return -1;
    }

    ptResponse->ptRaw = ptRawData;
    ptResponse->chPortSrc = ptRawData->chPort;
    if (0 != address_to_id(ptResponse->chPortSrc, ptCfg->chSlave, &ptResponse->chID)) {
        return -1;
    }

    return 0;
}

/**
 * \brief       将从端口接收到的类型为 raw_data_t 的数据，转换成 request 请求格式
 * \param[in]   ptRawData     指向 raw_data_t 类型的结构体的数据
 * \param[out]  ptCfg         指向 request_cfg_t 类型结构体的指针
 * \return      -1(失败)
 */
int32_t raw_data_transform_request(raw_data_t *ptRawData, proxy_request_t *ptRequest)
{
    request_cfg_t *ptCfg = NULL;

    if ((NULL == ptRawData) || (NULL == ptRequest)) {
        return -1;
    }

    ptCfg = &ptRequest->tBodyCfg;
    if (0 != make_request_cfg(ptRawData->chBuf, ptRawData->hwBufSize, ptCfg)) {
        return -1;
    }

    ptRequest->ptRaw = ptRawData;
    ptRequest->chPortSrc = ptRawData->chPort;
    if (0 != address_to_id(ptRequest->chPortSrc, ptCfg->chSlave, &ptRequest->chID)) {
        return -1;
    }

    if (0 != add_dst_port_to_request(ptRequest)) {
        return -1;
    }

    return 0;
}

/**
 * \brief       根据 request_cfg_t 类型的结构体，生成 MODBUS 协议体数据
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 request_cfg_t 结构体的指针
 * \return      -1(失败)
 */
int32_t make_request_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const request_cfg_t *ptCfg)
{
    int32_t nLength = 0;
    uint16_t hwCRC = 0;

    if ((NULL == pchBuffer) || (NULL == ptCfg)) {
        return -1;
    }

    nLength = calc_request_body_size(ptCfg);
    if ((nLength < 0) || (nLength > (int32_t)hwBufferSize)) {
        return -1;
    }

    nLength = 0;
    pchBuffer[nLength++] = ptCfg->chSlave;
    pchBuffer[nLength++] = ptCfg->chCode;

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
            pchBuffer[nLength++] = ptCfg->hwDataAddr >> 8;
            pchBuffer[nLength++] = ptCfg->hwDataAddr;
            pchBuffer[nLength++] = ptCfg->hwDataNumber >> 8;
            pchBuffer[nLength++] = ptCfg->hwDataNumber;
            break;

        default:
            return -1;
    }

    hwCRC = CRC16(pchBuffer, nLength);
    pchBuffer[nLength++] = hwCRC;
    pchBuffer[nLength++] = hwCRC >> 8;

    return nLength;
}

/**
 * \brief       根据 response_cfg_t 类型的结构体，生成 MODBUS 协议体数据
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 response_cfg_t 结构体的指针
 * \return      -1(失败)
 */
int32_t make_response_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const response_cfg_t *ptCfg)
{
    int32_t nLength = 0;
    uint16_t hwCRC = 0;

    if ((NULL == pchBuffer) || (NULL == ptCfg)) {
        return -1;
    }

    nLength = calc_response_body_size(ptCfg);
    if ((nLength < 0) || (nLength > (int32_t)hwBufferSize)) {
        return -1;
    }

    nLength = 0;
    pchBuffer[nLength++] = ptCfg->chSlave;
    pchBuffer[nLength++] = ptCfg->chCode;

    switch (ptCfg->chCode) {
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            pchBuffer[nLength++] = ptCfg->chByteCount;
            short_copy_xch(&pchBuffer[nLength], ptCfg->pchHR, ptCfg->chByteCount >> 1, true);
            nLength += ptCfg->chByteCount;
            break;

        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_COILS:
            pchBuffer[nLength++] = ptCfg->chByteCount;
            memcpy(&pchBuffer[nLength], ptCfg->pchHR, ptCfg->chByteCount);
            nLength += ptCfg->chByteCount;
            break;

        default:
            return -1;
    }

    hwCRC = CRC16(pchBuffer, nLength);
    pchBuffer[nLength++] = hwCRC;
    pchBuffer[nLength++] = hwCRC >> 8;

    return nLength;
}

/**
 * \brief       比较 request 请求和 response 应答是否匹配
 * \param[in]   ptReqCfg        指向 request_cfg_t 结构体的指针
 * \param[in]   ptRespCfg       指向 response_cfg_t 结构体的指针
 * \return      -1(失败)
 */
int32_t is_response_match_request(const request_cfg_t *ptReqCfg, const response_cfg_t *ptRespCfg)
{
    uint32_t wSize;
    int32_t nRetval = -1;

    if ((NULL == ptReqCfg) || (NULL == ptRespCfg)) {
        return -1;
    }

    if (ptReqCfg->chCode != ptRespCfg->chCode) {
        return -1;
    }

    switch (ptReqCfg->chCode) {
        case MB_FUNC_READ_INPUT_REGISTERS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
            wSize = CALC_READ_REGISTERS_REQUIRED_BYTES(ptReqCfg->hwDataNumber);
            nRetval = (wSize != ptRespCfg->chByteCount) ? -1 : 0;
            break;

        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_COILS:
            wSize = CALC_READ_COILS_REQUIRED_BYTES(ptReqCfg->hwDataNumber);
            nRetval = (wSize != ptRespCfg->chByteCount) ? -1 : 0;
            break;

        case MB_FUNC_WRITE_REGISTER:
        case MB_FUNC_WRITE_SINGLE_COIL:
            nRetval = (0 != memcmp(ptReqCfg->pchWR, ptRespCfg->pchHR, 2)) ? -1 : 0;
            break;

        default:
            break;
    }

    return nRetval;
}

/**
 * \brief       完全拷贝一个 request 请求
 * \param[in]   ptRequest        指向 request 类型结构体的指针
 * \return      request请求的拷贝
 *              NULL(拷贝失败)
 */
proxy_request_t *proxy_mb_request_copy(const proxy_request_t *ptRequest)
{
    proxy_request_t *ptRequestCopy = NULL;
    raw_data_t *ptRawData = NULL;

    if (NULL == ptRequest) {
        return NULL;
    }

    ptRequestCopy = (proxy_request_t *)PROXY_REQUEST_MALLOC();
    if (NULL == ptRequestCopy) {
        return NULL;
    }

    ptRawData = RAW_DATA_MALLOC();
    if (NULL != ptRawData) {
        memcpy(ptRequestCopy, ptRequest, sizeof(proxy_request_t));
        ptRequestCopy->ptRaw = ptRawData;

        memcpy(ptRawData->chBuf, ptRequest->ptRaw->chBuf, ptRequest->ptRaw->hwBufSize);
        ptRawData->chPort = ptRequest->ptRaw->chPort;
        ptRawData->hwBufSize = ptRequest->ptRaw->hwBufSize;

    } else {
        PROXY_REQUEST_FREE(ptRequestCopy);
        ptRequestCopy = NULL;
    }

    return ptRequestCopy;
}

/**
 * \brief       在给定波特率的条件下，计算字节数据传输时间
 * \param[in]   wBaudrate        波特率
 * \param[in]   hwSendLength     字节发送长度
 * \return      传输时间，单位毫秒
 */
static uint32_t get_transfer_time(uint32_t wBaudrate, uint16_t hwSendLength)
{
    uint32_t wTransTime = 0;

    /*!
        以波特率 9600 为基准，1 + 8 + 1， 大概1ms一个字符。200ms 则是处理数据所需
        时间的预估值
    */
    switch (wBaudrate) {
        case 2400:
            wTransTime = hwSendLength << 2;
            break;
        case 4800:
            wTransTime = hwSendLength << 1;
            break;
        default:
        case 9600:
            wTransTime = hwSendLength;
            break;
        case 19200:
            wTransTime = hwSendLength >> 1;
            break;
        case 38400:
            wTransTime = hwSendLength >> 2;
            break;
        case 57600:
        case 115200:
            wTransTime = (hwSendLength >> 2) + (hwSendLength >> 3);
            break;
    }

    return (wTransTime == 0) ? 1 : wTransTime;
}

/**
 * \brief       计算从主机发送请求，到从机返回应答的等待时间（预估值）
 * \param[in]   ptData        指向 raw_data_t 类型结构体的指针
 * \param[in]   wBaudrate     波特率
 * \return      等待时间，单位毫秒
 */
uint32_t calc_wait_response_time(const raw_data_t *ptData, uint32_t wBaudrate)
{
    uint32_t wWaitTime = PROCESS_TIME;
    uint16_t hwSendLength = 0;

    switch (ptData->chBuf[1]) {
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            hwSendLength =  CHAR_HL_SHORT(ptData->chBuf[4], ptData->chBuf[5]);
            break;

        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_COILS:
            hwSendLength =  CHAR_HL_SHORT(ptData->chBuf[4], ptData->chBuf[5]) >> 3;
            break;

        default:
            break;
    }

    hwSendLength += ptData->hwBufSize;
    wWaitTime += get_transfer_time(wBaudrate, hwSendLength);

    return wWaitTime;
}

/**
* \brief       判断一个 request 是一个写操作
* \param[in]   ptRequest        指向 request 类型结构体的指针
* \return      true
*              false
*/
bool is_request_write_operation(const proxy_request_t *ptRequest)
{
    bool bRetval = false;

    switch (ptRequest->tBodyCfg.chCode) {
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        case MB_FUNC_WRITE_SINGLE_COIL:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS_RDHR:
        case MB_FUNC_WRITE_REGISTER:
            bRetval = true;
            break;

        default:
            break;
    }

    return bRetval;
}

/**
* \brief       计算两个区域的重叠部分
* \param[in]   ptInput  包含两个区域信息的输入结构体
* \param[out]  ptOutput 包含重叠区域信息的输出结构体
* \return      -1(失败)
*/
int32_t calc_overlapping_area(const overlapping_area_input_t *ptInput, overlapping_area_output_t *ptOutput)
{
    int32_t nStart = 0;
    int32_t nEnd = 0;
    int32_t nOverSize = 0;

    if ((NULL == ptInput) || (NULL == ptOutput)) {
        return -1;
    } else if ((0 >= ptInput->nFirstAreaSize) || (0 >= ptInput->nSecondAreaSize)) {
        return -1;
    }

    nStart = MIN(ptInput->nFirstAreaStart, ptInput->nSecondAreaStart);

    nEnd = MAX(ptInput->nFirstAreaStart + ptInput->nFirstAreaSize,
               ptInput->nSecondAreaStart + ptInput->nSecondAreaSize);

    nOverSize = ptInput->nFirstAreaSize + ptInput->nSecondAreaSize - (nEnd - nStart);
    if (0 >= nOverSize) {
        return -1;
    }

    if (ptInput->nSecondAreaStart < ptInput->nFirstAreaStart) {
        ptOutput->nAreaStart = ptInput->nFirstAreaStart;
    } else {
        ptOutput->nAreaStart = ptInput->nSecondAreaStart;
    }

    ptOutput->nAreaSize = nOverSize;

    return 0;
}

/*************************** End of file ****************************/
