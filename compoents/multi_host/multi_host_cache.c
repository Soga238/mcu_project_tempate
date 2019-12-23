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
*       multi_host_cache.c *                                         *
*                                                                    *
**********************************************************************
*/

#include "./multi_host_cache.h"
#include "./cache_data_list.h"
#include "./multi_host_port.h"
#include "./multi_host_cache_utilties.h"

#include "../../compoents/modbus/modbus.h"
#include "../../service/heap/heap.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#ifndef MAXMIUM_CACHE_BUF_SIZE
    #define MAXMIUM_CACHE_BUF_SIZE          64  // 缓存数据个数
#endif

#define EXPIRATION_ININTIAL_VALUE           5
#define UPDATE_TIME_PERIOD                  250 // 更新时间周期，单位毫秒

/*********************************************************************
*
*       Defines, Fsm
*
**********************************************************************
*/

def_simple_fsm(delete_expired_cache_record,
               def_params(
                   cache_data_t tCacheData;
                   uint16_t hwTimeCounter;
               ))
end_def_simple_fsm(delete_expired_cache_record)

declare_fsm_initialiser(delete_expired_cache_record)
declare_fsm_implementation(delete_expired_cache_record)

def_simple_fsm(update_cache_record,
               def_params(
                   uint32_t wTimeout;
                   uint8_t chFailedCounter;
                   cache_data_t tCacheData;
                   raw_data_t *ptRawData;
                   proxy_request_t *ptRequest;
                   proxy_response_t *ptResponse;
                   fsm(safe_read_and_write_operation) tOperation;
               ))
end_def_simple_fsm(update_cache_record)

declare_fsm_initialiser(update_cache_record)
declare_fsm_implementation(update_cache_record)

def_simple_fsm(read_cache_data,
               def_params(
                   proxy_request_t *ptRequest;
                   proxy_response_t *ptResponse;
                   raw_data_t *ptRaw;
                   uint32_t wDelayCounter;
               ))
end_def_simple_fsm(read_cache_data)

declare_fsm_initialiser(read_cache_data, args(proxy_request_t *ptRequest))
declare_fsm_implementation(read_cache_data, args(proxy_response_t **pptResponse))

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
cache_data_list_t s_tCacheRecordList;
cache_node_t s_tCacheNodeBuffer[MAXMIUM_CACHE_BUF_SIZE];

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
static int32_t cache_record_reset_peek(void);
int32_t cache_record_peek(proxy_request_t *ptRequest);
int32_t make_cache_response_with_request(const proxy_request_t *ptRequest, proxy_response_t *ptResponse);

/*********************************************************************
*
*       Function Implention
*
**********************************************************************
*/

/**
 * \brief 初始化缓存数据记录链表
 */
int32_t cache_record_init(void)
{
    int32_t nRetval = 0;

    nRetval = cache_data_list_init(&s_tCacheRecordList, s_tCacheNodeBuffer, MAXMIUM_CACHE_BUF_SIZE);
    
    return nRetval;
}

/**
 * \brief       定时调用，删除过期计数器为零的缓存记录
 * \return      fsm_rt_t
 */
fsm_initialiser(delete_expired_cache_record)
init_body()

fsm_implementation(delete_expired_cache_record)
{
    def_states(DECREASE_EXPIRATION_COUNTER, DELETE_EXPIRED_RECORD, DELAY_1S)

    body_begin()

    on_start(
        this.hwTimeCounter = 0;
    )

    state(DECREASE_EXPIRATION_COUNTER) {
        decrease_expiration_counter_in_list(&s_tCacheRecordList);
        transfer_to(DELETE_EXPIRED_RECORD);
    }

    state(DELETE_EXPIRED_RECORD) { //-V729
        delete_expired_record_in_list(&s_tCacheRecordList);
        transfer_to(DELAY_1S);
    }

    state(DELAY_1S) { //-V729
        if (++this.hwTimeCounter > 1000) {
            this.hwTimeCounter = 0;
            update_state_to(DECREASE_EXPIRATION_COUNTER);
        }
    }

    body_end()
}

/**
 * \brief       定时调用，读取设备寄存器数据，更新自身缓存
 * \return      fsm_rt_t
 */
