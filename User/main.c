#include "stm32f10x.h" // Device header
#include "Drv_OLED.h"
#include "Drv_Encoder.h"
#include "Drv_Key.h"
#include "Menu.h"


int main(void)
{

	//-------Init-------

	OLED_Init();
	Drv_Key_Init();
	Timer_Init();
	Dev_Encoder_Init();
	Menu_Init();
	Menu_Washer_Power_On();

	//-------Main Loop-------

	while (1)
	{
		Menu_Washer_Run();
	}
}


// -------Interrupts-------

void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Drv_Key_Scan();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
