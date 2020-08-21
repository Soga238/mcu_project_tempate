/****************************************************************************
 * Copyright (c) [2019] [core.zhang@outlook.com]                            *
 * [C modbus] is licensed under Mulan PSL v2.                               *
 * You can use this software according to the terms and conditions of       *
 * the Mulan PSL v2.                                                        *
 * You may obtain a copy of Mulan PSL v2 at:                                *
 *          http://license.coscl.org.cn/MulanPSL2                           *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF     *
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO        *
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.       *
 * See the Mulan PSL v2 for more details.                                   *
 *                                                                          *
 ***************************************************************************/

/* Includes --------------------------------------------------------*/
#include "./modbus.h"
#include "../../../api/include/iot_os.h"

/* Global variables ------------------------------------------------*/
/* Private typedef -------------------------------------------------*/
/* Private define --------------------------------------------------*/
#define MB_SER_ADU_SIZE_MIN             (6)        /*! RTU串行帧的最小长度 */
#define MB_SER_ADU_SIZE_MAX             (256)      /*! RTU串行帧的最大长度 */

// (PDU_SIZE + 3) = ADU_SIZE
#define MB_SER_PDU_SIZE_MIN             (3)        /*! RTU串行帧的最小长度 */
#define MB_SER_PDU_SIZE_MAX             (253)      /*! RTU串行帧的最大长度 */

#define EV_MASTER_NONE                  (0)
#define EV_MASTER_RECV_TIMEOUT          (1 << 0)
#define EV_MASTER_RECV_CPL              (1 << 1)
#define EV_MASTER_ERROR                 (1 << 2)
#define EV_MASTER_START_SEND            (1 << 3)

/* Private macro ---------------------------------------------------*/
#define CALC_MODBUS_CRC16(ptr, len)     MODBUS_CRC16(ptr, len)

#ifndef FALL_THROUGH
    #define FALL_THROUGH()
#endif

/* Private variables -----------------------------------------------*/
/* Private function prototypes -------------------------------------*/
/* Private functions -----------------------------------------------*/

static uint32_t __get_tick_1ms(void)
{
    // Note: 以5ms为单位
    return iot_os_get_system_tick() * 5;
}

// 非 KEIL环境

#ifndef WEAK
extern void port_hold_register_cb(const mb_master_t *ptMaster,
                                  const mb_request_t *ptRequest,
                                  const mb_response_t *ptResponse);

extern void port_coil_cb(const mb_master_t *ptMaster,
                         const mb_request_t *ptRequest,
                         const mb_response_t *ptResponse);

extern void port_error_cb(const mb_master_t *ptMaster,
                          const mb_request_t *ptRequest,
                          const mb_response_t *ptResponse);
#else
WEAK void port_hold_register_cb(const mb_master_t *ptMaster,
                                const mb_request_t *ptRequest,
                                const mb_response_t *ptResponse)
{
}

WEAK void port_coil_cb(const mb_master_t *ptMaster,
                       const mb_request_t *ptRequest,
                       const mb_response_t *ptResponse)
{
}

WEAK void port_error_cb(const mb_master_t *ptMaster,
                        const mb_request_t *ptRequest,
                        const mb_response_t *ptResponse)
{
}
#endif

static void master_handle_callback(const mb_master_t *ptMaster,
                                   const mb_request_t *ptRequest,
                                   const mb_response_t *ptResponse)
{
    switch (ptRequest->chCode) {
        case MB_CODE_WRITE_REGISTER:
        case MB_CODE_WRITE_MULTIPLE_REGISTERS:
        case MB_CODE_READ_INPUT_REGISTERS:
        case MB_CODE_READ_HOLDING_REGISTERS:
            port_hold_register_cb(ptMaster, ptRequest, ptResponse);
            break;

        case MB_CODE_WRITE_COIL:
        case MB_CODE_READ_DISCRETE_INPUTS:
        case MB_CODE_READ_COILS:
            port_coil_cb(ptMaster, ptRequest, ptResponse);
            break;

        default:
            return;
    }
}

static bool mb_post_event(mb_control_t *ptCtl, uint8_t tEvent)
{
    ptCtl->wEvent |= tEvent;
    return true;
}

static bool mb_wait_event(mb_control_t *ptCtl, uint8_t tEvent)
{
    if (ptCtl->wEvent & tEvent) {
        ptCtl->wEvent &= ~tEvent;
        return true;
    }
    return false;
}

