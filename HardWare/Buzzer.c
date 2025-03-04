#include "stm32f10x.h"                  // Device header
#include "Buzzer.h"
#include "Delay.h"

typedef enum
{
	BUZZER_OFF,
	BUZZER_ON,
} BUZZER_STATUS;

typedef struct
{
	BUZZER_ID ID;
	GPIO_TypeDef* PORT;
	uint16_t PIN;
	GPIOMode_TypeDef MODE;
	uint32_t _RCC;
	uint8_t Is_High_Active;        // 高电平有效
} BUZZER_HARDWARE;

typedef struct {
	BUZZER_ID ID;
	BUZZER_STATUS Status;
	BUZZER_HARDWARE HW;
} BUZZER_Data;

void Buzzer_On(BUZZER_Device* pDev);
void Buzzer_Off(BUZZER_Device* pDev);
void Buzzer_Revert(BUZZER_Device* pDev);
uint8_t Is_Buzzer_On(BUZZER_Device* pDev);


//--------------------------------------------------


static BUZZER_HARDWARE g_Buzzer_HWs[BUZZER_NUM] = {
	{
		.ID = BUZZER,
		.PORT = GPIOA,
		.PIN = GPIO_Pin_12,
		.MODE = GPIO_Mode_Out_PP,
		._RCC = RCC_APB2Periph_GPIOA,
		.Is_High_Active = 0	// 低电平有效
	},
};
static BUZZER_Data g_Buzzer_Datas[BUZZER_NUM];
static BUZZER_Device g_Buzzer_Devs[BUZZER_NUM];


BUZZER_Device* Drv_Buzzer_GetDevice(BUZZER_ID ID)
{
	for (int i = 0; i < sizeof(g_Buzzer_Devs) / sizeof(g_Buzzer_Devs[0]); i++)
	{
		BUZZER_Data* pData = (BUZZER_Data*)g_Buzzer_Devs[i].Priv_Data;
		if (pData == 0) return 0;

		if (pData->ID == ID)
			return &g_Buzzer_Devs[i];
	}

	return 0;
}

void Drv_Buzzer_Init(void)
{
	for (uint8_t i = 0; i < BUZZER_NUM; i++)
	{
		// Get Hardware
		BUZZER_HARDWARE hw;
		for (uint8_t j = 0; j < sizeof(g_Buzzer_HWs) / sizeof(g_Buzzer_HWs[0]); j++)
		{
			if (g_Buzzer_HWs[j].ID == (BUZZER_ID)i)
				hw = g_Buzzer_HWs[j];
		}

		// Data Init
		g_Buzzer_Datas[i].ID = (BUZZER_ID)i;
		g_Buzzer_Datas[i].Status = BUZZER_OFF;
		g_Buzzer_Datas[i].HW = hw;

		// Device Init
		g_Buzzer_Devs[i].Buzzer_On = Buzzer_On;
		g_Buzzer_Devs[i].Buzzer_Off = Buzzer_Off;
		g_Buzzer_Devs[i].Buzzer_Revert = Buzzer_Revert;
		g_Buzzer_Devs[i].Is_Buzzer_On = Is_Buzzer_On;
		g_Buzzer_Devs[i].Priv_Data = (void*)&g_Buzzer_Datas[i];

		// Hardware Init
		RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = hw.PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = hw.MODE;
		GPIO_Init(hw.PORT, &GPIO_InitStructure);

		// Buzzer Off
		Buzzer_Off((BUZZER_Device*)&g_Buzzer_Devs[i]);
	}
}


//--------------------------------------------------


void Buzzer_On(BUZZER_Device* pDev)
{
	BUZZER_Data* pData = (BUZZER_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	if (pData->HW.Is_High_Active)
		GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);
	else
		GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
}

void Buzzer_Off(BUZZER_Device* pDev)
{
	BUZZER_Data* pData = (BUZZER_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	if (pData->HW.Is_High_Active)
		GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
	else
		GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);

}

void Buzzer_Revert(BUZZER_Device* pDev)
{
	BUZZER_Data* pData = (BUZZER_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	if (GPIO_ReadInputDataBit(pData->HW.PORT, pData->HW.PIN))
		GPIO_ResetBits(pData->HW.PORT, pData->HW.PIN);
	else
		GPIO_SetBits(pData->HW.PORT, pData->HW.PIN);

}

uint8_t Is_Buzzer_On(BUZZER_Device* pDev)
{
	BUZZER_Data* pData = (BUZZER_Data*)pDev->Priv_Data;
	if (pData == 0) return 0;

	return pData->Status == BUZZER_ON;
}
