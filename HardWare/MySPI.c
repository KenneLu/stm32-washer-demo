#include "stm32f10x.h"                  // Device header
#include "MySPI.h"

void MySPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(SPI_GOIO_x, SPI_GPIO_PIN_SS, (BitAction)BitValue);
}

void MySPI_Init(void)
{
	RCC_APB2PeriphClockCmd(SPI_GPIO_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(SPI_SPI_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = SPI_GOIO_MODE_SS;
	GPIO_InitStructure.GPIO_Pin = SPI_GPIO_PIN_SS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GOIO_x, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = SPI_GOIO_MODE_SCK;
	GPIO_InitStructure.GPIO_Pin = SPI_GPIO_PIN_SCK;
	GPIO_Init(SPI_GOIO_x, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = SPI_GOIO_MODE_MISO;
	GPIO_InitStructure.GPIO_Pin = SPI_GPIO_PIN_MISO;
	GPIO_Init(SPI_GOIO_x, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = SPI_GOIO_MODE_MOSI;
	GPIO_InitStructure.GPIO_Pin = SPI_GPIO_PIN_MOSI;
	GPIO_Init(SPI_GOIO_x, &GPIO_InitStructure);

	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; // 主机
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 双线全双工
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // 8位数据帧
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // 高位现行
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; // 波特率分频
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; // 默认模式0
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; // 默认模式0
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // 软件NSS
	SPI_InitStructure.SPI_CRCPolynomial = 7; // 默认值7（无需过多了解）
	SPI_Init(SPI_SPI_x, &SPI_InitStructure);

	SPI_Cmd(SPI_SPI_x, ENABLE); // 使能SPI1

	MySPI_W_SS(1); // 默认不选中从机
}

void MySPI_Start(void)
{
	MySPI_W_SS(0);
}

void MySPI_Stop(void)
{
	MySPI_W_SS(1);
}

uint8_t MySPI_SwapByte(uint8_t ByteSend)
{
	while (SPI_I2S_GetFlagStatus(SPI_SPI_x, SPI_I2S_FLAG_TXE) != SET);

	SPI_I2S_SendData(SPI_SPI_x, ByteSend);

	while (SPI_I2S_GetFlagStatus(SPI_SPI_x, SPI_I2S_FLAG_RXNE) != SET);

	return SPI_I2S_ReceiveData(SPI_SPI_x);
}
