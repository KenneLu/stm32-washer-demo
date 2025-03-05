#include "stm32f10x.h"                  // Device header
#include "Sys_Delay.h"
#include "Drv_TB6612.h"
#include "Drv_PWM.h"
#include "Drv_OLED.h"


typedef struct
{
    TB6612_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN_AN1;
    uint16_t PIN_AN2;
    GPIOMode_TypeDef MODE;
    uint32_t _RCC;
    PWM_TIM* TIM;
    OC_ID OC;
} TB6612_HARDWARE;

typedef struct {
    TB6612_ID ID;
    TB6612_HARDWARE HW;
} TB6612_Data;

void SetSpeed(TB6612_Device* pDev, int8_t Speed);


//--------------------------------------------------


static TB6612_HARDWARE g_TB6612_HWs[TB6612_NUM] = {
    {
        .ID = TB6612,
        .PORT = GPIOB,
        .PIN_AN1 = GPIO_Pin_0,
        .PIN_AN2 = GPIO_Pin_1,
        .MODE = GPIO_Mode_Out_PP,
        ._RCC = RCC_APB2Periph_GPIOB,
        .TIM = &PWM_TIM1,
        .OC = OC1,
    },
};
static TB6612_Data g_TB6612_Datas[TB6612_NUM];
static TB6612_Device g_TB6612_Devs[TB6612_NUM];


//--------------------------------------------------


TB6612_Device* Drv_TB6612_GetDevice(TB6612_ID ID)
{
    for (int i = 0; i < sizeof(g_TB6612_Devs) / sizeof(g_TB6612_Devs[0]); i++)
    {
        TB6612_Data* pData = (TB6612_Data*)g_TB6612_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_TB6612_Devs[i];
    }
    return 0;
}

void Drv_TB6612_Init(void)
{
    for (uint8_t i = 0; i < TB6612_NUM; i++)
    {
        // Get Hardware
        TB6612_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_TB6612_HWs) / sizeof(g_TB6612_HWs[0]); j++)
        {
            if (g_TB6612_HWs[j].ID == (TB6612_ID)i)
                hw = g_TB6612_HWs[j];
        }

        // Data Init
        g_TB6612_Datas[i].ID = (TB6612_ID)i;
        g_TB6612_Datas[i].HW = hw;

        // Device Init
        g_TB6612_Devs[i].SetSpeed = SetSpeed;
        g_TB6612_Devs[i].Priv_Data = (void*)&g_TB6612_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = hw.MODE;
        GPIO_InitStructure.GPIO_Pin = hw.PIN_AN1 | hw.PIN_AN2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);

        PWM_TIM_Init(hw.TIM, 0);
        PWM_GPIO_Init(hw.TIM, hw.OC);
    }
}


//--------------------------------------------------

uint16_t Speed2PWM(float Speed)
{
    // 计算角度对应的 PWM 值
    return (uint16_t)(Speed * 20000 / 100);
}

void SetSpeed(TB6612_Device* pDev, int8_t Speed)
{
    TB6612_Data* pData = (TB6612_Data*)pDev->Priv_Data;
    if (pData == 0)
        return;

    static int8_t Speed_Cur = 0;
    if (Speed_Cur != Speed)
    {
        Speed_Cur = Speed;
        if (Speed >= 0) // 正转
        {
            GPIO_SetBits(pData->HW.PORT, pData->HW.PIN_AN1);      // 置高电平
            GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN_AN2);    // 置低电平，设置方向为正转
            PWM_SetCompare_x(pData->HW.TIM, pData->HW.OC, Speed2PWM(Speed)); // PWM设置为速度值
        }
        else    // 反转
        {
            GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN_AN1);	// 置低电平
            GPIO_SetBits(pData->HW.PORT, pData->HW.PIN_AN2);	    // 置高电平，设置方向为反转
            PWM_SetCompare_x(pData->HW.TIM, pData->HW.OC, Speed2PWM(-Speed));    //PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
        }
    }
}



// int8_t MotorBreath[] = { 0, 20, 30, 40, 30, 20, 0, -20, -30, -40, -30, -20 };	// 定义转动速度数组
// void TB6612_Motor_Breathe(void)
// {
//     OLED_ShowString_Easy(1, 1, "Speed:+000");
//     OLED_ShowString_Easy(2, 1, "PWM:");
//     for (uint8_t i = 0; i < sizeof(MotorBreath) / sizeof(MotorBreath[0]); i++)
//     {
//         TB6612_Motor_SetSpeed(MotorBreath[i]);  // 设置直流电机的速度为速度变量
//         OLED_ShowSignedNum_Easy(1, 7, MotorBreath[i], 3);	    // OLED显示速度变量
//         OLED_ShowNum_Easy(2, 5, Speed2PWM(MotorBreath[i]), 5);	//OLED显示角度变量
//         Delay_s(1);
//     }
// }
