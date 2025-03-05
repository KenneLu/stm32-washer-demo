#include "stm32f10x.h"                  // Device header
#include "MySPI.h"


typedef enum
{
	SPI_HWSPI2,
	SPI_HW_NUM,
} HWSPI_ID;

typedef enum
{
	SPI_SW_1,
	SPI_SW_NUM,
} SWSPI_ID;

typedef struct
{
	HWSPI_ID ID;
	GPIO_TypeDef* GPIO_PORT;
	uint16_t GPIO_PIN_SCK;
	uint16_t GPIO_PIN_MOSI;
	uint16_t GPIO_PIN_MISO;
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
	uint8_t IS_INIT;
} HWSPI_HARDWARE;

typedef struct
{
	SPI_DEVICE_ID ID;
	GPIO_TypeDef* GPIO_PORT;
	uint16_t GPIO_PIN_SS;
	GPIOMode_TypeDef GPIO_MODE_SS;
	uint32_t GPIO_RCC;
	uint8_t USE_HWSPI;	// 是否使用硬件SPI
	uint8_t SPI_ID;		// 可能是硬件SPI，也可能是软件SPI
	void* SPI_HARDWARE;	// 可能是硬件SPI，也可能是软件SPI
} SPI_DEVICE_HARDWARE;

typedef struct {
	SPI_DEVICE_ID ID;
	SPI_DEVICE_HARDWARE HW;
} SPI_DEVICE_Data;


void Write_SS(SPI_Device* pDev, uint8_t BitValue);
void Start(SPI_Device* pDev);
void Stop(SPI_Device* pDev);
uint8_t SwapData(SPI_Device* pDev, uint8_t DataSend);


//--------------------------------------------------


static HWSPI_HARDWARE g_HWSPI_HWs[SPI_HW_NUM] = {
	{
		.ID = SPI_HWSPI2,
		.GPIO_PORT = GPIOB,
		.GPIO_PIN_SCK = GPIO_Pin_13,
		.GPIO_PIN_MISO = GPIO_Pin_14,
		.GPIO_PIN_MOSI = GPIO_Pin_15,
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
		.IS_INIT = 0,
	},
};


static SPI_DEVICE_HARDWARE g_SPI_Device_HWs[SPI_DEVICE_NUM] = {
	{
		.ID = SPI_DEVICE_W25Q64,
		.GPIO_PORT = GPIOB,
		.GPIO_PIN_SS = GPIO_Pin_12,
		.GPIO_MODE_SS = GPIO_Mode_Out_PP,
		.GPIO_RCC = RCC_APB2Periph_GPIOB,
		.USE_HWSPI = 1,
		.SPI_ID = SPI_HWSPI2,
	},
};
static SPI_DEVICE_Data g_SPI_Datas[SPI_DEVICE_NUM];
static SPI_Device g_SPI_Devs[SPI_DEVICE_NUM];


//--------------------------------------------------


SPI_Device* Drv_SPI_GetDevice(SPI_DEVICE_ID ID)
{
	for (int i = 0; i < sizeof(g_SPI_Devs) / sizeof(g_SPI_Devs[0]); i++)
	{
		SPI_DEVICE_Data* pData = (SPI_DEVICE_Data*)g_SPI_Devs[i].Priv_Data;
		if (pData == 0)
			return 0;
		if (pData->ID == ID)
			return &g_SPI_Devs[i];
	}
	return 0;
}

