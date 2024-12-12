#include "stm32f10x.h"                  // Device header
#include "Buzzer.h"
#include "Delay.h"

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(SPI_GPIO_RCC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = SPI_GOIO_MODE;
	GPIO_InitStructure.GPIO_Pin = SPI_GPIO_PIN_IO;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GOIO_x, &GPIO_InitStructure);

	GPIO_SetBits(SPI_GOIO_x, SPI_GPIO_PIN_IO);
}

void Buzzer_Breathe(void)
{
	Buzzer_Revert();
	Delay_ms(1000);
}

void Buzzer_On(uint8_t On)
{
	if (On)
		GPIO_ResetBits(SPI_GOIO_x, SPI_GPIO_PIN_IO);
	else
		GPIO_SetBits(SPI_GOIO_x, SPI_GPIO_PIN_IO);
}

void Buzzer_Revert(void)
{
	if (GPIO_ReadInputDataBit(SPI_GOIO_x, SPI_GPIO_PIN_IO))
		GPIO_ResetBits(SPI_GOIO_x, SPI_GPIO_PIN_IO);
	else
		GPIO_SetBits(SPI_GOIO_x, SPI_GPIO_PIN_IO);
}
