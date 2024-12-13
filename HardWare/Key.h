#include "Timer.h"

#ifndef _KEY_H_
#define _KEY_H_

typedef enum
{
    KEY_ENCODER_PRESS,
    KEYNUM,
} KEY_ID;

typedef enum
{
    STEP_WAIT,              // �ȴ�����
    STEP_PRESS,             // ��������
    STEP_LONG_PRESS,        // ��������
    STEP_CONTINUOUS_PRESS,  // ��������
} SCAN_STEP; // ����������

typedef struct
{
    SCAN_STEP ScanStep;
    uint16_t ShakeTime;        // ����
    uint16_t LongPressTimer;   // ����
    uint16_t ContPressTimer;   // ��������
} KEY_SCAN; // ����ɨ�����

typedef enum
{
    KEY_IDLE,                    // ����
    KEY_PRESS,                   // ��������
    KEY_RELEASE,                 // �����ͷ�
    KEY_LONG_PRESS,              // ��������
    KEY_LONG_PRESS_CONTINUOUS,   // ��������
    KEY_LONG_PRESS_RELEASE,      // �����ͷ�
} KEY_STATUS;

typedef struct
{
    GPIO_TypeDef* GPIO_x;
    uint32_t GPIO_RCC;
    uint16_t GPIO_PIN;
    GPIOMode_TypeDef GPIO_MODE;
    uint8_t High_Active;        // �ߵ�ƽ��Ч
} KEY_GPIO;

typedef void (*KeyCallBack) (void);
typedef struct
{
    KeyCallBack Press;              // ����
    KeyCallBack Release;            // �ͷ�
    KeyCallBack LongPress;          // ����
    KeyCallBack LongPress_Cont;     // ��������
    KeyCallBack LongPress_Release;  // �����ͷ�
} KEY_CALLBACK;

typedef struct
{
    KEY_ID ID;
    KEY_STATUS Status;
    KEY_CALLBACK Callback;
    KEY_GPIO GPIO;
    KEY_SCAN Scan;
} MY_KEY;

extern MY_KEY KeyAttribute[KEYNUM];

typedef void (*KeyEvent_CallBack_t) (KEY_ID Keys, KEY_STATUS Status);


//��������ʱ��
#define KEY_SCANTIME 20 * SYS_MS    // 20ms

//��������ʱ��
#define KEY_PRESS_LONG_TIME 1 * SYS_S   // 1s

//�����������ʱ��
#define KEY_PRESS_CONTINUE_TIME 150 * SYS_MS    // 150ms

void Key_Init(void);
void Key_CBRegister_P(KEY_ID KeyID, KeyCallBack CB);
void Key_CBRegister_R(KEY_ID KeyID, KeyCallBack CB);
void Key_CBRegister_LP(KEY_ID KeyID, KeyCallBack CB);
void Key_CBRegister_LP_Cont(KEY_ID KeyID, KeyCallBack CB);
void Key_CBRegister_LP_R(KEY_ID KeyID, KeyCallBack CB);
uint8_t Key_Get_GPIO_Status(KEY_ID KeyID);
void Key_Scan(void);

#endif
