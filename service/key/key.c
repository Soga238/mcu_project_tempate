/****************************************Copyright (c)***************************************
**                                  常州汇邦电子有限公司
**
**                                 http://www.cnwinpark.cn
**
**
**--------------文件信息---------------------------------------------------------------------
** 文 件 名:    key.c
** 修改日期:    2017-12-30
** 上个版本:
** 描    述:
**
**-------------------------------------------------------------------------------------------
** 创    建:    JianHang
** 创建日期:    2017-12-30
** 版    本:    1.0
** 描    述:    按键驱动函数文件，不支持组合按键
**
**-------------------------------------------------------------------------------------------
** 修    改:
** 修改日期:
** 版    本:
** 描    述:
**
*********************************************************************************************/
#include "key_queue.h"

extern uint8_t get_key_scan_value( void );                 // 按键扫描函数（外部定义）

/*
    按键通过读取IO口高低电平，判断按键是否按下，程序连续
    读取 KEY_SCAN_COUNT 次，若保持低电平，确定按键真的被按下
*/
#define KEY_SCAN_COUNT                      3           // 按键扫描次数
#define KEY_LONG_PRESSED_COUNT              40          // 进入按键长按状态计数阈值
#define KEY_REPEAT_COUNT                    4           // 进入按键连发状态计数阈值
#define KEY_QUEUE_SIZE                      8           // 按键队列长度

static key_event_t key_fronted_buffer[KEY_QUEUE_SIZE];  // 按键事件生产者缓存区
static key_event_t key_decotor_buffer[KEY_QUEUE_SIZE];  // 按键事件消费者缓存区

static key_queue_t key_fronted_queue;                   // 按键事件生产者队列
static key_queue_t key_decotor_queue;                   // 按键事件消费者队列

/********************************************************************************************
* 函数名称：key_init
* 功能说明：按键相关配置初始化
* 输入参数：无
* 输出参数：无
* 其它说明：无
*********************************************************************************************/
void key_init( void )
{
    Key_Queue_Init( &key_fronted_queue, key_fronted_buffer, KEY_QUEUE_SIZE );
    Key_Queue_Init( &key_decotor_queue, key_decotor_buffer, KEY_QUEUE_SIZE );
}

#define KEY_CHECK_START                     0           // 开始状态
#define KEY_CHECK_SCAN_PORT                 1           // 扫描GPIO端口
#define KEY_CHECK_SCAN_COUNT                2           // 扫描计数
#define KEY_CHECK_SCAN_END                  3           // 扫描结束

/********************************************************************************************
* 函数名称：key_check
* 功能说明：按键键值读取函数
* 输入参数：keyValue   ：按键读取完成后，保存在keyValue指向的空间中
* 输出参数：1：   按键键值读取完成
*           0：   按键键值读取中
* 其它说明：无
*********************************************************************************************/
static int8_t key_check( uint8_t* keyValue )
{
    static uint8_t s_chState = KEY_CHECK_START;
    static uint8_t s_chCount = 0;
    static uint8_t s_chHistoryValue = KEY_UP;
    static uint8_t s_chCurrentValue = KEY_UP;

    switch( s_chState ) {
    case KEY_CHECK_START:
        s_chCount = 0;                                  // 扫描计数值初始化为0
        s_chState = KEY_CHECK_SCAN_PORT;                // 跳转GPIO端口扫描

    case KEY_CHECK_SCAN_PORT:
        s_chCurrentValue = get_key_scan_value();        // 获取当前按键值
        if( s_chCurrentValue != s_chHistoryValue ) {    // 历史键值和当前扫描键值不同，复位扫描计数
            /*键值不同说明按键状态改变*/
            s_chCount = 0;
            s_chHistoryValue = s_chCurrentValue;        // 更新历史键值
            break;
        } else {
            s_chCount++;                                // 扫描计数加一
            s_chState = KEY_CHECK_SCAN_COUNT;
        }

    case KEY_CHECK_SCAN_COUNT:
        if( s_chCount < KEY_SCAN_COUNT ) {              // 扫描计数未到阈值
            s_chState = KEY_CHECK_SCAN_PORT;
            break;
        }
        s_chState = KEY_CHECK_SCAN_END;                 // 进入扫描结束状态

    case KEY_CHECK_SCAN_END:
        s_chState = KEY_CHECK_START;
        *keyValue = s_chCurrentValue;                  // 保存此次读取的键值
        return 1;

    default:
        break;
    }
    return 0;
}


