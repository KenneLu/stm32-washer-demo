#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "Encoder.h"
#include "TB6612.h"
#include "MPU6050.h"

#define TEST_ENCODER 0
#define TEST_TB6612 0
#define TEST_MPU6050 1

int main(void)
{

	//-------Init-------

	OLED_Init();
#if TEST_ENCODER
	Encoder_Init();
	OLED_ShowString(1, 1, "CNT:+00000Times");
	OLED_ShowString(2, 1, "Pressed:");
#endif
#if TEST_TB6612
	TB6612_Init();
	OLED_ShowString(3, 1, "Speed:+000");
#endif
#if TEST_MPU6050
	MPU6050_Init();
	uint8_t ID = MPU6050_GetID();
	OLED_ShowString(1, 1, "ID:");
	OLED_ShowHexNum(1, 4, ID, 2);
	int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	OLED_ShowString(2, 1, "AX");
	OLED_ShowString(3, 1, "AY");
	OLED_ShowString(4, 1, "AZ");
	OLED_ShowString(2, 9, "GX");
	OLED_ShowString(3, 9, "GY");
	OLED_ShowString(4, 9, "GZ");
#endif


	//-------Main Loop-------
	while (1)
	{

#if TEST_ENCODER
		OLED_ShowSignedNum(1, 5, Encoder_GetCount(), 5);
		OLED_ShowNum(2, 10, (uint32_t)Encoder_Pressed(), 1);
#endif
#if TEST_TB6612
		TB6612_Motor_Breathe();
#endif
#if TEST_MPU6050
		MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
		OLED_ShowSignedNum(2, 3, AccX, 4);
		OLED_ShowSignedNum(3, 3, AccY, 4);
		OLED_ShowSignedNum(4, 3, AccZ, 4);
		OLED_ShowSignedNum(2, 11, GyroX, 4);
		OLED_ShowSignedNum(3, 11, GyroY, 4);
		OLED_ShowSignedNum(4, 11, GyroZ, 4);
		Delay_ms(100);
#endif

	}
}
