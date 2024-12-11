#include "stm32f10x.h"                  //Device header
#include "Encoder.h"

void Encoder_Init()
{
    //开启 GPIO 时钟
    RCC_APB2PeriphClockCmd(Encoder_GPIO_RCC, ENABLE);

    //初始化 GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = Encoder_GOIO_MODE;
    GPIO_InitStructure.GPIO_Pin = Encoder_GOIO_PIN_A | Encoder_GOIO_PIN_B | Encoder_GOIO_PIN_C;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Encoder_GOIO_x, &GPIO_InitStructure);

    //初始化 IC
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = Encoder_TIM_Channel_A;
    TIM_ICInitStructure.TIM_ICFilter = 0xF;
    TIM_ICInit(Encoder_TIM_x, &TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = Encoder_TIM_Channel_B;
    TIM_ICInitStructure.TIM_ICFilter = 0xF;
    TIM_ICInit(Encoder_TIM_x, &TIM_ICInitStructure);

    //开启 Encoder_TIM_x 时钟
    RCC_APB1PeriphClockCmd(Encoder_TIM_RCC, ENABLE);

    //初始化编码器接口
    TIM_EncoderInterfaceConfig(Encoder_TIM_x, TIM_EncoderMode_TI12, TIM_ICPolarity_Falling, TIM_ICPolarity_Rising);

    //配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1; //利用补码特性得到负数
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(Encoder_TIM_x, &TIM_TimeBaseInitStructure);

    //启动定时器
    TIM_Cmd(Encoder_TIM_x, ENABLE);
}

uint8_t Encoder_Pressed(void)
{
    return GPIO_ReadInputDataBit(Encoder_GOIO_x, Encoder_GOIO_PIN_C) == 0;
}

int16_t Encoder_GetCount(void)
{
    return (int16_t)TIM_GetCounter(Encoder_TIM_x) / 4; //编码器计数器的计数值是 4 倍的 TIM_GetCounter() 的值
}

int16_t Encoder_GetSpeed(void)
{
    int16_t Temp = TIM_GetCounter(Encoder_TIM_x);
    TIM_SetCounter(Encoder_TIM_x, 0);
    return Temp;
}
