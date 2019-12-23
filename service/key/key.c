/****************************************Copyright (c)***************************************
**                                  ���ݻ��������޹�˾
**
**                                 http://www.cnwinpark.cn
**
**
**--------------�ļ���Ϣ---------------------------------------------------------------------
** �� �� ��:    key.c
** �޸�����:    2017-12-30
** �ϸ��汾:
** ��    ��:
**
**-------------------------------------------------------------------------------------------
** ��    ��:    JianHang
** ��������:    2017-12-30
** ��    ��:    1.0
** ��    ��:    �������������ļ�����֧����ϰ���
**
**-------------------------------------------------------------------------------------------
** ��    ��:
** �޸�����:
** ��    ��:
** ��    ��:
**
*********************************************************************************************/
#include "key_queue.h"

extern uint8_t get_key_scan_value( void );                 // ����ɨ�躯�����ⲿ���壩

/*
    ����ͨ����ȡIO�ڸߵ͵�ƽ���жϰ����Ƿ��£���������
    ��ȡ KEY_SCAN_COUNT �Σ������ֵ͵�ƽ��ȷ��������ı�����
*/
#define KEY_SCAN_COUNT                      3           // ����ɨ�����
#define KEY_LONG_PRESSED_COUNT              40          // ���밴������״̬������ֵ
#define KEY_REPEAT_COUNT                    4           // ���밴������״̬������ֵ
#define KEY_QUEUE_SIZE                      8           // �������г���

static key_event_t key_fronted_buffer[KEY_QUEUE_SIZE];  // �����¼������߻�����
static key_event_t key_decotor_buffer[KEY_QUEUE_SIZE];  // �����¼������߻�����

static key_queue_t key_fronted_queue;                   // �����¼������߶���
static key_queue_t key_decotor_queue;                   // �����¼������߶���

/********************************************************************************************
* �������ƣ�key_init
* ����˵��������������ó�ʼ��
* �����������
* �����������
* ����˵������
*********************************************************************************************/
void key_init( void )
{
    Key_Queue_Init( &key_fronted_queue, key_fronted_buffer, KEY_QUEUE_SIZE );
    Key_Queue_Init( &key_decotor_queue, key_decotor_buffer, KEY_QUEUE_SIZE );
}

#define KEY_CHECK_START                     0           // ��ʼ״̬
#define KEY_CHECK_SCAN_PORT                 1           // ɨ��GPIO�˿�
#define KEY_CHECK_SCAN_COUNT                2           // ɨ�����
#define KEY_CHECK_SCAN_END                  3           // ɨ�����

/********************************************************************************************
* �������ƣ�key_check
* ����˵����������ֵ��ȡ����
* ���������keyValue   ��������ȡ��ɺ󣬱�����keyValueָ��Ŀռ���
* ���������1��   ������ֵ��ȡ���
*           0��   ������ֵ��ȡ��
* ����˵������
*********************************************************************************************/
static int8_t key_check( uint8_t* keyValue )
{
    static uint8_t s_chState = KEY_CHECK_START;
    static uint8_t s_chCount = 0;
    static uint8_t s_chHistoryValue = KEY_UP;
    static uint8_t s_chCurrentValue = KEY_UP;

    switch( s_chState ) {
    case KEY_CHECK_START:
        s_chCount = 0;                                  // ɨ�����ֵ��ʼ��Ϊ0
        s_chState = KEY_CHECK_SCAN_PORT;                // ��תGPIO�˿�ɨ��

    case KEY_CHECK_SCAN_PORT:
        s_chCurrentValue = get_key_scan_value();        // ��ȡ��ǰ����ֵ
        if( s_chCurrentValue != s_chHistoryValue ) {    // ��ʷ��ֵ�͵�ǰɨ���ֵ��ͬ����λɨ�����
            /*��ֵ��ͬ˵������״̬�ı�*/
            s_chCount = 0;
            s_chHistoryValue = s_chCurrentValue;        // ������ʷ��ֵ
            break;
        } else {
            s_chCount++;                                // ɨ�������һ
            s_chState = KEY_CHECK_SCAN_COUNT;
        }

    case KEY_CHECK_SCAN_COUNT:
        if( s_chCount < KEY_SCAN_COUNT ) {              // ɨ�����δ����ֵ
            s_chState = KEY_CHECK_SCAN_PORT;
            break;
        }
        s_chState = KEY_CHECK_SCAN_END;                 // ����ɨ�����״̬

    case KEY_CHECK_SCAN_END:
        s_chState = KEY_CHECK_START;
        *keyValue = s_chCurrentValue;                  // ����˴ζ�ȡ�ļ�ֵ
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
* �������ƣ�key_fronted
* ����˵������¼������ֵ�仯
* �����������
* ���������1��   ��¼��ֵ�仯���
*           0��   ��¼��ֵ�仯������
* ����˵������
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
        if( !key_check( &key_now_value ) ) {            // ��ѯ��ֵ����������key_now_value
            break;
        }

        if( key_now_value == key_history_value ) {      // ��ֵ�ޱ仯�������˴β�ѯ
            break;
        }
        s_chState = KEY_TRIGER_EDGE_ACTIVED;

    case KEY_TRIGER_EDGE_ACTIVED:
        /*
            key_history_value������һ�εİ���ֵ����ΪNULL˵������֮ǰ����
            ������״̬�������״̬�����뷢����ֵ�仯��˵�������ͷ�
        */
        if( KEY_NULL != key_history_value ) {           // �������ͷ�
            tKey.tEvent = KEY_UP;
            tKey.keyValue = key_history_value;
            Key_Enqueue( &key_fronted_queue, &tKey );   // ���������¼��������߶���
        }
        /*
            key_now_value���汾�εİ���ֵ����ΪNULL˵����������
            ������״̬��
        */
        if( KEY_NULL != key_now_value ) {               // ����������
            tKey.tEvent = KEY_DOWN;
            tKey.keyValue = key_now_value;
            Key_Enqueue( &key_fronted_queue, &tKey );   // ���������¼��������߶���
        }
        s_chState = KEY_TRIGER_END;

    case KEY_TRIGER_END:                                // �����仯��¼����״̬
        s_chState = KEY_TRIGER_START;
        key_history_value = key_now_value;
        return 0;

    default:
        break;
    }

    return 1;
}

