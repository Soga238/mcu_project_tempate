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

#define MB_READ_REGCNT_MIN             (0x0001)      /*! ��ȡ���ּĴ�������С���� */
#define MB_READ_REGCNT_MAX             (0x007D)      /*! ��ȡ���ּĴ�������󳤶� */

#define MB_READ_COILCNT_MIN            (0x0001)      /*! ��ȡ��Ȧ����С����      */
#define MB_READ_COILCNT_MAX            (0x07D0)      /*! ��ȡ��Ȧ����󳤶�      */

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define PROCESS_TIME                    150         /*! Ԥ��Զ���豸���� modbus ָ�������ʱ�䣬��λ���� */

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

/**
 * \brief       ID��תMODBUS�ӻ���ַ
 * \param[in]   chPort     �˿ں�
 * \param[in]   chId       IDֵ
 * \param[out]  pchSlave   MODBUS�ӻ���ַ
 * \return      -1(ʧ��)
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
 * \brief       MODBUS�ӻ���ַתID��
 * \param[in]   chPort     �˿ں�
 * \param[in]   chSlave    MODBUS�ӻ���ַ
 * \param[out]  *pchID     ָ��ID������ָ��
 * \return      -1(ʧ��)
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
 * \brief       Ϊ request �������ת��Ŀ�Ķ˿ں�
 * \param[in]   ptRequest     ָ��request�ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ���� request_cfg ��Ӧ�� modbus Э��ռ�õ��ֽڳ���
 * \param[in]   ptCfg     ָ�� request_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ���� response_cfg ��Ӧ�� modbus Э��ռ�õ��ֽڳ���
 * \param[in]   ptCfg     ָ�� resonse_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ����һ����� MODBUS Э����ֽ����ݣ����� request_cfg_t ���͵Ľṹ��
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� request_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ����һ�� MODBUS �ֽ����ݣ����� response_cfg_t ���͵Ľṹ��
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� response_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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

            // ʹ�� volatile ���� hwCoilValue �����Ż��˶δ���
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

        // �쳣����
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
 * \brief       ���Ӷ˿ڽ��յ�������Ϊ raw_data_t �����ݣ�ת���� response �����ʽ
 * \param[in]   ptRawData     ָ�� raw_data_t ���͵Ľṹ�������
 * \param[out]  ptCfg         ָ�� response_cfg_t ���ͽṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ���Ӷ˿ڽ��յ�������Ϊ raw_data_t �����ݣ�ת���� request �����ʽ
 * \param[in]   ptRawData     ָ�� raw_data_t ���͵Ľṹ�������
 * \param[out]  ptCfg         ָ�� request_cfg_t ���ͽṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ���� request_cfg_t ���͵Ľṹ�壬���� MODBUS Э��������
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� request_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ���� response_cfg_t ���͵Ľṹ�壬���� MODBUS Э��������
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� response_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       �Ƚ� request ����� response Ӧ���Ƿ�ƥ��
 * \param[in]   ptReqCfg        ָ�� request_cfg_t �ṹ���ָ��
 * \param[in]   ptRespCfg       ָ�� response_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
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
 * \brief       ��ȫ����һ�� request ����
 * \param[in]   ptRequest        ָ�� request ���ͽṹ���ָ��
 * \return      request����Ŀ���
 *              NULL(����ʧ��)
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
 * \brief       �ڸ��������ʵ������£������ֽ����ݴ���ʱ��
 * \param[in]   wBaudrate        ������
 * \param[in]   hwSendLength     �ֽڷ��ͳ���
 * \return      ����ʱ�䣬��λ����
 */
static uint32_t get_transfer_time(uint32_t wBaudrate, uint16_t hwSendLength)
{
    uint32_t wTransTime = 0;

    /*!
        �Բ����� 9600 Ϊ��׼��1 + 8 + 1�� ���1msһ���ַ���200ms ���Ǵ�����������
        ʱ���Ԥ��ֵ
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
 * \brief       ����������������󣬵��ӻ�����Ӧ��ĵȴ�ʱ�䣨Ԥ��ֵ��
 * \param[in]   ptData        ָ�� raw_data_t ���ͽṹ���ָ��
 * \param[in]   wBaudrate     ������
 * \return      �ȴ�ʱ�䣬��λ����
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
* \brief       �ж�һ�� request ��һ��д����
* \param[in]   ptRequest        ָ�� request ���ͽṹ���ָ��
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
* \brief       ��������������ص�����
* \param[in]   ptInput  ��������������Ϣ������ṹ��
* \param[out]  ptOutput �����ص�������Ϣ������ṹ��
* \return      -1(ʧ��)
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
