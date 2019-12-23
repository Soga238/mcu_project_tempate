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
*       multi_host_request_response.c *                              *
*                                                                    *
**********************************************************************
*/

#ifndef MULTI_HOST_REQUEST_H
#define MULTI_HOST_REQUEST_H

#include "./multi_host_recv_send.h"
#include "../../service/critical/critical.h"

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/

extern_simple_fsm(transform_request,
                  def_params(
                      raw_data_t *ptData;
                      proxy_request_t *ptRequest;
                  ))
extern_fsm_initialiser(transform_request)
extern_fsm_implementation(transform_request, args(uint8_t chPort))

extern_simple_fsm(transform_response,
                  def_params(
                      raw_data_t *ptData;
                      proxy_response_t *ptResponse;
                  ))
extern_fsm_initialiser(transform_response)
extern_fsm_implementation(transform_response, args(uint8_t chPort))

extern_simple_fsm(read_and_write_operation,
                  def_params(
                      uint8_t chPort;
                      proxy_request_t *ptRequest;
                      uint16_t hwCounter;
                      raw_data_t *ptRaw;
                      fsm(forward_raw_data) tForwardRawData;
                      uint32_t wBaudrate;
                      uint32_t wTimeout;
                  ))

extern_fsm_initialiser(read_and_write_operation, args(proxy_request_t *ptRequest))
extern_fsm_implementation(read_and_write_operation, args(proxy_response_t **pptResponse))

extern_simple_fsm(safe_read_and_write_operation,
                  def_params(
                      fsm(read_and_write_operation) tOperation;
                      critical_sector_t *ptLock;
                  ))
extern_fsm_initialiser(safe_read_and_write_operation, args(proxy_request_t *ptRequest))
extern_fsm_implementation(safe_read_and_write_operation, args(proxy_response_t **pptResponse))

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
extern void multi_host_request_response_init(void);

extern proxy_request_t *receive_request(void);

#endif

/*************************** End of file ****************************/