#define KEY_DETCTED_START                       0       // ��ʼ״̬
#define KEY_DETCTED_WAIT_DOWN_EVENT             1       // �ȴ������¼�
#define KEY_DETCTED_DETERMINE_EVENT_TYPE        2       // �ж��¼�����
#define KEY_DETCTED_WAIT_UP_EVENT               3       // �ȴ������¼�
#define KEY_DETCETD_LONG_COUNT                  4
#define KEY_DETCTED_DETERMINE_EVENT_TYPE2       5       // �ж��¼�����
#define KEY_DETCTED_WAIT_UP_EVENT2              6
#define KEY_DETCTED_REPEAT_COUNT                7
#define KEY_DETCTED_WAIT_LONG_PRESSED_FINISH    8

#define RESET_FSM_STATE() do{s_chState = KEY_DETCTED_START;}while(0)

/********************************************************************************************
* �������ƣ�key_detcted
* ����˵���������¼���������
* �����������
* �����������
* ����˵������
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

    case KEY_DETCTED_WAIT_DOWN_EVENT:                   // �ȴ���������
        if( 0 == Key_Dequeue( &key_fronted_queue, &tKey ) ) {
            break;                                      // �˳�����
        }

        s_chState = KEY_DETCTED_DETERMINE_EVENT_TYPE;

    case KEY_DETCTED_DETERMINE_EVENT_TYPE:              // �жϰ�������
        if( KEY_UP == tKey.tEvent ) {                   // ��һ�����ǵ����¼���ֱ���˳�����
            s_chState = KEY_DETCTED_WAIT_DOWN_EVENT;
            break;
        }

        Key_Enqueue( &key_decotor_queue, &tKey );       // DOWN�¼����
        s_chState = KEY_DETCTED_WAIT_UP_EVENT;


    case KEY_DETCTED_WAIT_UP_EVENT:                     //һ��ʱ���ڣ�����δ�ͷţ����볤���ж�
        if( 0 == Key_Dequeue( &key_fronted_queue, &tKey ) ) {
            s_chState = KEY_DETCETD_LONG_COUNT;
            long_count++;
            break;
        }

        s_chState = KEY_DETCTED_DETERMINE_EVENT_TYPE2;

    case KEY_DETCTED_DETERMINE_EVENT_TYPE2:             // ��������ͷţ��˳������򳤰�״̬ѭ��
        if( tKey.tEvent == KEY_DOWN ) {
            s_chState = KEY_DETCTED_WAIT_UP_EVENT;
        } else {
            Key_Enqueue( &key_decotor_queue, &tKey );   // δ���������ͷ�
            tKey.tEvent = KEY_PRESSED;
            Key_Enqueue( &key_decotor_queue, &tKey );   // һ�ζ̰��¼����
            RESET_FSM_STATE();
            return ;
        }

        break;

    case KEY_DETCETD_LONG_COUNT:                        // ������ʱ
        if( long_count >= KEY_LONG_PRESSED_COUNT ) {    // ���ﳤ������ֵ�������¼����
            s_chState = KEY_DETCTED_WAIT_UP_EVENT2;
            long_count = 0;
            tKey.tEvent = KEY_LONG_PRESSED;
            Key_Enqueue( &key_decotor_queue, &tKey );
        } else {
            s_chState = KEY_DETCTED_WAIT_UP_EVENT;
            break;
        }

    case KEY_DETCTED_WAIT_UP_EVENT2:                    // ������δ�ͷţ������¼��ж�
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
* �������ƣ�key_task
* ����˵��������������
* �����������
* �����������
* ����˵�����ⲿ����10ms����
*********************************************************************************************/
void key_task( void )
{
    key_fronted();
    key_detcted();
}

/********************************************************************************************
* �������ƣ�get_key
* ����˵������ȡ�����¼�
* ���������Key  �������¼��������ָ��
* �����������
* ����˵����
*********************************************************************************************/
uint8_t get_key( key_event_t* Key )
{
    return Key_Dequeue( &key_decotor_queue, Key );
}

/********************************************************************************************
* �������ƣ�key_clear
* ����˵������������¼�����
* �����������
* �����������
* ����˵������
*********************************************************************************************/
void key_clear( void )
{
    Key_Queue_Clr( &key_decotor_queue );
    Key_Queue_Clr( &key_fronted_queue );
}



















