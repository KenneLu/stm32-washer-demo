#include "stm32f10x.h"                  // Device header
#include "MySPI.h"


typedef struct
{
	SPI_HW_ID ID;
	GPIO_TypeDef* GPIO_PORT;
	uint16_t GPIO_PIN_SS;
	uint16_t GPIO_PIN_SCK;
	uint16_t GPIO_PIN_MOSI;
	uint16_t GPIO_PIN_MISO;
	GPIOMode_TypeDef GPIO_MODE_SS;
	GPIOMode_TypeDef GPIO_MODE_SCK;
	GPIOMode_TypeDef GPIO_MODE_MOSI;
	GPIOMode_TypeDef GPIO_MODE_MISO;
	uint32_t GPIO_RCC;
	SPI_TypeDef* SPI_PORT;
	uint32_t SPI_RCC;
	uint16_t SPI_Mode;
	uint16_t SPI_Direction;
	uint16_t SPI_DataSize;
	uint16_t SPI_FirstBit;
	uint16_t SPI_BaudRatePrescaler;
	uint16_t SPI_CPOL;
	uint16_t SPI_CPHA;
	uint16_t SPI_NSS;
	uint16_t SPI_CRCPolynomial;
} SPI_HW_HARDWARE;

typedef struct {
	SPI_HW_ID ID;
	SPI_HW_HARDWARE HW;
} SPI_HW_Data;


void Write_SS(SPI_HW_Device* pDev, uint8_t BitValue);
void Start(SPI_HW_Device* pDev);
void Stop(SPI_HW_Device* pDev);
uint8_t SwapData(SPI_HW_Device* pDev, uint8_t DataSend);


//--------------------------------------------------


static SPI_HW_HARDWARE g_SPI_HW_HWs[SPI_HW_NUM] = {
	{
		.ID = SPI_HW,
		.GPIO_PORT = GPIOB,
		.GPIO_PIN_SS = GPIO_Pin_12,
		.GPIO_PIN_SCK = GPIO_Pin_13,
		.GPIO_PIN_MISO = GPIO_Pin_14,
		.GPIO_PIN_MOSI = GPIO_Pin_15,
		.GPIO_MODE_SS = GPIO_Mode_Out_PP,
		.GPIO_MODE_SCK = GPIO_Mode_AF_PP,
		.GPIO_MODE_MISO = GPIO_Mode_IPU,
		.GPIO_MODE_MOSI = GPIO_Mode_AF_PP,
		.GPIO_RCC = RCC_APB2Periph_GPIOB,
		.SPI_PORT = SPI2,
		.SPI_RCC = RCC_APB1Periph_SPI2,
		.SPI_Mode = SPI_Mode_Master, // 主机
		.SPI_Direction = SPI_Direction_2Lines_FullDuplex, // 双线全双工
		.SPI_DataSize = SPI_DataSize_8b, // 8位数据帧
		.SPI_FirstBit = SPI_FirstBit_MSB, // 高位现行
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128, // 波特率分频
		.SPI_CPOL = SPI_CPOL_Low, // 默认模式0
		.SPI_CPHA = SPI_CPHA_1Edge, // 默认模式0
		.SPI_NSS = SPI_NSS_Soft, // 软件NSS
		.SPI_CRCPolynomial = 7, // 默认值7（无需过多了解）
	},
};
static SPI_HW_Data g_SPI_HW_Datas[SPI_HW_NUM];
static SPI_HW_Device g_SPI_HW_Devs[SPI_HW_NUM];


//--------------------------------------------------


SPI_HW_Device* Drv_SPI_HW_GetDevice(SPI_HW_ID ID)
{
	for (int i = 0; i < sizeof(g_SPI_HW_Devs) / sizeof(g_SPI_HW_Devs[0]); i++)
	{
		SPI_HW_Data* pData = (SPI_HW_Data*)g_SPI_HW_Devs[i].Priv_Data;
		if (pData == 0)
			return 0;
		if (pData->ID == ID)
			return &g_SPI_HW_Devs[i];
	}
	return 0;
}

