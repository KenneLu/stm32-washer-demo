#include "stm32f10x.h"                  // Device header
#include "Sys_Timer.h"


#if FREERTOS == 0

void Timer_Init(void)
{
    // 1.开启通用时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 2.选择时钟源：内部时钟
    TIM_InternalClockConfig(TIM2); // 不写也行，因为默认使用内部时钟

    // 3.配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 滤波电路采样信号的分频系数
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_Period = SYS_TIM_Period;    // ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler = SYS_TIM_Prescaler;  // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0; // 重复计数器在高级定时器才用，
    //通用定时器不需要，置0。
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    //清除在初始化时，为了预装入分频器缓冲寄存器，
    //而主动触发更新事件所产生的标志位，
    //防止复位后，因标志位而立刻进中断。
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);


    // 4.使能更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 5.配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 6.启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

uint16_t Timer_GetCounter(void)
{
    return TIM_GetCounter(TIM2);
}

/*
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
*/

#endif