fsm_initialiser(update_cache_record)
init_body({
    this.ptRawData = (raw_data_t *)RAW_DATA_MALLOC();
    this.ptRequest = (proxy_request_t *)PROXY_REQUEST_MALLOC();
    this.ptRequest->ptRaw = this.ptRawData;
    if ((NULL == this.ptRawData) || (NULL == this.ptRequest))
    {
        abort_init();
    }
})

fsm_implementation(update_cache_record)
{
    def_states(PEEK_RECORD, READ_DATA, UPDATE_RECORD, DELAY_500MS)

    body_begin()

    on_start(
        this.wTimeout = 0;
        this.chFailedCounter = 0;
    )

    state(PEEK_RECORD) {
        if (0 == cache_record_peek(this.ptRequest)) {
            this.ptResponse = NULL;
            init_fsm(safe_read_and_write_operation, &this.tOperation, args(this.ptRequest));
            update_state_to(READ_DATA);
        } else {
            cache_record_reset_peek();
            fsm_cpl();
        }
    }

    state(READ_DATA) {
        if (fsm_rt_cpl == call_fsm(safe_read_and_write_operation, &this.tOperation, args(&this.ptResponse))) {
            if (NULL != this.ptResponse) {
                this.ptResponse->chPortDst = this.ptRequest->chPortSrc;
                update_state_to(UPDATE_RECORD);
            } else {
                /*! 读取缓存失败，删除请求对应的缓存记录 */
                delete_cache_record_by_request(this.ptRequest);
                MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"[response] read failed\r\n");
            }

            transfer_to(PEEK_RECORD);
        }
    }

    state(UPDATE_RECORD) {
        if (0 != refresh_cache_record(this.ptRequest, this.ptResponse)) {
            SYSLOG_D("update failed\r\n");
        }

        RAW_DATA_FREE(this.ptResponse->ptRaw);
        PROXY_RESPONSE_FREE(this.ptResponse);

        transfer_to(DELAY_500MS);
    }

    state(DELAY_500MS) { //-V729
        if (++this.wTimeout > UPDATE_TIME_PERIOD) {
            this.wTimeout = 0;
            update_state_to(PEEK_RECORD);
        }
    }

    body_end()
}

/**
 * \brief       根据 request 请求，从缓存中读取数据
 * \param[in]   request 请求结构体指针
 * \return      fsm_rt_t
 */
fsm_initialiser(read_cache_data, args(proxy_request_t *ptRequest))
init_body(
    this.ptRequest = ptRequest;
)

fsm_implementation(read_cache_data, args(proxy_response_t **pptResponse))
{
    def_states(MALLOC_RESPONSE, MALLOC_RAW_DATA, FIND_MATCH_REQUEST, ADD_REPLY_TIME)

    body_begin()

    on_start(
        this.wDelayCounter = 0;
    );

    state(MALLOC_RESPONSE) { //-V729
        this.ptResponse = PROXY_RESPONSE_MALLOC();
        if (NULL != this.ptResponse) {
            update_state_to(MALLOC_RAW_DATA);
        }
    }

    state(MALLOC_RAW_DATA) {
        this.ptRaw = (raw_data_t *)RAW_DATA_MALLOC();
        if (NULL != this.ptRaw) {
            this.ptResponse->ptRaw = this.ptRaw;
            update_state_to(FIND_MATCH_REQUEST);
        }
    }

    state(FIND_MATCH_REQUEST) {
        if (0 == make_cache_response_with_request(this.ptRequest, this.ptResponse)) {
            *pptResponse = this.ptResponse;
            update_state_to(ADD_REPLY_TIME);
        } else {
            /*! 当没有缓存可读的时候，不需要立即返回 */
            if (++this.wDelayCounter > MINMIUM_READ_CACHE_WAIT_TIME) {
                *pptResponse = NULL;
                RAW_DATA_FREE(this.ptRaw);
                PROXY_RESPONSE_FREE(this.ptResponse);
                fsm_cpl();
            }
        }
    }

    state(ADD_REPLY_TIME) {
        /*! 人为增加读取缓存的时间 */
        if (++this.wDelayCounter > MINMIUM_READ_CACHE_PROCESS_TIME) {
            this.wDelayCounter = 0;
            fsm_cpl();
        }
    }

    body_end()
}

int32_t cache_record_reset_peek(void)
{
    return cache_data_list_peek_reset(&s_tCacheRecordList);
}

