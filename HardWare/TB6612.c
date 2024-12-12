#include "stm32f10x.h"                  // Device header
#include "TB6612.h"
#include "PWM.h"
#include "Delay.h"
#include "OLED.h"

static PWM_TIM* TBB6612_TIM_x = &PWM_TIM1;
static OC_ID TBB6612_OC_x = OC1;
int8_t MotorBreath[] = { 0, 20, 30, 40, 30, 20, 0, -20, -30, -40, -30, -20 };	// 定义转动速度数组

void TB6612_Init(void)
{
    RCC_APB2PeriphClockCmd(TB6612_GPIO_RCC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = TB6612_GOIO_MODE;
    GPIO_InitStructure.GPIO_Pin = TB6612_GOIO_PIN_AIN1 | TB6612_GOIO_PIN_AIN2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TB6612_GOIO_x, &GPIO_InitStructure);

    PWM_TIM_Init(TBB6612_TIM_x, 0);
    PWM_GPIO_Init(TBB6612_TIM_x, TBB6612_OC_x);
}

uint16_t Speed2PWM(float Speed)
{
    // 计算角度对应的 PWM 值
    return (uint16_t)(Speed * 20000 / 100);
}

void TB6612_Motor_Breathe(void)
{
    OLED_ShowString(1, 1, "Speed:+000");
    OLED_ShowString(2, 1, "PWM:");
    for (uint8_t i = 0; i < sizeof(MotorBreath) / sizeof(MotorBreath[0]); i++)
    {
        TB6612_Motor_SetSpeed(MotorBreath[i]);  // 设置直流电机的速度为速度变量
        OLED_ShowSignedNum(1, 7, MotorBreath[i], 3);	    // OLED显示速度变量
        OLED_ShowNum(2, 5, Speed2PWM(MotorBreath[i]), 5);	//OLED显示角度变量
        Delay_s(1);
    }
}

void TB6612_Motor_SetSpeed(int8_t Speed)
{
    if (Speed >= 0) // 正转
    {
        GPIO_SetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN1);      // 置高电平
        GPIO_ResetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN2);    // 置低电平，设置方向为正转
        PWM_SetCompare_x(TBB6612_TIM_x, TBB6612_OC_x, Speed2PWM(Speed)); // PWM设置为速度值
    }
    else    // 反转
    {
        GPIO_ResetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN1);	// 置低电平
        GPIO_SetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN2);	    // 置高电平，设置方向为反转
        PWM_SetCompare_x(TBB6612_TIM_x, TBB6612_OC_x, Speed2PWM(-Speed));    //PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
    }
}