#define KEY_TRIGER_START                    0           // 
#define KEY_TRIGER_EDGE_CHECK               1
#define KEY_TRIGER_EDGE_ACTIVED             2
#define KEY_TRIGER_END                      3
/********************************************************************************************
* 函数名称：key_fronted
* 功能说明：记录按键键值变化
* 输入参数：无
* 输出参数：1：   记录键值变化完成
*           0：   记录键值变化进行中
* 其它说明：无
*********************************************************************************************/
static int8_t key_fronted( void )
{
    static uint8_t s_chState = KEY_TRIGER_START;
    static uint8_t key_history_value = KEY_NULL;
    static uint8_t key_now_value;
    key_event_t tKey;

    switch( s_chState ) {
    case KEY_TRIGER_START:
        s_chState = KEY_TRIGER_EDGE_CHECK;

    case KEY_TRIGER_EDGE_CHECK:
        if( !key_check( &key_now_value ) ) {            // 查询键值，并保存在key_now_value
            break;
        }

        if( key_now_value == key_history_value ) {      // 键值无变化，结束此次查询
            break;
        }
        s_chState = KEY_TRIGER_EDGE_ACTIVED;

    case KEY_TRIGER_EDGE_ACTIVED:
        /*
            key_history_value保存上一次的按键值，不为NULL说明按键之前处于
            被按下状态。进入此状态，必须发生键值变化，说明按键释放
        */
        if( KEY_NULL != key_history_value ) {           // 按键被释放
            tKey.tEvent = KEY_UP;
            tKey.keyValue = key_history_value;
            Key_Enqueue( &key_fronted_queue, &tKey );   // 按键弹起事件入生产者队列
        }
        /*
            key_now_value保存本次的按键值，不为NULL说明按键处于
            被按下状态。
        */
        if( KEY_NULL != key_now_value ) {               // 按键被按下
            tKey.tEvent = KEY_DOWN;
            tKey.keyValue = key_now_value;
            Key_Enqueue( &key_fronted_queue, &tKey );   // 按键按下事件入生产者队列
        }
        s_chState = KEY_TRIGER_END;

    case KEY_TRIGER_END:                                // 按键变化记录结束状态
        s_chState = KEY_TRIGER_START;
        key_history_value = key_now_value;
        return 0;

    default:
        break;
    }

    return 1;
}

#define KEY_DETCTED_START                       0       // 初始状态
#define KEY_DETCTED_WAIT_DOWN_EVENT             1       // 等待按下事件
#define KEY_DETCTED_DETERMINE_EVENT_TYPE        2       // 判断事件类型
#define KEY_DETCTED_WAIT_UP_EVENT               3       // 等待弹起事件
#define KEY_DETCETD_LONG_COUNT                  4
#define KEY_DETCTED_DETERMINE_EVENT_TYPE2       5       // 判断事件类型
#define KEY_DETCTED_WAIT_UP_EVENT2              6
#define KEY_DETCTED_REPEAT_COUNT                7
#define KEY_DETCTED_WAIT_LONG_PRESSED_FINISH    8

#define RESET_FSM_STATE() do{s_chState = KEY_DETCTED_START;}while(0)

