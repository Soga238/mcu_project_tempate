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
 * \brief       ID号转MODBUS从机地址
 * \param[in]   chPort     端口号
 * \param[in]   chId       ID值
 * \param[out]  pchSlave   MODBUS从机地址
 * \return      -1(失败)
 */
extern int8_t id_to_address(uint8_t chPort, uint8_t chId, uint8_t *pchSlave);

/**
 * \brief       计算从主机发送请求，到从机返回应答的等待时间（预估值）
 * \param[in]   ptData        指向 raw_data_t 类型结构体的指针
 * \param[in]   wBaudrate     波特率
 * \return      等待时间，单位毫秒
 */
extern uint32_t calc_wait_response_time(const raw_data_t *ptData, uint32_t wBaudrate);

/**
 * \brief       将从端口接收到的类型为 raw_data_t 的数据，转换成 request 请求格式
 * \param[in]   ptRawData     指向 raw_data_t 类型的结构体的数据
 * \param[out]  ptCfg         指向 request_cfg_t 类型结构体的指针
 * \return      -1(失败)
 */
extern int32_t raw_data_transform_request(raw_data_t *ptRawData, proxy_request_t *ptRequest);

/**
 * \brief       将从端口接收到的类型为 raw_data_t 的数据，转换成 response 请求格式
 * \param[in]   ptRawData     指向 raw_data_t 类型的结构体的数据
 * \param[out]  ptCfg         指向 response_cfg_t 类型结构体的指针
 * \return      -1(失败)
 */
extern int32_t raw_data_transform_response(raw_data_t *ptRawData, proxy_response_t *ptResponse);

/**
 * \brief       完全拷贝一个 request 请求
 * \param[in]   ptRequest        指向 request 类型结构体的指针
 * \return      request请求的拷贝
 *              NULL(拷贝失败)
 */

extern proxy_request_t *proxy_mb_request_copy(const proxy_request_t *ptRequest);

/**
 * \brief       根据 response_cfg_t 类型的结构体，生成 MODBUS 协议体数据
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 response_cfg_t 结构体的指针
 * \return      -1(失败)
 */
extern int32_t make_response_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const response_cfg_t *ptBodyCfg);

/**
 * \brief       根据 request_cfg_t 类型的结构体，生成 MODBUS 协议体数据
 * \param[in]   pchBuffer     指向字节数组的头指针
 * \param[in]   hwBufferSize  字节数组长度
 * \param[out]  ptCfg         指向 request_cfg_t 结构体的指针
 * \return      -1(失败)
 */
extern int32_t make_request_body(uint8_t *pchBuffer, uint16_t hwBufferSize, const request_cfg_t *ptCfg);

/**
 * \brief       比较 request 请求和 response 应答是否匹配
 * \param[in]   ptReqCfg        指向 request_cfg_t 结构体的指针
 * \param[in]   ptRespCfg       指向 response_cfg_t 结构体的指针
 * \return      -1(失败)
 */
extern int32_t is_response_match_request(const request_cfg_t *ptReqfg, const response_cfg_t *ptRespCfg);

/**
* \brief       判断一个 request 是一个写操作
* \param[in]   ptRequest        指向 request 类型结构体的指针
* \return      true
*              false
*/
extern bool is_request_write_operation(const proxy_request_t *ptRequest);

/**
* \brief       计算两个区域的重叠部分
* \param[in]   ptInput  包含两个区域信息的输入结构体
* \param[out]  ptOutput 包含重叠区域信息的输出结构体
* \return      -1(失败)
*/
extern int32_t calc_overlapping_area(const overlapping_area_input_t *ptInput, overlapping_area_output_t *ptOutput);

#endif

/*************************** End of file ****************************/
