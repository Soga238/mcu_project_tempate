#ifndef MULTI_HOST_DATA_TYPE_H
#define MULTI_HOST_DATA_TYPE_H

#include "./multi_host_cfg.h"

typedef struct {
    uint8_t     chPort;
    uint8_t     chBuf[RAW_DATA_BUF_SIZE];
    uint16_t    hwBufSize;
} raw_data_t;                       /*! �����շ��ṹ��     */

typedef struct {
    union {
        uint16_t *phwWR;            /*! �Ĵ�������ָ��      */
        uint8_t  *pchWR;
    };

    uint8_t  chSlave;               /*! �ӻ�վ̨��          */
    uint8_t  chCode;                /*! ������              */
    uint16_t hwDataAddr;            /*! ���ݵ�ַ            */
    uint16_t hwDataNumber;          /*! ���ݶ�ȡ��д�����   */
} request_cfg_t;

typedef struct {
    union {
        uint16_t *phwHR;            /*! �Ĵ�������ָ��      */
        uint8_t  *pchHR;
    };

    uint8_t  chSlave;               /*! �ӻ�վ̨��          */
    uint8_t  chCode;                /*! ������              */

    /*! hwDataAddr �� hwOutputNumber д�����ݷ���ʱ���� */
    uint16_t hwDataAddr;            /*! д���ַ            */
    uint16_t hwOutputNumber;        /*��д������            */
    
    uint8_t  chByteCount;           /*! ʵ���ֽڸ���        */
    uint8_t  chExceptionCode;

} response_cfg_t;


/*! ����ԭʼ����ṹ�� */
typedef struct {
    request_cfg_t tBodyCfg;
    raw_data_t       *ptRaw;        /*! modbusԭʼͨѶ����֡ */

    uint8_t     chPortSrc;
    uint8_t     chPortDstBuf[TOTAL_PORT_NUM - 1];
    uint8_t     chPortDstNum;
    uint8_t     chID;
} proxy_request_t;

/*! ����ԭʼ����ṹ�� */
typedef struct {
    response_cfg_t tBodyCfg;
    raw_data_t       *ptRaw;        /*! modbusԭʼͨѶ����֡ */

    uint8_t     chPortSrc;
    uint8_t     chPortDst;
    uint8_t     chID;
} proxy_response_t;

/**
 *      compare_fn *fnCompare = NULL;
 */

typedef int32_t compare_fn(void *res, void *data);
typedef int32_t travel_fn(void *res, void *data);

typedef int32_t destroy_fn(void *res);
typedef int32_t equal_fn(void *res, void *data);

#endif
