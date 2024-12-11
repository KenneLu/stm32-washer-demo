#ifndef __TB6612_H
#define __TB6612_H

//GPIOA
#define TB6612_GPIO_RCC         RCC_APB2Periph_GPIOB
#define TB6612_GOIO_x           GPIOB
#define TB6612_GOIO_PIN_AIN1    GPIO_Pin_0
#define TB6612_GOIO_PIN_AIN2    GPIO_Pin_1
#define TB6612_GOIO_MODE        GPIO_Mode_Out_PP

void TB6612_Init(void);
void TB6612_Motor_Breathe(void);
void TB6612_Motor_SetSpeed(int8_t Speed);

#endif