static uint16_t master_send_request(uint8_t *pchBuf,
                                    const mb_request_t *ptRequest)
{
    uint16_t hwFrameLength;
    uint16_t hwCRC;

    pchBuf[0] = ptRequest->chSlaveNum;
    pchBuf[1] = ptRequest->chCode;
    pchBuf[2] = ptRequest->hwDataAddr >> 8;
    pchBuf[3] = ptRequest->hwDataAddr;
    pchBuf[4] = ptRequest->hwDataNum >> 8;
    pchBuf[5] = ptRequest->hwDataNum;

    switch (ptRequest->chCode) {
        // Fixed length
        case MB_CODE_READ_COILS:
        case MB_CODE_READ_DISCRETE_INPUTS:
        case MB_CODE_READ_HOLDING_REGISTERS:
        case MB_CODE_READ_INPUT_REGISTERS:
            hwFrameLength = 6;
            break;

        case MB_CODE_WRITE_REGISTER:
            short_copy_xch(&pchBuf[4],
                           ptRequest->phwWR,
                           ptRequest->hwDataNum,
                           true);
            hwFrameLength = 4 + (ptRequest->hwDataNum << 1);
            break;

        case MB_CODE_WRITE_MULTIPLE_REGISTERS:
            pchBuf[6] = ptRequest->hwDataNum << 1;
            short_copy_xch(&pchBuf[7],
                           ptRequest->phwWR,
                           ptRequest->hwDataNum,
                           true);
            hwFrameLength = 4 + 3 + pchBuf[6];
            break;

        case MB_CODE_WRITE_COIL:
            short_copy_xch(&pchBuf[4],
                           ptRequest->phwWR,
                           ptRequest->hwDataNum,
                           true);
            hwFrameLength = 4 + (ptRequest->hwDataNum << 1);
            break;

        default:
            return 0;
    }

    hwCRC = CALC_MODBUS_CRC16(pchBuf, hwFrameLength);
    pchBuf[hwFrameLength++] = hwCRC & 0x00FF;
    pchBuf[hwFrameLength++] = (hwCRC & 0xFF00) >> 8;

    return hwFrameLength;
}

static bool master_recv_response(uint8_t *pchBuf,
                                 uint16_t hwLength,
                                 mb_response_t *ptResponse)
{
    if (NULL == pchBuf || NULL == ptResponse) {
        return false;
    }

    if (hwLength < MB_SER_ADU_SIZE_MIN ||
        hwLength > MB_SER_ADU_SIZE_MAX) {
        return false;
    }

    if (!check_crc16(pchBuf, hwLength)) {
        return false;
    }

    ptResponse->chSlaveNum = pchBuf[0];
    ptResponse->chCode = pchBuf[1];

    switch (ptResponse->chCode) {
        case MB_CODE_READ_DISCRETE_INPUTS:
        case MB_CODE_READ_COILS:
        case MB_CODE_READ_INPUT_REGISTERS:
        case MB_CODE_READ_HOLDING_REGISTERS:
            ptResponse->hwByteCount = pchBuf[2];
            ptResponse->pchRegStart = &pchBuf[3];
            break;

        case MB_CODE_WRITE_COIL:
        case MB_CODE_WRITE_REGISTER:
            ptResponse->hwDataAddr = CHAR_HL_SHORT(pchBuf[2], pchBuf[3]);
            ptResponse->hwDataNum = 1;
            break;

        case MB_CODE_WRITE_MULTIPLE_REGISTERS:
            ptResponse->hwDataAddr = CHAR_HL_SHORT(pchBuf[2], pchBuf[3]);
            ptResponse->hwDataNum = CHAR_HL_SHORT(pchBuf[4], pchBuf[5]);
            ptResponse->hwByteCount = pchBuf[6];
            break;

        case (0x80 + MB_CODE_READ_INPUT_REGISTERS):
        case (0x80 + MB_CODE_READ_HOLDING_REGISTERS):
        case (0x80 + MB_CODE_READ_DISCRETE_INPUTS):
        case (0x80 + MB_CODE_READ_COILS):
        case (0x80 + MB_CODE_WRITE_COIL):
        case (0x80 + MB_CODE_WRITE_REGISTER):
        default:
            return false;
    }

    return true;
}

