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
*       multi_host_recv_send.h *                                     *
*                                                                    *
**********************************************************************
*/

#ifndef MULTI_HOST_RECV_SEND_H
#define MULTI_HOST_RECV_SEND_H

#include "./multi_host_data_type.h"
#include "../../service/critical/critical.h"

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/
extern_simple_fsm(receive_data,
                  def_params(
                      raw_data_t *ptData;
                      uint8_t chCounter;
                      uint8_t chPort;
                      uint8_t chBuffer[RAW_DATA_BUF_SIZE];
                      uint16_t hwSize;
                  ))
extern_fsm_implementation(receive_data);
extern_fsm_initialiser(receive_data);

// 发送单端口数据
extern_simple_fsm(send_port_data,
                  def_params(
                      raw_data_t *ptData;
                      uint8_t chPort;
                  ))
extern_fsm_implementation(send_port_data);
extern_fsm_initialiser(send_port_data, args(uint8_t chPort))

// 发送所有端口的数据
extern_simple_fsm(send_data,
                  def_params(
                      raw_data_t *ptData;
                      fsm(send_port_data) tBuf[TOTAL_PORT_NUM];
                  ))
extern_fsm_implementation(send_data);
extern_fsm_initialiser(send_data);

extern_simple_fsm(forward_raw_data,
                  def_params(raw_data_t *ptData;
                             uint8_t chDstPort;
                             const uint8_t *pchBuf;
                             uint16_t hwSize;
                             critical_sector_t *ptLock;
                            ))
extern_fsm_initialiser(forward_raw_data,
                       args(
                           uint8_t chDstPort,
                           const uint8_t *pchBuf,
                           uint16_t hwSize
))
extern_fsm_implementation(forward_raw_data)

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern void multi_host_recv_send_init(void);

extern raw_data_t *receive_raw_data_by_source_port(uint8_t chPort);

extern void clear_received_raw_data_object(uint8_t chPort);

#endif

/*************************** End of file ****************************/
