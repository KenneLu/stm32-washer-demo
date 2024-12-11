#include "stm32f10x.h" // Device header
#include "OLED.h"
#include "Encoder.h"
#include "TB6612.h"

int main(void)
{
	OLED_Init();
	Encoder_Init();
	OLED_ShowString(1, 1, "CNT:+00000Times");
	OLED_ShowString(2, 1, "Pressed:");
	while (1)
	{
		// OLED_ShowSignedNum(1, 5, Encoder_GetCount(), 5);
		// OLED_ShowNum(2, 10, (uint32_t)Encoder_Pressed(), 1);

		TB6612_Init();
		TB6612_Motor_Breathe();
	}
}