void Drv_SPI_Init(void)
{
	// Device Get SPI(HW/SW)
	for (uint8_t i = 0; i < SPI_DEVICE_NUM; i++)
	{
		if (g_SPI_Device_HWs[i].USE_HWSPI == 1)
		{
			// Hardware SPI
			for (uint8_t j = 0; j < SPI_HW_NUM; j++)
			{
				if (g_HWSPI_HWs[j].ID == g_SPI_Device_HWs[i].SPI_ID)
					g_SPI_Device_HWs[i].SPI_HARDWARE = &g_HWSPI_HWs[j];
			}
		}
		else
		{
			// TODO: Software SPI
		}
	}

	for (uint8_t i = 0; i < SPI_DEVICE_NUM; i++)
	{

		// Get Device Hardware
		SPI_DEVICE_HARDWARE hw;
		for (uint8_t j = 0; j < SPI_DEVICE_NUM; j++)
		{
			if (g_SPI_Device_HWs[j].ID == (SPI_DEVICE_ID)i)
				hw = g_SPI_Device_HWs[j];
		}

		// Data Init
		g_SPI_Datas[i].ID = (SPI_DEVICE_ID)i;
		g_SPI_Datas[i].HW = hw;

		// Device Init
		g_SPI_Devs[i].Start = Start;
		g_SPI_Devs[i].Stop = Stop;
		g_SPI_Devs[i].SwapData = SwapData;
		g_SPI_Devs[i].Priv_Data = (void*)&g_SPI_Datas[i];

		// Hardware Init

		// Hardware Init: Device GPIO 
		RCC_APB2PeriphClockCmd(hw.GPIO_RCC, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = hw.GPIO_MODE_SS;
		GPIO_InitStructure.GPIO_Pin = hw.GPIO_PIN_SS;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(hw.GPIO_PORT, &GPIO_InitStructure);

		// Hardware Init: Device SPI
		if (hw.USE_HWSPI == 1)
		{
			HWSPI_HARDWARE* pHWSPI = (HWSPI_HARDWARE*)hw.SPI_HARDWARE;

			// GPIO Init
			RCC_APB2PeriphClockCmd(pHWSPI->GPIO_RCC, ENABLE);
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = pHWSPI->GPIO_MODE_SCK;
			GPIO_InitStructure.GPIO_Pin = pHWSPI->GPIO_PIN_SCK;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(pHWSPI->GPIO_PORT, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = pHWSPI->GPIO_MODE_MISO;
			GPIO_InitStructure.GPIO_Pin = pHWSPI->GPIO_PIN_MISO;
			GPIO_Init(pHWSPI->GPIO_PORT, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = pHWSPI->GPIO_MODE_MOSI;
			GPIO_InitStructure.GPIO_Pin = pHWSPI->GPIO_PIN_MOSI;
			GPIO_Init(pHWSPI->GPIO_PORT, &GPIO_InitStructure);

			// HWSPI Init
			RCC_APB1PeriphClockCmd(pHWSPI->SPI_RCC, ENABLE);
			SPI_InitTypeDef SPI_InitStructure;
			SPI_InitStructure.SPI_Mode = pHWSPI->SPI_Mode;
			SPI_InitStructure.SPI_Direction = pHWSPI->SPI_Direction;
			SPI_InitStructure.SPI_DataSize = pHWSPI->SPI_DataSize;
			SPI_InitStructure.SPI_FirstBit = pHWSPI->SPI_FirstBit;
			SPI_InitStructure.SPI_BaudRatePrescaler = pHWSPI->SPI_BaudRatePrescaler;
			SPI_InitStructure.SPI_CPOL = pHWSPI->SPI_CPOL;
			SPI_InitStructure.SPI_CPHA = pHWSPI->SPI_CPHA;
			SPI_InitStructure.SPI_NSS = pHWSPI->SPI_NSS;
			SPI_InitStructure.SPI_CRCPolynomial = pHWSPI->SPI_CRCPolynomial;
			SPI_Init(pHWSPI->SPI_PORT, &SPI_InitStructure);

			// Enable
			SPI_Cmd(pHWSPI->SPI_PORT, ENABLE);

			// SPI Init Finish
			pHWSPI->IS_INIT = 1;
		}

		// Default Settings
		Write_SS(&g_SPI_Devs[i], 1); // 默认不选中从机
	}
}


//--------------------------------------------------


void Write_SS(SPI_Device* pDev, uint8_t BitValue)
{
	SPI_DEVICE_Data* pData = (SPI_DEVICE_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	GPIO_WriteBit(pData->HW.GPIO_PORT, pData->HW.GPIO_PIN_SS, (BitAction)BitValue);
}

void Start(SPI_Device* pDev)
{
	Write_SS(pDev, 0);
}

void Stop(SPI_Device* pDev)
{
	Write_SS(pDev, 1);
}

uint8_t SwapData(SPI_Device* pDev, uint8_t DataSend)
{
	SPI_DEVICE_Data* pData = (SPI_DEVICE_Data*)pDev->Priv_Data;
	if (pData == 0)
		return 0;

	if (pData->HW.USE_HWSPI == 1)
	{
		HWSPI_HARDWARE* pHWSPI = (HWSPI_HARDWARE*)pData->HW.SPI_HARDWARE;
		while (SPI_I2S_GetFlagStatus(pHWSPI->SPI_PORT, SPI_I2S_FLAG_TXE) != SET);

		SPI_I2S_SendData(pHWSPI->SPI_PORT, DataSend);

		while (SPI_I2S_GetFlagStatus(pHWSPI->SPI_PORT, SPI_I2S_FLAG_RXNE) != SET);

		return SPI_I2S_ReceiveData(pHWSPI->SPI_PORT);
	}
	else
	{
		// TODO: Software SPI
		return 0;
	}
}
