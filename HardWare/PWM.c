#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void PWM_Init(void)
{
    // 1.开启通用时钟，默认内部时钟源
    RCC_APB2PeriphClockCmd(PWM_GPIO_RCC, ENABLE);   // 开启GPIO的时钟
    RCC_APB2PeriphClockCmd(PWM_TIM_RCC, ENABLE);	// 开启TIM的时钟
    TIM_InternalClockConfig(PWM_TIM_x);             // 配置时钟源

    // 2.配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 滤波电路采样信号的分频系数
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;         // ARR 分辨率 1%
    TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;      // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;    // 重复计数器，置0。
    TIM_TimeBaseInit(PWM_TIM_x, &TIM_TimeBaseInitStructure);

    // 3.配置OC单元
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);                         // 默认初始化
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               // 设置OC模式
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 设置极性
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   // 设置输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;                              // 设置CCR
    if (PWM_TIM_x == TIM1) TIM_OC1Init(PWM_TIM_x, &TIM_OCInitStructure);
    else if (PWM_TIM_x == TIM2) TIM_OC2Init(PWM_TIM_x, &TIM_OCInitStructure);
    else if (PWM_TIM_x == TIM3) TIM_OC3Init(PWM_TIM_x, &TIM_OCInitStructure);
    else if (PWM_TIM_x == TIM4) TIM_OC4Init(PWM_TIM_x, &TIM_OCInitStructure);

    // 4.配置GPIO输出模式
    RCC_APB2PeriphClockCmd(PWM_GPIO_RCC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = PWM_GOIO_MODE;
    GPIO_InitStructure.GPIO_Pin = PWM_GOIO_PIN_OC;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 5.启动定时器
    if (PWM_TIM_x == TIM1) TIM_CtrlPWMOutputs(PWM_TIM_x, ENABLE);    // 定时器1是高级定时器
    TIM_Cmd(PWM_TIM_x, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare) // 通过CCR调整频率
{
    TIM_SetCompare1(PWM_TIM_x, Compare); // 0~20000±5000，对应占空比 0%~100%±25%
}

void PWM_SetPrescaler(uint16_t Prescaler) // 通过PSC调整频率
{
    TIM_PrescalerConfig(PWM_TIM_x, Prescaler, TIM_PSCReloadMode_Update);
}
