#ifndef __PWM_H
#define __PWM_H

//GPIOA
#define PWM_GPIO_RCC      RCC_APB2Periph_GPIOA
#define PWM_GOIO_x        GPIOA
#define PWM_GOIO_PIN_OC   GPIO_Pin_8
#define PWM_GOIO_MODE     GPIO_Mode_IPU

//TIM1 OC 编码器接口
#define PWM_TIM_RCC       RCC_APB2Periph_TIM1
#define PWM_TIM_x         TIM1

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetPrescaler(uint16_t Prescaler);

#endif
