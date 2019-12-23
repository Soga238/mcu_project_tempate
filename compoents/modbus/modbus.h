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
*       modbus.h *                                                   *
*                                                                    *
**********************************************************************
*/
#ifndef __MODBUS_H__
#define __MODBUS_H__
#include "..\compoents_cfg.h"
#include "..\..\service\soft_timer\soft_timer.h"

#define MB_SLAVE_ADDREESS_MIN                   0x01    /*! �ӻ���ַ��Сֵ       */
#define MB_SLAVE_ADDREESS_MAX                   0xF7    /*! �ӻ���ַ���ֵ       */

/*
*********************************************************************************
*                               MODBUS ������
*********************************************************************************
*/
#define MB_FUNC_NONE                            0x00    /*! δ���幦����         */
#define MB_FUNC_READ_COILS                      0x01    /*! ��ȡ�����Ȧ         */
#define MB_FUNC_READ_DISCRETE_INPUTS            0x02    /*! ��ȡ���������Ȧ*/
#define MB_FUNC_READ_HOLDING_REGISTERS          0x03    /*! ����/������ּĴ���  */
#define MB_FUNC_READ_INPUT_REGISTERS            0x04    /*! ����/�������Ĵ���  */
#define MB_FUNC_WRITE_SINGLE_COIL               0x05    /*! ǿ�Ƶ�����Ȧ         */
#define MB_FUNC_WRITE_REGISTER                  0x06    /*! д�������ּĴ���     */
#define MB_FUNC_WRITE_MULTIPLE_COILS            0x0F    /*! д�����Ȧ�Ĵ���     */
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS        0x10    /*! д������ּĴ���     */
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS_RDHR   0x17    /*! ��/д������ּĴ���  */
/*
*********************************************************************************
*                               MODBUS ��׼�쳣
*********************************************************************************
*/
#define MB_EX_NONE                              0x00    /*! ���쳣               */
#define MB_EX_ILLEGAL_FUNCTION                  0x01    /*! �Ƿ�����             */
#define MB_EX_ILLEGAL_DATA_ADDRESS              0x02    /*! �Ƿ����ݵ�ַ         */
#define MB_EX_ILLEGAL_DATA_VALUE                0x03    /*! �Ƿ�����ֵ           */
#define MB_EX_SLAVE_DEVICE_FAILURE              0x04    /*! �ӻ��豸����         */
#define MB_EX_ACKNOWLEDGE                       0x05    /*! ȷ��                 */
#define MB_EX_SLAVE_BUSY                        0x06    /*! �ӻ���æ             */
#define MB_EX_MEMORY_PARITY_ERROR               0x08    /*! �ڴ���żУ�����     */
#define MB_EX_GATEWAY_PATH_FAILED               0x0A    /*! ����������·��       */
#define MB_EX_GATEWAY_TGT_FAILED                0x0B    /*! ����Ŀ���豸��Ӧʧ�� */

/*
*********************************************************************************
*                               MODBUS ���ú�
*********************************************************************************
*/
#define COIL_VALUE_ON                   0xFF00
#define COIL_VALUE_OFF                  0x0000

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef uint16_t (*pSerialFun)(uint8_t *, uint16_t);     /*! ���ں���ָ��        */
typedef uint16_t (*pDmaFun)(uint16_t);                   /*! DMAʹ�ܺ���ָ��     */

typedef void (*pfSerialIO)(uint8_t Sw);                  /*! �ӿڲ�������ָ��    */

typedef struct serial_ctl serial_ctl_t;
struct serial_ctl {
    uint8_t    *pSndBuf;            /*! ���ݽ��ջ�����      */
    uint8_t    *pRcvBuf;            /*! ���ݷ��ͻ�����      */

    uint16_t    hwSndBufSize;       /*! ���ݽ��ջ�������    */
    uint16_t    hwRcvBufSize;       /*! ���ݷ��ͻ�������    */

