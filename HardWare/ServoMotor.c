#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "ServoMotor.h"
#include "OLED.h"
#include "Delay.h"

static PWM_TIM* Servo_Motor_TIM_x = &PWM_TIM1;
static OC_ID Servo_Motor_OC_x = OC4;
float ServoBreath[] = { 0, 30, 60, 90, 120, 150, 180 };	// 定义转动速度数组

void Servo_Motor_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_Structure_Custom = {
    .TIM_ClockDivision = TIM_CKD_DIV1, // 滤波电路采样信号的分频系数
    .TIM_CounterMode = TIM_CounterMode_Up, // 向上计数
    .TIM_Period = 20000 - 1,         // ARR 分辨率 1%
    .TIM_Prescaler = 72 - 1,       // PSC
    .TIM_RepetitionCounter = 0,    // 重复计数器，置0。
    };
    PWM_TIM_Init(Servo_Motor_TIM_x, &TIM_Structure_Custom);
    PWM_GPIO_Init(Servo_Motor_TIM_x, Servo_Motor_OC_x);
}

uint16_t Angle2PWM(float Angle)
{
    // 计算角度对应的 PWM 值
    return (uint16_t)(Angle / 180 * 2000 + 500);
}

void Servo_Motorr_Breathe(void)
{
    OLED_ShowString(3, 1, "Angle:");
    OLED_ShowString(4, 1, "PWM:");
    for (int i = 0; i < sizeof(ServoBreath) / sizeof(ServoBreath[0]); i++)
    {
        Servo_Motor_SetAngle(ServoBreath[i]);  // 设置直流电机的速度为速度变量
        OLED_ShowSignedNum(3, 7, ServoBreath[i], 3);	    // OLED显示速度变量
        OLED_ShowNum(4, 5, Angle2PWM(ServoBreath[i]), 5);	//OLED显示角度变量
        Delay_s(1);
    }
}

void Servo_Motor_SetAngle(float Angle)
{
    PWM_SetCompare_x(Servo_Motor_TIM_x, Servo_Motor_OC_x, Angle2PWM(Angle)); // PWM设置为角度值
}
