#include "stm32f10x.h"                  // Device header
#include "Buzzer.h"
#include "Delay.h"

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(BUZZER_GPIO_RCC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = BUZZER_GOIO_MODE;
	GPIO_InitStructure.GPIO_Pin = BUZZER_GPIO_PIN_IO;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BUZZER_GOIO_x, &GPIO_InitStructure);

	GPIO_SetBits(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO);
}

void Buzzer_Breathe(void)
{
	Buzzer_Revert();
	Delay_ms(1000);
}

void Buzzer_On(uint8_t On)
{
	if (On)
		GPIO_ResetBits(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO);
	else
		GPIO_SetBits(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO);
}

void Buzzer_Revert(void)
{
	if (GPIO_ReadInputDataBit(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO))
		GPIO_ResetBits(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO);
	else
		GPIO_SetBits(BUZZER_GOIO_x, BUZZER_GPIO_PIN_IO);
}
