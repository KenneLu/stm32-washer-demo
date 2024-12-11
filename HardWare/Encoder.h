#ifndef __ENCODER_H
#define __ENCODER_H

//GPIOA
#define Encoder_GPIO_RCC      RCC_APB2Periph_GPIOA
#define Encoder_GOIO_x        GPIOA
#define Encoder_GOIO_PIN_A    GPIO_Pin_6
#define Encoder_GOIO_PIN_B    GPIO_Pin_7
#define Encoder_GOIO_PIN_C    GPIO_Pin_5
#define Encoder_GOIO_MODE     GPIO_Mode_IPU

//TIM3 IC 编码器接口
#define Encoder_TIM_RCC       RCC_APB1Periph_TIM3
#define Encoder_TIM_x         TIM3
#define Encoder_TIM_Channel_A TIM_Channel_1
#define Encoder_TIM_Channel_B TIM_Channel_2

void Encoder_Init(void);
uint8_t Encoder_Pressed(void);
int16_t Encoder_GetCount(void);
int16_t Encoder_GetSpeed(void);

#endif