static int32_t create_record(const proxy_request_t *ptRequest)
{
    cache_data_t tCache;
    uint8_t chByteLength = 0;
    int32_t nIndex = -1;

    chByteLength = calc_storage_space_in_byte(ptRequest->tBodyCfg.chCode,
                   ptRequest->tBodyCfg.hwDataNumber);

    tCache.pchDataBuffer = port_malloc_4(chByteLength);
    if (NULL == tCache.pchDataBuffer) {
        return -1;
    }

    tCache.chExpirationCounter  = EXPIRATION_ININTIAL_VALUE;
    tCache.chPort               = ptRequest->chPortDstBuf[0];
    tCache.chID                 = ptRequest->chID;
    tCache.chCode               = ptRequest->tBodyCfg.chCode;
    tCache.hwDataAddr           = ptRequest->tBodyCfg.hwDataAddr;
    tCache.hwDataNumber         = ptRequest->tBodyCfg.hwDataNumber;

    nIndex = cache_data_list_insert_tail(&s_tCacheRecordList, &tCache);
    if (nIndex < 0) {
        port_free_4(tCache.pchDataBuffer);
    }

    return nIndex;
}

static inline void add_attr_to_search_cache_record_obj(search_cache_data_t *ptSearch, const proxy_request_t *ptRequest)
{
    ptSearch->chID = ptRequest->chID;
    ptSearch->chCode = ptRequest->tBodyCfg.chCode;
    ptSearch->hwDataAddr = ptRequest->tBodyCfg.hwDataAddr;
    ptSearch->hwDataNumber = ptRequest->tBodyCfg.hwDataNumber;
}

int32_t create_cache_record(const proxy_request_t *ptRequest)
{
    int32_t nRet = -1;
    cache_data_t *ptCache = NULL;
    search_cache_data_t tSearch;

    if (NULL == ptRequest) {
        return -1;
    }

    add_attr_to_search_cache_record_obj(&tSearch, ptRequest);
    ptCache = cache_data_list_search_data_ptr(&s_tCacheRecordList,
              find_equal_cache_record_hook,
              &tSearch);

    if (NULL == ptCache) {
        nRet = create_record(ptRequest);
        MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"ID(%d) addr(%04x) num(%d)\r\n",
                 ptRequest->chID,
                 ptRequest->tBodyCfg.hwDataAddr,
                 ptRequest->tBodyCfg.hwDataNumber);
        MLOG_RAW(RTT_CTRL_TEXT_BRIGHT_GREEN"save record (%d)\r\n", nRet);
    }

    return nRet;
}

int32_t delete_cache_record_by_request(const proxy_request_t *ptRequest)
{
    search_cache_data_t tSearch;

    tSearch.chID = ptRequest->chID;
    tSearch.hwDataAddr = ptRequest->tBodyCfg.hwDataAddr;
    tSearch.hwDataNumber = ptRequest->tBodyCfg.hwDataNumber;

    switch (ptRequest->tBodyCfg.chCode) {
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            tSearch.chCode = MB_FUNC_READ_HOLDING_REGISTERS;
            cache_data_list_delete_equal(&s_tCacheRecordList, find_equal_cache_record_hook,
                                         &tSearch,
                                         destroy_cache_data_hook);
            break;

        case MB_FUNC_WRITE_MULTIPLE_COILS:
            tSearch.chCode = MB_FUNC_READ_COILS;
            cache_data_list_delete_equal(&s_tCacheRecordList, find_equal_cache_record_hook,
                                         &tSearch,
                                         destroy_cache_data_hook);
            break;

        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            tSearch.chCode = ptRequest->tBodyCfg.chCode;
            cache_data_list_delete_equal(&s_tCacheRecordList, find_equal_cache_record_hook,
                                         &tSearch,
                                         destroy_cache_data_hook);
            break;

        default:
            break;
    }

    return 0;
}

