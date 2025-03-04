#include "stm32f10x.h"                  // Device header
#include "Drv_I2C_HW.h"
#include "Delay.h"


typedef struct
{
    I2C_HW_ID ID;
    I2C_TypeDef* PORT;
    uint32_t _RCC;
    uint16_t I2C_Mode;
    uint32_t I2C_ClockSpeed;
    uint16_t I2C_DutyCycle;
    uint16_t I2C_OwnAddress1;
    uint16_t I2C_Ack;
    uint16_t I2C_AcknowledgedAddress;
} I2C_HW_HARDWARE;

typedef struct {
    I2C_HW_ID ID;
    I2C_HW_HARDWARE HW;
} I2C_HW_Data;


void I2C_HW_GenerateSTART(I2C_HW_Device* pDev, FunctionalState NewState);
void I2C_HW_GenerateSTOP(I2C_HW_Device* pDev, FunctionalState NewState);
void I2C_HW_AcknowledgeConfig(I2C_HW_Device* pDev, FunctionalState NewState);
void I2C_HW_Send7bitAddress(I2C_HW_Device* pDev, uint8_t Address, uint8_t I2C_Direction);
void I2C_HW_SendData(I2C_HW_Device* pDev, uint8_t Data);
uint8_t I2C_HW_ReceiveData(I2C_HW_Device* pDev);
ErrorStatus I2C_HW_CheckEvent(I2C_HW_Device* pDev, uint32_t I2C_EVENT);


//--------------------------------------------------


static I2C_HW_HARDWARE g_I2C_HW_HWs[I2C_HW_NUM] = {
    {
        .ID = I2C_HW,
        .PORT = I2C2,
        ._RCC = RCC_APB1Periph_I2C2,
        .I2C_Mode = I2C_Mode_I2C,
        .I2C_ClockSpeed = 50000,            // 50KHz
        .I2C_DutyCycle = I2C_DutyCycle_2,   // 标准速度下强制1:1，此参数无效
        .I2C_OwnAddress1 = 0x00,            // 从机模式参数，随便填
        .I2C_Ack = I2C_Ack_Enable,          // 暂定，之后可改
        .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,    // 从机模式参数
    },
};
static I2C_HW_Data g_I2C_HW_Datas[I2C_HW_NUM];
static I2C_HW_Device g_I2C_HW_Devs[I2C_HW_NUM];


I2C_HW_Device* Drv_I2C_HW_GetDevice(I2C_HW_ID ID)
{
    for (int i = 0; i < sizeof(g_I2C_HW_Devs) / sizeof(g_I2C_HW_Devs[0]); i++)
    {
        I2C_HW_Data* pData = (I2C_HW_Data*)g_I2C_HW_Devs[i].Priv_Data;
        if (!pData) return 0;
        if (pData->ID == ID)
            return &g_I2C_HW_Devs[i];
    }

    return 0;
}

void Drv_I2C_HW_Init(void)
{
    for (uint8_t i = 0; i < I2C_HW_NUM; i++)
    {
        // Get Hardware
        I2C_HW_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_I2C_HW_HWs) / sizeof(g_I2C_HW_HWs[0]); j++)
        {
            if (g_I2C_HW_HWs[j].ID == (I2C_HW_ID)i)
                hw = g_I2C_HW_HWs[j];
        }

        // Data Init
        g_I2C_HW_Datas[i].ID = (I2C_HW_ID)i;
        g_I2C_HW_Datas[i].HW = hw;

        // Device Init
        g_I2C_HW_Devs[i].I2C_HW_GenerateSTART = I2C_HW_GenerateSTART;
        g_I2C_HW_Devs[i].I2C_HW_GenerateSTOP = I2C_HW_GenerateSTOP;
        g_I2C_HW_Devs[i].I2C_HW_AcknowledgeConfig = I2C_HW_AcknowledgeConfig;
        g_I2C_HW_Devs[i].I2C_HW_Send7bitAddress = I2C_HW_Send7bitAddress;
        g_I2C_HW_Devs[i].I2C_HW_SendData = I2C_HW_SendData;
        g_I2C_HW_Devs[i].I2C_HW_ReceiveData = I2C_HW_ReceiveData;
        g_I2C_HW_Devs[i].I2C_HW_CheckEvent = I2C_HW_CheckEvent;
        g_I2C_HW_Devs[i].Priv_Data = (void*)&g_I2C_HW_Datas[i];

        // Hardware Init
        RCC_APB1PeriphClockCmd(hw._RCC, ENABLE);
        I2C_InitTypeDef I2C_InitStructure;
        I2C_InitStructure.I2C_Mode = hw.I2C_Mode;
        I2C_InitStructure.I2C_ClockSpeed = hw.I2C_ClockSpeed;
        I2C_InitStructure.I2C_DutyCycle = hw.I2C_DutyCycle;
        I2C_InitStructure.I2C_OwnAddress1 = hw.I2C_OwnAddress1;
        I2C_InitStructure.I2C_Ack = hw.I2C_Ack;
        I2C_InitStructure.I2C_AcknowledgedAddress = hw.I2C_AcknowledgedAddress;
        I2C_Init(hw.PORT, &I2C_InitStructure);

        // Enable
        I2C_Cmd(hw.PORT, ENABLE);
    }
}


//--------------------------------------------------


void I2C_HW_GenerateSTART(I2C_HW_Device* pDev, FunctionalState NewState)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return;
    I2C_GenerateSTART(pData->HW.PORT, NewState);
}

void I2C_HW_GenerateSTOP(I2C_HW_Device* pDev, FunctionalState NewState)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return;
    I2C_GenerateSTOP(pData->HW.PORT, NewState);
}

void I2C_HW_AcknowledgeConfig(I2C_HW_Device* pDev, FunctionalState NewState)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return;
    I2C_AcknowledgeConfig(pData->HW.PORT, NewState);
}

void I2C_HW_Send7bitAddress(I2C_HW_Device* pDev, uint8_t Address, uint8_t I2C_Direction)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return;
    I2C_Send7bitAddress(pData->HW.PORT, Address, I2C_Direction);
}

void I2C_HW_SendData(I2C_HW_Device* pDev, uint8_t Data)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return;
    I2C_SendData(pData->HW.PORT, Data);
}

uint8_t I2C_HW_ReceiveData(I2C_HW_Device* pDev)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    I2C_ReceiveData(pData->HW.PORT);
}

ErrorStatus I2C_HW_CheckEvent(I2C_HW_Device* pDev, uint32_t I2C_EVENT)
{
    I2C_HW_Data* pData = (I2C_HW_Data*)pDev->Priv_Data;
    if (pData == 0) return ERROR;
    return I2C_CheckEvent(pData->HW.PORT, I2C_EVENT);
}

