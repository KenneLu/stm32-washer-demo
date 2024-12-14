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
    STEP_WAIT,              // 等待按下
    STEP_PRESS,             // 单击按下
    STEP_LONG_PRESS,        // 长按按下
    STEP_CONTINUOUS_PRESS,  // 持续长按
} SCAN_STEP; // 按键检测过程

typedef struct
{
    SCAN_STEP ScanStep;
    uint16_t ShakeTime;        // 消抖
    uint16_t LongPressTimer;   // 长按
    uint16_t ContPressTimer;   // 连续长按
} KEY_SCAN; // 按键扫描相关

typedef enum
{
    KEY_IDLE,                    // 空闲
    KEY_PRESS,                   // 单击按下
    KEY_RELEASE,                 // 单击释放
    KEY_LONG_PRESS,              // 长按按下
    KEY_LONG_PRESS_CONTINUOUS,   // 长按持续
    KEY_LONG_PRESS_RELEASE,      // 长按释放
} KEY_STATUS;

typedef struct
{
    GPIO_TypeDef* GPIO_x;
    uint32_t GPIO_RCC;
    uint16_t GPIO_PIN;
    GPIOMode_TypeDef GPIO_MODE;
    uint8_t High_Active;        // 高电平有效
} KEY_GPIO;

typedef void (*KeyCallBack) (void);
typedef struct
{
    KeyCallBack Press;              // 按下
    KeyCallBack Release;            // 释放
    KeyCallBack LongPress;          // 长按
    KeyCallBack LongPress_Cont;     // 持续长按
    KeyCallBack LongPress_Release;  // 长按释放
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


//按键消抖时间
#define KEY_SCANTIME 20 * SYS_MS    // 20ms

//连续长按时间
#define KEY_PRESS_LONG_TIME 800 * SYS_MS   // 0.5s

//持续长按间隔时间
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