static int32_t add_attr_to_refresh_cache_record_obj(update_cache_data_t *ptData,
        const proxy_request_t *ptRequest, const proxy_response_t *ptResponse)
{
    int32_t nRetval = -1;

    ptData->chID = ptRequest->chID;
    ptData->chCode = ptRequest->tBodyCfg.chCode;
    ptData->hwDataAddr = ptRequest->tBodyCfg.hwDataAddr;
    ptData->hwDataNumber = ptRequest->tBodyCfg.hwDataNumber;

    switch (ptData->chCode) {
        case MB_FUNC_READ_COILS:
        case MB_FUNC_READ_DISCRETE_INPUTS:
        case MB_FUNC_READ_HOLDING_REGISTERS:
        case MB_FUNC_READ_INPUT_REGISTERS:
            ptData->pchDataBuffer = ptResponse->tBodyCfg.pchHR; //-V1037
            nRetval = 0;
            break;

        case MB_FUNC_WRITE_REGISTER:
        case MB_FUNC_WRITE_SINGLE_COIL:
            ptData->pchDataBuffer = ptResponse->tBodyCfg.pchHR;
            nRetval = 0;
            break;

        default:
            ptData->pchDataBuffer = NULL;
            break;
    }

    return nRetval;
}

/**
* \brief       根据 request 请求和 resposne 应答， 更新缓存记录里的数据
* \param[in]   ptRequest     request 请求结构体指针
* \param[in]   ptResponse    response 应答结构体指针
* \return      -1(失败)
*/
int32_t refresh_cache_record(const proxy_request_t *ptRequest, const proxy_response_t *ptResponse)
{
    update_cache_data_t tData;

    if ((NULL == ptRequest) || (NULL == ptResponse)) {
        return -1;
    }

    if (0 != is_response_match_request(&ptRequest->tBodyCfg, &ptResponse->tBodyCfg)) {
        return -1;
    }

    if (0 == add_attr_to_refresh_cache_record_obj(&tData, ptRequest, ptResponse)) {
        cache_data_list_travel(&s_tCacheRecordList, refresh_cache_data_object_hook, &tData);
        return 0;
    }

    return -1;
}

static inline void refresh_cache_data_expiration_counter(cache_data_t *ptCache)
{
    ptCache->chExpirationCounter = EXPIRATION_ININTIAL_VALUE;
}

/**
* \brief       根据 request 请求, 从缓存中找出匹配的数据，填充到 response 结构体内
* \param[in]   ptRequest     request 请求结构体指针
* \param[out]  ptResponse    response 应答结构体指针
* \return      -1(失败)
*/
int32_t make_cache_response_with_request(const proxy_request_t *ptRequest, proxy_response_t *ptResponse)
{
    int32_t nLength = 0;
    cache_data_t *ptCache = NULL;
    search_cache_data_t tSearch;

    if ((NULL == ptRequest) || (NULL == ptResponse)) {
        return -1;
    } else if (NULL == ptResponse->ptRaw) {
        return -1;
    }

    add_attr_to_search_cache_record_obj(&tSearch, ptRequest);

    ptCache = cache_data_list_search_data_ptr(&s_tCacheRecordList,
              find_equal_cache_record_hook,
              &tSearch);

    if (NULL == ptCache) {
        return -1;
    }

    nLength = calc_storage_space_in_byte(ptCache->chCode,
                                         ptRequest->tBodyCfg.hwDataNumber);

    if (0 >= nLength) {
        return -1;
    }

    ptResponse->tBodyCfg.chSlave        = ptRequest->tBodyCfg.chSlave;
    ptResponse->tBodyCfg.chCode         = ptRequest->tBodyCfg.chCode;
    ptResponse->tBodyCfg.chByteCount    = (uint8_t)nLength;
    ptResponse->tBodyCfg.pchHR          = ptCache->pchDataBuffer;

    nLength = make_response_body(ptResponse->ptRaw->chBuf, RAW_DATA_BUF_SIZE,
                                 &ptResponse->tBodyCfg);

    ptResponse->ptRaw->hwBufSize = (uint16_t)nLength;
    ptResponse->ptRaw->chPort = ptRequest->chPortDstBuf[0];

    refresh_cache_data_expiration_counter(ptCache);

    return 0;
}

int32_t cache_record_peek(proxy_request_t *ptRequest)
{
    cache_data_t tData;

    if (NULL == ptRequest) {
        return -1;
    }

    if (0 == cache_data_list_peek(&s_tCacheRecordList, &tData)) {
        return cache_data_transform_request(&tData, ptRequest);
    }

    return -1;
}

/*************************** End of file ****************************/
