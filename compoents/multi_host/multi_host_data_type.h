#ifndef MULTI_HOST_DATA_TYPE_H
#define MULTI_HOST_DATA_TYPE_H

#include "./multi_host_cfg.h"

typedef struct {
    uint8_t     chPort;
    uint8_t     chBuf[RAW_DATA_BUF_SIZE];
    uint16_t    hwBufSize;
} raw_data_t;                       /*! 串口收发结构体     */

typedef struct {
    union {
        uint16_t *phwWR;            /*! 寄存器数据指针      */
        uint8_t  *pchWR;
    };

    uint8_t  chSlave;               /*! 从机站台号          */
    uint8_t  chCode;                /*! 功能码              */
    uint16_t hwDataAddr;            /*! 数据地址            */
    uint16_t hwDataNumber;          /*! 数据读取或写入个数   */
} request_cfg_t;

typedef struct {
    union {
        uint16_t *phwHR;            /*! 寄存器数据指针      */
        uint8_t  *pchHR;
    };

    uint8_t  chSlave;               /*! 从机站台号          */
    uint8_t  chCode;                /*! 功能码              */

    /*! hwDataAddr 和 hwOutputNumber 写入数据返回时可用 */
    uint16_t hwDataAddr;            /*! 写入地址            */
    uint16_t hwOutputNumber;        /*！写入数量            */
    
    uint8_t  chByteCount;           /*! 实际字节个数        */
    uint8_t  chExceptionCode;

} response_cfg_t;


/*! 代理原始请求结构体 */
typedef struct {
    request_cfg_t tBodyCfg;
    raw_data_t       *ptRaw;        /*! modbus原始通讯数据帧 */

    uint8_t     chPortSrc;
    uint8_t     chPortDstBuf[TOTAL_PORT_NUM - 1];
    uint8_t     chPortDstNum;
    uint8_t     chID;
} proxy_request_t;

/*! 代理原始请求结构体 */
typedef struct {
    response_cfg_t tBodyCfg;
    raw_data_t       *ptRaw;        /*! modbus原始通讯数据帧 */

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