static bool master_do_action(mb_master_t *ptMaster, uint32_t wTimeout)
{
    uint32_t hwSndLength = 0;
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_SLAVE_ADDREESS_MIN > ptRequest->chSlaveNum ||
        MB_SLAVE_ADDREESS_MAX < ptRequest->chSlaveNum) {
        return false;
    }

    if ((ptMaster->tSerialCtl.hwRcvBufSize < MB_SER_ADU_SIZE_MAX)
        || (ptMaster->tSerialCtl.hwSndBufSize < MB_SER_ADU_SIZE_MAX)) {
        return false;
    }

    switch (ptRequest->chCode) {
        case MB_CODE_WRITE_REGISTER:
        case MB_CODE_WRITE_MULTIPLE_REGISTERS:
        case MB_CODE_READ_INPUT_REGISTERS:
        case MB_CODE_READ_HOLDING_REGISTERS:
            if (MB_READ_REGCNT_MIN > ptRequest->hwDataNum ||
                MB_READ_REGCNT_MAX < ptRequest->hwDataNum) {
                return false;
            }
            break;

        case MB_CODE_WRITE_COIL:
        case MB_CODE_READ_COILS:
        case MB_CODE_READ_DISCRETE_INPUTS:
            if (MB_READ_COILCNT_MIN > ptRequest->hwDataNum ||
                MB_READ_COILCNT_MAX < ptRequest->hwDataNum) {
                return false;
            }
            break;
        default:
            return false;
    }

    hwSndLength =
        master_send_request(ptMaster->tSerialCtl.pSndBuf, &ptMaster->tRequest);
    if (hwSndLength && hwSndLength <= ptMaster->tSerialCtl.hwSndBufSize) {
        ptMaster->tSerialCtl.hwSndLen = hwSndLength;
        ptMaster->tRequest.wTimeout = wTimeout;
        return true;
    }

    return false;
}

static int32_t recv_timeout_callback(void *arg)
{
    mb_master_t *ptMaster = (mb_master_t *) arg;
    soft_timer_delete(ptMaster->tRequest.ptTimer);
    ptMaster->tRequest.ptTimer = NULL;
    mb_post_event(ptMaster, EV_MASTER_RECV_TIMEOUT);
    return 0;
}

static void master_poll_in_block(mb_master_t *ptMaster)
{
    enum work_state {
        START = 0,
        WAIT_EVENT,
        SEND_REQUEST,
        WAIT_RESPONSE,
        HANDLE_RESPONSE,
        HANDLE_ERROR,
    };

    switch (ptMaster->chState) {
        case START:
            ptMaster->tStatus = MB_STATUE_IDLE;

            FALL_THROUGH();
        case WAIT_EVENT:
            ptMaster->chState = WAIT_EVENT;
            if (!mb_wait_event(ptMaster, EV_MASTER_START_SEND)) {
                break;
            }

            FALL_THROUGH();
        case SEND_REQUEST:
            ptMaster->chState = SEND_REQUEST;
            if (!ptMaster->tSerialCtl.fnSend(ptMaster->tSerialCtl.pSndBuf,
                                             ptMaster->tSerialCtl.hwSndLen)) {
                ptMaster->chState = HANDLE_ERROR;
                break;
            }

            FALL_THROUGH();
        case WAIT_RESPONSE:
            ptMaster->chState = WAIT_RESPONSE;

            /*! read in block mode*/
            ptMaster->tSerialCtl.hwRcvLen = \
                ptMaster->tSerialCtl.fnRecv(ptMaster->tSerialCtl.pRcvBuf,
                                            ptMaster->tSerialCtl.hwRcvBufSize);

            if (ptMaster->tSerialCtl.hwRcvLen <= 0) {
                ptMaster->chState = HANDLE_ERROR;
                break;
            }

            FALL_THROUGH();
        case HANDLE_RESPONSE:
            ptMaster->chState = HANDLE_RESPONSE;
            if (master_recv_response(ptMaster->tSerialCtl.pRcvBuf,
                                     ptMaster->tSerialCtl.hwRcvLen,
                                     &ptMaster->tResponse)) {
                master_handle_callback(ptMaster, &ptMaster->tRequest,
                                       &ptMaster->tResponse);
                ptMaster->chState = START;
                break;
            }

            FALL_THROUGH();
        default:
        case HANDLE_ERROR:
            ptMaster->chState = HANDLE_ERROR;
            port_error_cb(ptMaster, &ptMaster->tRequest, &ptMaster->tResponse);
            ptMaster->chState = START;
            break;
    }
}

