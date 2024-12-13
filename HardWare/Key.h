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

//����������
typedef enum
{
    KEY_STEP_WAIT,              //�ȴ�����
    KEY_STEP_CLICK,             //��������
    KEY_STEP_LONG_PRESS,        //����
    KEY_STEP_CONTINUOUS_PRESS,  //��������
} KEY_STEP_TYPEDEF;

typedef enum
{
    KEY_IDLE,                   //��������
    KEY_CLICK,                  //����ȷ��
    KEY_CLICK_RELEASE,          //�����ͷ�
    KEY_LONG_PRESS,             //����ȷ��
    KEY_LONG_PRESS_CONTINUOUS,  //��������
    KEY_LONG_PRESS_RELEASE,     //�����ͷ�
} KEY_EVENT_TYPEDEF;

typedef void (*KeyEvent_CallBack_t) (KEY_TYPEDEF keys, KEY_EVENT_TYPEDEF state);

//��������ʱ��
#define KEY_SCANTIME 20 * SYS_MS    //20ms

//��������ʱ��
#define KEY_PRESS_LONG_TIME 1 * SYS_S   //1s

//�����������ʱ��
#define KEY_PRESS_CONTINUE_TIME 150 * SYS_MS    //150ms

void KeyInit(void);
void KeyScanCBRegister(KeyEvent_CallBack_t pCB);
void KeyProc(void);



#endif