    uint16_t    hwSndLen;           /*! �������ݳ���        */
    uint16_t    hwRcvLen;           /*! �ѽ������ݳ���      */

    pSerialFun  fnSend;             /*! ���ڷ��ͺ���ָ��    */
    pSerialFun  fnRecv;             /*! ���ڽ��պ���ָ��    */
};

/**
 *  \brief modbus����ṹ�壬����������ȡ��һ��
 */
typedef struct mb_request {
    uint32_t wTimeout;              /*! ���ڽ��ճ�ʱʱ��    */
    timer_table_t *ptTimer;         /*! ��ʱ���ṹ��ָ��  */

    uint16_t *phwHR;                /*! ���ڽ��պ���ָ��    */
    uint16_t *phwWR;                /*! ���ڽ��պ���ָ��    */

    /*!! ����д�뵥������ʱ��ŵ����� */
    uint16_t hwValue;

    uint8_t  chSlaveNumber;         /*! �ӻ�վ̨��          */
    uint8_t  chCode;                /*! ������              */
    uint16_t hwDataAddr;            /*! ���ݵ�ַ            */
    uint16_t hwDataNumber;          /*! ���ݶ�ȡ��д�����  */
} mb_request_t;

/**
 *  \brief modbusӦ��ṹ�壬���ڱ�����ȡ��һ��
 */
typedef struct mb_response {
    uint8_t chSlaveNumber;          /*! �ӻ�վ̨��          */
    uint8_t chCode;                 /*! ������              */

    uint16_t hwByteCount;           /*! �ֽڸ���            */
    uint16_t hwDataAddr;            /*! ���ݵ�ַ            */
    uint16_t hwDataNumber;          /*! ���ݶ�ȡ��д�����  */

    uint8_t *pchRegStart;           /*! Ӧ�����ݻ�����ͷָ��*/
} mb_response_t;

typedef enum {
    MB_STATUE_IDLE = 0,
    MB_STATUS_BUSY = 1
} mb_eu_state_t;

typedef struct {
    mb_request_t tRequest;          /*! modbus����ṹ��    */
    mb_response_t tResponse;        /*! modbusӦ��ṹ��    */
    serial_ctl_t tSerialCtl;        /*! ���ڲ����ṹ��      */

    uint8_t chState;                /*! �ڲ�״̬������      */
    mb_eu_state_t tStatus;

    uint8_t chRecvBlock;            /*! �Ƿ��������        */
    
    uint32_t wEvent;                /*! �ڲ��¼�����        */
    
    /*�� uint8_t chErrorCode; */
} mb_control_t, mb_master_t, mb_slave_t;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

extern bool mb_control_init(mb_control_t *ptCtl, serial_ctl_t *ptSerialConfig, uint8_t chRecvBlock);

extern bool mb_control_is_idle(mb_control_t *ptCtl);

#define mb_master_init      mb_control_init
#define mb_master_is_idle   mb_control_is_idle

extern void mb_master_poll(mb_master_t *ptMaster);

extern int8_t mb_read_hold_register(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwDataAddr, uint16_t hwDataNumber, uint32_t wTimeout);

extern int8_t mb_write_hold_register(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwDataAddr, uint16_t hwValue, uint32_t wTimeout);

extern int8_t mb_write_hold_multipe_register(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwDataAddr, uint16_t *phwBuf, uint16_t hwDataNumber, uint32_t wTimeout);

extern int8_t mb_write_single_coil(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwCoilAddr, uint8_t chSwitch, uint32_t wTimeout);

extern int8_t mb_read_input_register(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwDataAddr, uint16_t hwDataNumber, uint32_t wTimeout);

extern int8_t mb_read_coils(mb_master_t *ptMaster, uint8_t chSlaveNumber, uint16_t hwDataAddr, uint16_t hwDataNumber, uint32_t wTimeout);

#define mb_slave_init       mb_control_init
#define mb_slave_is_idle    mb_control_is_idle

extern void mb_slave_push(mb_slave_t *ptSlave);

#endif
/* end of file*/