static void master_poll_in_noblock(mb_master_t *ptMaster)
{
    enum work_state {
        START = 0,
        WAIT_EVENT,
        SEND_REQUEST,
        WAIT_RESPONSE,
        HANDLE_RESPONSE,
        HANDLE_ERROR,
    };

    switch (ptMaster->chState) {
        case START:
            ptMaster->tStatus = MB_STATUE_IDLE;

            FALL_THROUGH();
        case WAIT_EVENT:
            ptMaster->chState = WAIT_EVENT;
            if (!mb_wait_event(ptMaster, EV_MASTER_START_SEND)) {
                break;
            }

            FALL_THROUGH();
        case SEND_REQUEST:
            ptMaster->chState = SEND_REQUEST;
            ptMaster->tRequest.ptTimer = \
                soft_timer_create(ptMaster->tRequest.wTimeout, false,
                                  recv_timeout_callback,
                                  ptMaster);

            if (NULL == ptMaster->tRequest.ptTimer) {
                ptMaster->chState = HANDLE_ERROR;
                break;
            }

            if (!ptMaster->tSerialCtl.fnSend(ptMaster->tSerialCtl.pSndBuf,
                                             ptMaster->tSerialCtl.hwSndLen)) {
                soft_timer_delete(ptMaster->tRequest.ptTimer);
                ptMaster->chState = HANDLE_ERROR;
                break;
            }

            FALL_THROUGH();
        case WAIT_RESPONSE:
            ptMaster->chState = WAIT_RESPONSE;
            if (mb_wait_event(ptMaster, EV_MASTER_RECV_CPL)) {
                soft_timer_delete(ptMaster->tRequest.ptTimer);
            } else {

                /*! read in poll mode*/
                ptMaster->tSerialCtl.hwRcvLen =
                    ptMaster->tSerialCtl.fnRecv(ptMaster->tSerialCtl.pRcvBuf,
                                                ptMaster->tSerialCtl.hwRcvBufSize);
                if (0 < ptMaster->tSerialCtl.hwRcvLen) {
                    soft_timer_delete(ptMaster->tRequest.ptTimer);
                    ptMaster->chState = HANDLE_RESPONSE;
                } else if (mb_wait_event(ptMaster, EV_MASTER_RECV_TIMEOUT)) {
                    ptMaster->chState = HANDLE_ERROR;
                    break;
                } else {
                    break;
                }
            }

            FALL_THROUGH();
        case HANDLE_RESPONSE:
            if (master_recv_response(ptMaster->tSerialCtl.pRcvBuf,
                                     ptMaster->tSerialCtl.hwRcvLen,
                                     &ptMaster->tResponse)) {

                master_handle_callback(ptMaster, &ptMaster->tRequest,
                                       &ptMaster->tResponse);
                ptMaster->chState = START;
                break;
            }

            FALL_THROUGH();
        default:
        case HANDLE_ERROR:
            ptMaster->chState = HANDLE_ERROR;
            port_error_cb(ptMaster, &ptMaster->tRequest, &ptMaster->tResponse);
            ptMaster->chState = START;
            break;
    }
}

void mb_master_poll(mb_master_t *ptMaster)
{
    if (NULL != ptMaster && ptMaster->bInitOk) {
        if (ptMaster->bIsRecvBlock) {
            // master_poll_in_block(ptMaster);
        } else {
            soft_timer_process();
            master_poll_in_noblock(ptMaster);
        }
    }
}

bool mb_control_init(mb_control_t *ptCtl, serial_ctl_t *ptSerialConfig,
                     uint8_t chRecvBlock)
{
    enum { START = 0, };

    if ((NULL == ptCtl) || (NULL == ptSerialConfig)) {
        return false;
    }

    if ((0 == ptSerialConfig->hwRcvBufSize)
        || (0 == ptSerialConfig->hwSndBufSize)) {
        return false;
    }

    if ((NULL == ptSerialConfig->fnRecv) ||
        (NULL == ptSerialConfig->fnSend)) {
        return false;
    }

    if ((NULL == ptSerialConfig->pRcvBuf)
        || NULL == (ptSerialConfig->pSndBuf)) {
        return false;
    }

    ptCtl->chState = START;
    ptCtl->wEvent = EV_MASTER_NONE;

    ptCtl->tSerialCtl = *ptSerialConfig;
    ptCtl->tSerialCtl.hwRcvLen = 0;
    ptCtl->tSerialCtl.hwSndLen = 0;

    ptCtl->bIsRecvBlock = (chRecvBlock != 0);
    ptCtl->bInitOk = 1;

    soft_timer_init(__get_tick_1ms, MAX_VALUE_32_BIT);

    return true;
}

