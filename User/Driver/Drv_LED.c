#include "stm32f10x.h"                  // Device header
#include "Drv_LED.h"
#include "Sys_Delay.h"

typedef enum
{
    LED_OFF,
    LED_ON,
} LED_STATUS;

typedef struct
{
    LED_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN;
    GPIOMode_TypeDef MODE;
    uint32_t _RCC;
    uint8_t Is_High_Active;        // 高电平有效
} LED_HARDWARE;

typedef struct {
    LED_ID ID;
    LED_STATUS Status;
    LED_HARDWARE HW;
} LED_Data;

void On_LED(LED_Device* pDev);
void Off_LED(LED_Device* pDev);
void Revert_LED(LED_Device* pDev);
uint8_t Is_On_LED(LED_Device* pDev);


//--------------------------------------------------


static LED_HARDWARE g_LED_HWs[LED_NUM] = {
    {
        .ID = LED_RED,
        .PORT = GPIOA,
        .PIN = GPIO_Pin_3,
        .MODE = GPIO_Mode_Out_PP,
        ._RCC = RCC_APB2Periph_GPIOA,
        .Is_High_Active = 1 // 高电平有效
    },
    {
        .ID = LED_BLUE,
        .PORT = GPIOA,
        .PIN = GPIO_Pin_2,
        .MODE = GPIO_Mode_Out_PP,
        ._RCC = RCC_APB2Periph_GPIOA,
        .Is_High_Active = 1 // 高电平有效
    },
};
static LED_Data g_LED_Datas[LED_NUM];
static LED_Device g_LED_Devs[LED_NUM];


//--------------------------------------------------


LED_Device* Drv_LED_GetDevice(LED_ID ID)
{
    for (int i = 0; i < sizeof(g_LED_Devs) / sizeof(g_LED_Devs[0]); i++)
    {
        LED_Data* pData = (LED_Data*)g_LED_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_LED_Devs[i];
    }
    return 0;
}

void Drv_LED_Init(void)
{
    for (uint8_t i = 0; i < LED_NUM; i++)
    {
        // Get Hardware
        LED_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_LED_HWs) / sizeof(g_LED_HWs[0]); j++)
        {
            if (g_LED_HWs[j].ID == (LED_ID)i)
                hw = g_LED_HWs[j];
        }

        // Data Init
        g_LED_Datas[i].ID = (LED_ID)i;
        g_LED_Datas[i].Status = LED_OFF;
        g_LED_Datas[i].HW = hw;

        // Device Init
        g_LED_Devs[i].On_LED = On_LED;
        g_LED_Devs[i].Off_LED = Off_LED;
        g_LED_Devs[i].Revert_LED = Revert_LED;
        g_LED_Devs[i].Is_On_LED = Is_On_LED;
        g_LED_Devs[i].Priv_Data = (void*)&g_LED_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = hw.PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = hw.MODE;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);

        // LED Off_LED
        Off_LED((LED_Device*)&g_LED_Devs[i]);
    }
}


//--------------------------------------------------


void On_LED(LED_Device* pDev)
{
    LED_Data* pData = (LED_Data*)pDev->Priv_Data;
    if (pData == 0) return;

    if (pData->HW.Is_High_Active)
        GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);
    else
        GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
}

void Off_LED(LED_Device* pDev)
{
    LED_Data* pData = (LED_Data*)pDev->Priv_Data;
    if (pData == 0) return;

    if (pData->HW.Is_High_Active)
        GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
    else
        GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);

}

void Revert_LED(LED_Device* pDev)
{
    LED_Data* pData = (LED_Data*)pDev->Priv_Data;
    if (pData == 0) return;

    if (GPIO_ReadInputDataBit(pData->HW.PORT, pData->HW.PIN))
        GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
    else
        GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);

}

uint8_t Is_On_LED(LED_Device* pDev)
{
    LED_Data* pData = (LED_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;

    return pData->Status == LED_ON;
}
