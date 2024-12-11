#include "stm32f10x.h"                  // Device header
#include "TB6612.h"
#include "PWM.h"
#include "Delay.h"
#include "OLED.h"

int8_t Speed;		// 定义速度变量[-100, 100], >20起转
int8_t MotorBreath[12] = { 0, 20, 30, 40, 30, 20, 0, -20, -30, -40, -30, -20 };	// 定义转动速度数组

void TB6612_Init(void)
{
    RCC_APB2PeriphClockCmd(TB6612_GPIO_RCC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = TB6612_GOIO_MODE;
    GPIO_InitStructure.GPIO_Pin = TB6612_GOIO_PIN_AIN1 | TB6612_GOIO_PIN_AIN2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TB6612_GOIO_x, &GPIO_InitStructure);

    PWM_Init();

    OLED_ShowString(3, 1, "Speed:+000");
}

void TB6612_Motor_Breathe(void)
{
    for (uint8_t i = 0; i < 12; i++)
    {
        int8_t TmpSpeed = MotorBreath[i];
        TB6612_Motor_SetSpeed(TmpSpeed);  // 设置直流电机的速度为速度变量
        OLED_ShowSignedNum(3, 7, TmpSpeed, 3);	    // OLED显示速度变量
        Delay_s(1);
    }
}

void TB6612_Motor_SetSpeed(int8_t Speed)
{
    if (Speed >= 0) // 正转
    {
        GPIO_SetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN1);      // 置高电平
        GPIO_ResetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN2);    // 置低电平，设置方向为正转
        PWM_SetCompare1(Speed); // PWM设置为速度值
    }
    else    // 反转
    {
        GPIO_ResetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN1);	// 置低电平
        GPIO_SetBits(TB6612_GOIO_x, TB6612_GOIO_PIN_AIN2);	    // 置高电平，设置方向为反转
        PWM_SetCompare1(-Speed);    //PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
    }
}