bool mb_control_is_idle(mb_control_t *ptCtl)
{
    if (NULL != ptCtl) {
        return ptCtl->tStatus == MB_STATUE_IDLE;
    }
    return false;
}

int8_t mb_read_hold_register(mb_master_t *ptMaster,
                             uint8_t chSlaveNumber,
                             uint16_t hwDataAddr,
                             uint16_t hwDataNumber,
                             uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = hwDataNumber;
    ptRequest->chCode = MB_CODE_READ_HOLDING_REGISTERS;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_read_input_register(mb_master_t *ptMaster,
                              uint8_t chSlaveNumber,
                              uint16_t hwDataAddr,
                              uint16_t hwDataNumber,
                              uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = hwDataNumber;
    ptRequest->chCode = MB_CODE_READ_INPUT_REGISTERS;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_read_discrete_inputs(mb_master_t *ptMaster,
                               uint8_t chSlaveNumber,
                               uint16_t hwDataAddr,
                               uint16_t hwDataNumber,
                               uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = hwDataNumber;
    ptRequest->chCode = MB_CODE_READ_DISCRETE_INPUTS;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_read_coils(mb_master_t *ptMaster,
                     uint8_t chSlaveNumber,
                     uint16_t hwDataAddr,
                     uint16_t hwDataNumber,
                     uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = hwDataNumber;
    ptRequest->chCode = MB_CODE_READ_COILS;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_write_hold_register(mb_master_t *ptMaster,
                              uint8_t chSlaveNumber,
                              uint16_t hwDataAddr,
                              uint16_t hwValue,
                              uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = 1;
    ptRequest->chCode = MB_CODE_WRITE_REGISTER;

    ptRequest->hwValue = hwValue;
    ptRequest->phwWR = &ptRequest->hwValue;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_write_hold_multi_register(mb_master_t *ptMaster,
                                    uint8_t chSlaveNumber,
                                    uint16_t hwDataAddr,
                                    const uint16_t *phwBuf,
                                    uint16_t hwDataNumber,
                                    uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataAddr = hwDataAddr;
    ptRequest->hwDataNum = hwDataNumber;
    ptRequest->chCode = MB_CODE_WRITE_MULTIPLE_REGISTERS;
    ptRequest->phwWR = phwBuf;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_write_single_coil(mb_master_t *ptMaster,
                            uint8_t chSlaveNumber,
                            uint16_t hwCoilAddr,
                            uint8_t chSwitch,
                            uint32_t wTimeout)
{
    mb_request_t *ptRequest = (mb_request_t *) &ptMaster->tRequest;

    if (MB_STATUE_IDLE != ptMaster->tStatus) {
        return 0;
    }

    ptRequest->chSlaveNum = chSlaveNumber;
    ptRequest->hwDataNum = 1;
    ptRequest->chCode = MB_CODE_WRITE_COIL;
    ptRequest->hwDataAddr = hwCoilAddr;
    ptRequest->hwValue = (chSwitch == 0 ? 0x0000 : 0xFF00);
    ptRequest->phwWR = &ptRequest->hwValue;

    if (master_do_action(ptMaster, wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

int8_t mb_do_request(mb_master_t *ptMaster, const mb_request_t *ptRequest)
{
    if ((NULL == ptMaster) || (NULL == ptRequest)) {
        return 0;
    }

    ptMaster->tRequest = *ptRequest;

    if (master_do_action(ptMaster, ptRequest->wTimeout)) {
        ptMaster->tStatus = MB_STATUS_BUSY;
        mb_post_event(ptMaster, EV_MASTER_START_SEND);
        return 1;
    } else {
        mb_post_event(ptMaster, EV_MASTER_ERROR);
    }

    return 0;
}

// -------------------------------------------------------------------------
//                              从机
// -------------------------------------------------------------------------

#ifndef WEAK
void port_slave_cb(mb_slave_t *ptSlave,
                   mb_request_t *ptRequest,
                   mb_response_t *ptResponse);

bool port_slave_filter_cb(const mb_slave_t *ptSlave, uint8_t chSlaveNumber);

void port_slave_error_cb(mb_slave_t *ptSlave,
                         mb_request_t *ptRequest,
                         mb_response_t *ptResponse);
#else
WEAK void port_slave_cb(mb_slave_t *ptSlave,
                        mb_request_t *ptRequest,
                        mb_response_t *ptResponse)
{
}

WEAK bool port_slave_filter_cb(const mb_slave_t *ptSlave, uint8_t chSlaveNumber)
{
    return true;
}

WEAK void port_slave_error_cb(mb_slave_t *ptSlave,
                              mb_request_t *ptRequest,
                              mb_response_t *ptResponse)
{
}

#endif

static bool slave_rcv_handle(uint8_t *pchBuf,
                             uint16_t hwLength,
                             mb_request_t *ptRequest)
{
    if (hwLength < MB_SER_ADU_SIZE_MIN || hwLength > MB_SER_ADU_SIZE_MAX) {
        return false;
    }

    if (!check_crc16(pchBuf, hwLength)) {
        return false;
    }

    ptRequest->chSlaveNum = pchBuf[0];
    ptRequest->chCode = pchBuf[1];

    switch (ptRequest->chCode) {
        case MB_CODE_READ_INPUT_REGISTERS:
            ptRequest->hwDataAddr = ((pchBuf[2] & 0x00FF) << 8) + pchBuf[3];
            ptRequest->hwDataNum = ((pchBuf[4] & 0x00FF) << 8) + pchBuf[5];
            break;

        default:
            break;
    }

    return true;
}

static bool slave_handle_request(mb_request_t *ptRequest,
                                 mb_response_t *ptResponse)
{
    switch (ptRequest->chCode) {
        case MB_CODE_READ_INPUT_REGISTERS:
            break;

        default:
            ptResponse->chCode = MB_EX_ILLEGAL_FUNCTION;
            return false;
    }

    switch (ptRequest->chCode) {
        default:
        case MB_CODE_READ_INPUT_REGISTERS:
            if ((MB_READ_REGCNT_MIN > ptRequest->hwDataNum) ||
                (MB_READ_REGCNT_MAX < ptRequest->hwDataNum)) {
                ptResponse->chCode = MB_EX_ILLEGAL_DATA_VALUE;
                return false;
            }
            break;
    }

    switch (ptRequest->chCode) {
        default:
        case MB_CODE_READ_INPUT_REGISTERS:
            if ((MB_DATA_START_ADDRESS > ptRequest->hwDataAddr) ||
                (MB_DATA_END_ADDRESS
                    < (ptRequest->hwDataAddr + ptRequest->hwDataNum))) {
                ptResponse->chCode = MB_EX_ILLEGAL_DATA_ADDRESS;
                return false;
            }
            break;
    }

    return true;
}

void mb_slave_push(mb_slave_t *ptSlave)
{
    uint16_t hwByteLength = 0;
    enum { START = 0, CHECK_REQUEST, HANDLE_REQUEST };

    if (NULL == ptSlave) {
        return;
    }

    switch (ptSlave->chState) {
        case START:
            ptSlave->tStatus = MB_STATUE_IDLE;
            ptSlave->chState = CHECK_REQUEST;

            FALL_THROUGH();
        case CHECK_REQUEST:
            hwByteLength =
                ptSlave->tSerialCtl.fnRecv(ptSlave->tSerialCtl.pRcvBuf,
                                           ptSlave->tSerialCtl.hwRcvBufSize);
            if (0 == hwByteLength) {
                break;
            }

            ptSlave->tStatus = MB_STATUS_BUSY;

            if (!port_slave_filter_cb(ptSlave,
                                      ptSlave->tSerialCtl.pRcvBuf[0])) {
                ptSlave->chState = START;
                break;
            }

            if (slave_rcv_handle(ptSlave->tSerialCtl.pRcvBuf, hwByteLength,
                                 &ptSlave->tRequest)) {
                ptSlave->chState = HANDLE_REQUEST;
            } else {
                ptSlave->chState = START;
                break;
            }

            FALL_THROUGH();
        case HANDLE_REQUEST:
            if (slave_handle_request(&ptSlave->tRequest, &ptSlave->tResponse)) {
                port_slave_cb(ptSlave, &ptSlave->tRequest, &ptSlave->tResponse);
            } else {
                port_slave_error_cb(ptSlave, &ptSlave->tRequest,
                                    &ptSlave->tResponse);
            }
            ptSlave->chState = START;
            break;

        default:
            ptSlave->chState = START;
            break;
    }
}

/*************************** End of file ****************************/