/********************************************************************************************
* 函数名称：key_detcted
* 功能说明：按键事件产生函数
* 输入参数：无
* 输出参数：无
* 其它说明：无
*********************************************************************************************/
void key_detcted( void )
{
    static uint8_t s_chState = KEY_DETCTED_START;
    static key_event_t tKey = {KEY_NULL, KEY_UP};
    static uint16_t long_count = 0;

    switch( s_chState ) {
    case KEY_DETCTED_START:
        long_count = 0;
        s_chState = KEY_DETCTED_WAIT_DOWN_EVENT;

    case KEY_DETCTED_WAIT_DOWN_EVENT:                   // 等待按键触发
        if( 0 == Key_Dequeue( &key_fronted_queue, &tKey ) ) {
            break;                                      // 退出函数
        }

        s_chState = KEY_DETCTED_DETERMINE_EVENT_TYPE;

    case KEY_DETCTED_DETERMINE_EVENT_TYPE:              // 判断按键类型
        if( KEY_UP == tKey.tEvent ) {                   // 第一个就是弹起事件，直接退出函数
            s_chState = KEY_DETCTED_WAIT_DOWN_EVENT;
            break;
        }

        Key_Enqueue( &key_decotor_queue, &tKey );       // DOWN事件入队
        s_chState = KEY_DETCTED_WAIT_UP_EVENT;


    case KEY_DETCTED_WAIT_UP_EVENT:                     //一定时间内，按键未释放，进入长按判断
        if( 0 == Key_Dequeue( &key_fronted_queue, &tKey ) ) {
            s_chState = KEY_DETCETD_LONG_COUNT;
            long_count++;
            break;
        }

        s_chState = KEY_DETCTED_DETERMINE_EVENT_TYPE2;

    case KEY_DETCTED_DETERMINE_EVENT_TYPE2:             // 如果按键释放，退出，否则长按状态循环
        if( tKey.tEvent == KEY_DOWN ) {
            s_chState = KEY_DETCTED_WAIT_UP_EVENT;
        } else {
            Key_Enqueue( &key_decotor_queue, &tKey );   // 未到长按就释放
            tKey.tEvent = KEY_PRESSED;
            Key_Enqueue( &key_decotor_queue, &tKey );   // 一次短按事件入队
            RESET_FSM_STATE();
            return ;
        }

        break;

    case KEY_DETCETD_LONG_COUNT:                        // 长按计时
        if( long_count >= KEY_LONG_PRESSED_COUNT ) {    // 到达长按计数值，长按事件入队
            s_chState = KEY_DETCTED_WAIT_UP_EVENT2;
            long_count = 0;
            tKey.tEvent = KEY_LONG_PRESSED;
            Key_Enqueue( &key_decotor_queue, &tKey );
        } else {
            s_chState = KEY_DETCTED_WAIT_UP_EVENT;
            break;
        }

    case KEY_DETCTED_WAIT_UP_EVENT2:                    // 按键仍未释放，连发事件判断
        if( 0 == Key_Dequeue( &key_fronted_queue, &tKey ) ) {
            s_chState = KEY_DETCTED_REPEAT_COUNT;
            long_count++;
        } else {
            s_chState = KEY_DETCTED_WAIT_LONG_PRESSED_FINISH;
        }

        break;

    case KEY_DETCTED_REPEAT_COUNT:
        if( long_count >= KEY_REPEAT_COUNT ) {
            long_count = 0;
            tKey.tEvent = KEY_REPEAT;
            Key_Enqueue( &key_decotor_queue, &tKey );
        }

        s_chState = KEY_DETCTED_WAIT_UP_EVENT2;
        break;

    case KEY_DETCTED_WAIT_LONG_PRESSED_FINISH:
        if( tKey.tEvent == KEY_UP ) {
            RESET_FSM_STATE();
            Key_Enqueue( &key_decotor_queue, &tKey );
        } else {
            s_chState = KEY_DETCTED_WAIT_UP_EVENT2;
        }
        break;

    default:
        break;
    }
}

/********************************************************************************************
* 函数名称：key_task
* 功能说明：按键处理函数
* 输入参数：无
* 输出参数：无
* 其它说明：外部函数10ms调用
*********************************************************************************************/
void key_task( void )
{
    key_fronted();
    key_detcted();
}

/********************************************************************************************
* 函数名称：get_key
* 功能说明：获取按键事件
* 输入参数：Key  ：按键事件保存变量指针
* 输出参数：无
* 其它说明：
*********************************************************************************************/
uint8_t get_key( key_event_t* Key )
{
    return Key_Dequeue( &key_decotor_queue, Key );
}

/********************************************************************************************
* 函数名称：key_clear
* 功能说明：清除按键事件缓存
* 输入参数：无
* 输出参数：无
* 其它说明：无
*********************************************************************************************/
void key_clear( void )
{
    Key_Queue_Clr( &key_decotor_queue );
    Key_Queue_Clr( &key_fronted_queue );
}



