void Drv_SPI_HW_Init(void)
{
	for (uint8_t i = 0; i < SPI_HW_NUM; i++)
	{
		// Get Hardware
		SPI_HW_HARDWARE hw;
		for (uint8_t j = 0; j < sizeof(g_SPI_HW_HWs) / sizeof(g_SPI_HW_HWs[0]); j++)
		{
			if (g_SPI_HW_HWs[j].ID == (SPI_HW_ID)i)
				hw = g_SPI_HW_HWs[j];
		}

		// Data Init
		g_SPI_HW_Datas[i].ID = (SPI_HW_ID)i;
		g_SPI_HW_Datas[i].HW = hw;

		// Device Init
		g_SPI_HW_Devs[i].Start = Start;
		g_SPI_HW_Devs[i].Stop = Stop;
		g_SPI_HW_Devs[i].SwapData = SwapData;;
		g_SPI_HW_Devs[i].Priv_Data = (void*)&g_SPI_HW_Datas[i];

		// Hardware Init
		RCC_APB2PeriphClockCmd(hw.GPIO_RCC, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = hw.GPIO_MODE_SS;
		GPIO_InitStructure.GPIO_Pin = hw.GPIO_PIN_SS;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(hw.GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = hw.GPIO_MODE_SCK;
		GPIO_InitStructure.GPIO_Pin = hw.GPIO_PIN_SCK;
		GPIO_Init(hw.GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = hw.GPIO_MODE_MISO;
		GPIO_InitStructure.GPIO_Pin = hw.GPIO_PIN_MISO;
		GPIO_Init(hw.GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = hw.GPIO_MODE_MOSI;
		GPIO_InitStructure.GPIO_Pin = hw.GPIO_PIN_MOSI;
		GPIO_Init(hw.GPIO_PORT, &GPIO_InitStructure);

		RCC_APB1PeriphClockCmd(hw.SPI_RCC, ENABLE);
		SPI_InitTypeDef SPI_InitStructure;
		SPI_InitStructure.SPI_Mode = hw.SPI_Mode;
		SPI_InitStructure.SPI_Direction = hw.SPI_Direction;
		SPI_InitStructure.SPI_DataSize = hw.SPI_DataSize;
		SPI_InitStructure.SPI_FirstBit = hw.SPI_FirstBit;
		SPI_InitStructure.SPI_BaudRatePrescaler = hw.SPI_BaudRatePrescaler;
		SPI_InitStructure.SPI_CPOL = hw.SPI_CPOL;
		SPI_InitStructure.SPI_CPHA = hw.SPI_CPHA;
		SPI_InitStructure.SPI_NSS = hw.SPI_NSS;
		SPI_InitStructure.SPI_CRCPolynomial = hw.SPI_CRCPolynomial;
		SPI_Init(hw.SPI_PORT, &SPI_InitStructure);

		// Enable
		SPI_Cmd(hw.SPI_PORT, ENABLE);

		// Default Settings
		Write_SS(&g_SPI_HW_Devs[i], 1); // 默认不选中从机
	}
}


//--------------------------------------------------




void Write_SS(SPI_HW_Device* pDev, uint8_t BitValue)
{
	SPI_HW_Data* pData = (SPI_HW_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	GPIO_WriteBit(pData->HW.GPIO_PORT, pData->HW.GPIO_PIN_SS, (BitAction)BitValue);
}

void Start(SPI_HW_Device* pDev)
{
	Write_SS(pDev, 0);
}

void Stop(SPI_HW_Device* pDev)
{
	Write_SS(pDev, 1);
}

uint8_t SwapData(SPI_HW_Device* pDev, uint8_t DataSend)
{
	SPI_HW_Data* pData = (SPI_HW_Data*)pDev->Priv_Data;
	if (pData == 0)
		return 0;

	while (SPI_I2S_GetFlagStatus(pData->HW.SPI_PORT, SPI_I2S_FLAG_TXE) != SET);

	SPI_I2S_SendData(pData->HW.SPI_PORT, DataSend);

	while (SPI_I2S_GetFlagStatus(pData->HW.SPI_PORT, SPI_I2S_FLAG_RXNE) != SET);

	return SPI_I2S_ReceiveData(pData->HW.SPI_PORT);
}
