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
*       multi_host_utilities.h *                                     *
*                                                                    *
**********************************************************************
*/

#ifndef MULTI_HOST_UTILTIES_H
#define MULTI_HOST_UTILTIES_H

#include "./multi_host_data_type.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct overlapping_area_input {
    int32_t nFirstAreaStart;
    int32_t nFirstAreaSize;
    int32_t nSecondAreaStart;
    int32_t nSecondAreaSize;
} overlapping_area_input_t;

typedef struct overlapping_area_output {
    int32_t nAreaStart;
    int32_t nAreaSize;
} overlapping_area_output_t;


/*********************************************************************
*
*       Macro
*
**********************************************************************
*/

#define CALC_READ_COILS_REQUIRED_BYTES(__COIL_NUMBER)           \
    ((((__COIL_NUMBER) & 0x0007) ? 1 : 0) + ((__COIL_NUMBER) >> 3))

#define CALC_READ_REGISTERS_REQUIRED_BYTES(__REGISTER_NUMBER)   \
    ((__REGISTER_NUMBER) << 1)

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
extern int8_t id_to_address(uint8_t chPort, uint8_t chId, uint8_t *pchSlave);

/**
 * \brief       ����������������󣬵��ӻ�����Ӧ��ĵȴ�ʱ�䣨Ԥ��ֵ��
 * \param[in]   ptData        ָ�� raw_data_t ���ͽṹ���ָ��
 * \param[in]   wBaudrate     ������
 * \return      �ȴ�ʱ�䣬��λ����
 */
extern uint32_t calc_wait_response_time(const raw_data_t *ptData, uint32_t wBaudrate);

/**
 * \brief       ���Ӷ˿ڽ��յ�������Ϊ raw_data_t �����ݣ�ת���� request �����ʽ
 * \param[in]   ptRawData     ָ�� raw_data_t ���͵Ľṹ�������
 * \param[out]  ptCfg         ָ�� request_cfg_t ���ͽṹ���ָ��
 * \return      -1(ʧ��)
 */
extern int32_t raw_data_transform_request(raw_data_t *ptRawData, proxy_request_t *ptRequest);

/**
 * \brief       ���Ӷ˿ڽ��յ�������Ϊ raw_data_t �����ݣ�ת���� response �����ʽ
 * \param[in]   ptRawData     ָ�� raw_data_t ���͵Ľṹ�������
 * \param[out]  ptCfg         ָ�� response_cfg_t ���ͽṹ���ָ��
 * \return      -1(ʧ��)
 */
extern int32_t raw_data_transform_response(raw_data_t *ptRawData, proxy_response_t *ptResponse);

/**
 * \brief       ��ȫ����һ�� request ����
 * \param[in]   ptRequest        ָ�� request ���ͽṹ���ָ��
 * \return      request����Ŀ���
 *              NULL(����ʧ��)
 */

extern proxy_request_t *proxy_mb_request_copy(const proxy_request_t *ptRequest);

/**
 * \brief       ���� response_cfg_t ���͵Ľṹ�壬���� MODBUS Э��������
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� response_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
 */
extern int32_t make_response_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const response_cfg_t *ptBodyCfg);

/**
 * \brief       ���� request_cfg_t ���͵Ľṹ�壬���� MODBUS Э��������
 * \param[in]   pchBuffer     ָ���ֽ������ͷָ��
 * \param[in]   hwBufferSize  �ֽ����鳤��
 * \param[out]  ptCfg         ָ�� request_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
 */
extern int32_t make_request_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const request_cfg_t *ptCfg);

/**
 * \brief       �Ƚ� request ����� response Ӧ���Ƿ�ƥ��
 * \param[in]   ptReqCfg        ָ�� request_cfg_t �ṹ���ָ��
 * \param[in]   ptRespCfg       ָ�� response_cfg_t �ṹ���ָ��
 * \return      -1(ʧ��)
 */
extern int32_t is_response_match_request(const request_cfg_t *ptReqfg, const response_cfg_t *ptRespCfg);

/**
* \brief       �ж�һ�� request ��һ��д����
* \param[in]   ptRequest        ָ�� request ���ͽṹ���ָ��
* \return      true
*              false
*/
extern bool is_request_write_operation(const proxy_request_t *ptRequest);

/**
* \brief       ��������������ص�����
* \param[in]   ptInput  ��������������Ϣ������ṹ��
* \param[out]  ptOutput �����ص�������Ϣ������ṹ��
* \return      -1(ʧ��)
*/
extern int32_t calc_overlapping_area(const overlapping_area_input_t *ptInput, overlapping_area_output_t *ptOutput);

#endif

/*************************** End of file ****************************/
