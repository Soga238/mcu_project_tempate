/****************************************Copyright (c)***************************************
**                                  常州汇邦电子有限公司
**
**                                 http://www.cnwinpark.cn
**
**
**--------------文件信息---------------------------------------------------------------------
** 文 件 名:    key_event.h
** 修改日期:    2017-12-30
** 上个版本:
** 描    述:
**
**-------------------------------------------------------------------------------------------
** 创    建:    JianHang
** 创建日期:    2017-12-30
** 版    本:    1.0
** 描    述:    按键事件定义头文件
**
**-------------------------------------------------------------------------------------------
** 修    改:
** 修改日期:
** 版    本:
** 描    述:
**
*********************************************************************************************/

#include "stdint.h"

#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#define KEY_NULL            0x00

typedef enum key_event_u key_event_u;
enum key_event_u {
    KEY_UP = 0,                     // 按键弹起
    KEY_DOWN,                       // 按键按下

    /*
        使用此三个事件即可
    */
    KEY_PRESSED,                    // 按键短按
    KEY_LONG_PRESSED,               // 按键长按
    KEY_REPEAT                      // 按键连发
};

typedef struct key_event_t key_event_t;
struct key_event_t {
    uint8_t keyValue;
    key_event_u tEvent;
};

#endif


