#pragma once

#ifndef __PWM_H
#define __PWM_H

typedef enum
{
    FALSE,
    TRUE
}INT_Status;

typedef enum
{
    Basic,
    General,
    Advanced
}TIM_Level;

typedef enum
{
    OC1, OC2, OC3, OC4
}OC_ID;

typedef struct
{
    uint16_t PIN;
    INT_Status Inited;
}OC_PIN;

typedef struct
{
    TIM_TypeDef* TIM_x;
    INT_Status Inited;
    TIM_Level Level;
    uint32_t TIM_RCC;
    TIM_TimeBaseInitTypeDef* InitStructure;
    uint32_t GPIO_RCC;
    GPIO_TypeDef* GPIO_x;
    OC_PIN* OC_PIN_List;
    uint8_t OC_PIN_Count;
    GPIOMode_TypeDef GPIO_MODE;
}PWM_TIM;

extern PWM_TIM PWM_TIM1, PWM_TIM2;

void PWM_TIM_Init(PWM_TIM* T, TIM_TimeBaseInitTypeDef* CustomStruct);
void PWM_GPIO_Init(PWM_TIM* T, OC_ID OC_x);
void PWM_SetCompare_x(PWM_TIM* T, OC_ID OC_x, uint16_t Compare);
void PWM_SetPrescaler(PWM_TIM* T, uint16_t Prescaler);

#endif
