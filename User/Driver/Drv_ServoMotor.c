#include "stm32f10x.h"                  // Device header
#include "Drv_PWM.h"
#include "Drv_ServoMotor.h"


typedef struct
{
    SERVOMOTOR_ID ID;
    PWM_TIM* TIM;
    OC_ID OC;
} SERVOMOTOR_HARDWARE;

typedef struct {
    SERVOMOTOR_ID ID;
    SERVOMOTOR_HARDWARE HW;
} SERVOMOTOR_Data;

void SetAngle(SERVOMOTOR_Device* pDev, float Angle);


//--------------------------------------------------


static SERVOMOTOR_HARDWARE g_ServoMotor_HWs[SERVOMOTOR_NUM] = {
    {
        .ID = SERVOMOTOR,
        .TIM = &PWM_TIM1,
        .OC = OC4,
    },
};
static SERVOMOTOR_Data g_ServoMotor_Datas[SERVOMOTOR_NUM];
static SERVOMOTOR_Device g_ServoMotor_Devs[SERVOMOTOR_NUM];


//--------------------------------------------------


SERVOMOTOR_Device* Drv_ServoMotor_GetDevice(SERVOMOTOR_ID ID)
{
    for (int i = 0; i < sizeof(g_ServoMotor_Devs) / sizeof(g_ServoMotor_Devs[0]); i++)
    {
        SERVOMOTOR_Data* pData = (SERVOMOTOR_Data*)g_ServoMotor_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_ServoMotor_Devs[i];
    }
    return 0;
}

void Drv_ServoMotor_Init(void)
{
    for (uint8_t i = 0; i < SERVOMOTOR_NUM; i++)
    {
        // Get Hardware
        SERVOMOTOR_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_ServoMotor_HWs) / sizeof(g_ServoMotor_HWs[0]); j++)
        {
            if (g_ServoMotor_HWs[j].ID == (SERVOMOTOR_ID)i)
                hw = g_ServoMotor_HWs[j];
        }

        // Data Init
        g_ServoMotor_Datas[i].ID = (SERVOMOTOR_ID)i;
        g_ServoMotor_Datas[i].HW = hw;

        // Device Init
        g_ServoMotor_Devs[i].SetAngle = SetAngle;
        g_ServoMotor_Devs[i].Priv_Data = (void*)&g_ServoMotor_Datas[i];

        // Hardware Init
        PWM_TIM_Init(hw.TIM, 0);
        PWM_GPIO_Init(hw.TIM, hw.OC);
    }
}


//--------------------------------------------------

uint16_t Angle2PWM(float Angle)
{
    // 计算角度对应的 PWM 值
    return (uint16_t)(Angle / 180 * 2000 + 500);
}

void SetAngle(SERVOMOTOR_Device* pDev, float Angle)
{
    SERVOMOTOR_Data* pData = (SERVOMOTOR_Data*)pDev->Priv_Data;
    if (pData == 0) return;

    PWM_SetCompare_x(pData->HW.TIM, pData->HW.OC, Angle2PWM(Angle)); // PWM设置为角度值
}


// #include "Drv_OLED.h"
// #include "Sys_Delay.h"
// float ServoBreath[] = { 0, 30, 60, 90, 120, 150, 180 };	// 定义转动速度数组
// void Servo_Motorr_Breathe(void)
// {
//     OLED_ShowString_Easy(3, 1, "Angle:");
//     OLED_ShowString_Easy(4, 1, "PWM:");
//     for (int i = 0; i < sizeof(ServoBreath) / sizeof(ServoBreath[0]); i++)
//     {
//         Servo_Motor_SetAngle(ServoBreath[i]);  // 设置直流电机的速度为速度变量
//         OLED_ShowSignedNum_Easy(3, 7, ServoBreath[i], 3);	    // OLED显示速度变量
//         OLED_ShowNum_Easy(4, 5, Angle2PWM(ServoBreath[i]), 5);	//OLED显示角度变量
//         Delay_s(1);
//     }
// }
