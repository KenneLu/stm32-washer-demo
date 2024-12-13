#pragma once

#include "Timer.h"

#ifndef _KEY_H_
#define _KEY_H_

#define KEY_GOIOx RCC_APB2Periph_GPIOB

#define S1_GPIO_RCC         RCC_APB2Periph_GPIOB
#define S1_GOIO_x           GPIOB
#define S1_GOIO_PIN         GPIO_Pin_11
#define S1_GOIO_MODE        GPIO_Mode_Out_PP

#define S2_GPIO_RCC         RCC_APB2Periph_GPIOB
#define S2_GOIO_x           GPIOB
#define S2_GOIO_PIN         GPIO_Pin_12
#define S2_GOIO_MODE        GPIO_Mode_Out_PP

typedef enum
{
    KEY_S1,
    KEY_S2,
    KEYNUM,
} KEY_TYPEDEF;

//按键检测过程
typedef enum
{
    KEY_STEP_WAIT,              //等待按键
    KEY_STEP_CLICK,             //按键按下
    KEY_STEP_LONG_PRESS,        //长按
    KEY_STEP_CONTINUOUS_PRESS,  //持续长按
} KEY_STEP_TYPEDEF;

typedef enum
{
    KEY_IDLE,                   //按键空闲
    KEY_CLICK,                  //单击确认
    KEY_CLICK_RELEASE,          //单击释放
    KEY_LONG_PRESS,             //长按确认
    KEY_LONG_PRESS_CONTINUOUS,  //长按持续
    KEY_LONG_PRESS_RELEASE,     //长按释放
} KEY_EVENT_TYPEDEF;

typedef void (*KeyEvent_CallBack_t) (KEY_TYPEDEF keys, KEY_EVENT_TYPEDEF state);

//按键消抖时间
#define KEY_SCANTIME 20 * SYS_MS    //20ms

//连续长按时间
#define KEY_PRESS_LONG_TIME 1 * SYS_S   //1s

//持续长按间隔时间
#define KEY_PRESS_CONTINUE_TIME 150 * SYS_MS    //150ms

void KeyInit(void);
void KeyScanCBRegister(KeyEvent_CallBack_t pCB);
void KeyProc(void);



#endif
