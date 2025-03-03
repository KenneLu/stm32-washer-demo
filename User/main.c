#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "Encoder.h"
#include "TB6612.h"
#include "ServoMotor.h"
#include "MPU6050.h"
#include "W25Q64.h"
#include "Buzzer.h"
#include "DHT11.h"
#include "MyAD.h"
#include "Timer.h"
#include "Key.h"
#include "Menu.h"

#define TEST_ENCODER 0
#define TEST_TB6612 0
#define TEST_SERVO_MOTOR 0
#define TEST_MPU6050 0
#define TEST_MPU6050_BUG 0
#define TEST_W25Q64 0
#define TEST_BUZZER 0
#define TEST_DHT11 0
#define TEST_TCRT5000 0
#define TEST_TIM 0
#define TEST_KEY 0
#define TEST_MENU 0
#define TEST_WAKE_UP 0
#define TEST_MENU_WASHER 0
#define TEST_MENU_WASHER_V2 1

#if TEST_W25Q64
uint8_t MID;
uint16_t DID;

uint8_t ArrayWrite1[] = { 0x01, 0x02, 0x03, 0x04 };
uint8_t ArrayWrite2[] = { 0xAA, 0xBB, 0xCC, 0xDD };
uint8_t ArrayWrite3[] = { 0x55, 0x66, 0x77, 0x88 };
uint8_t ArrayRead[4];

void Show_W(uint8_t* Array_W)
{
	OLED_ShowHexNum_Easy(2, 4, Array_W[0], 2);
	OLED_ShowHexNum_Easy(2, 7, Array_W[1], 2);
	OLED_ShowHexNum_Easy(2, 10, Array_W[2], 2);
	OLED_ShowHexNum_Easy(2, 13, Array_W[3], 2);
}

void Show_R(void)
{
	OLED_ShowHexNum_Easy(3, 4, ArrayRead[0], 2);
	OLED_ShowHexNum_Easy(3, 7, ArrayRead[1], 2);
	OLED_ShowHexNum_Easy(3, 10, ArrayRead[2], 2);
	OLED_ShowHexNum_Easy(3, 13, ArrayRead[3], 2);
}
#endif
#if TEST_TIM
uint16_t i = 0;
#endif
#if TEST_KEY
void Key_CB_Press(void)
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "Key Press");
}

void Key_CB_Release(void)
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "Key Releas");
}

void Key_CB_LongPress(void)
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "Key LP");
}

void Key_CB_LongPress_Continuos(void)
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "Key LP Cont");
}

void Key_CB_LongPress_Release(void)
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "Key LP Release");
}
#endif



int main(void)
{

	//-------Init-------

	OLED_Init();
#if TEST_ENCODER
	Encoder_Init();
	OLED_ShowString_Easy_Easy(1, 1, "CNT:+00000Times");
	OLED_ShowString_Easy_Easy(2, 1, "Pressed:");
#endif
#if TEST_TB6612
	TB6612_Init();
#endif
#if TEST_SERVO_MOTOR
	Servo_Motor_Init();
#endif
#if TEST_MPU6050
	MPU6050_Init();
	uint8_t ID = MPU6050_GetID();
	OLED_ShowString_Easy(1, 1, "ID:");
	OLED_ShowHexNum_Easy(1, 4, ID, 2);
	int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	OLED_ShowString_Easy(2, 1, "AX");
	OLED_ShowString_Easy(3, 1, "AY");
	OLED_ShowString_Easy(4, 1, "AZ");
	OLED_ShowString_Easy(2, 9, "GX");
	OLED_ShowString_Easy(3, 9, "GY");
	OLED_ShowString_Easy(4, 9, "GZ");
#endif
#if TEST_MPU6050_BUG
	MPU6050_Init(); // 初始化MPU6050
	TB6612_Init();	// 初始化电机驱动
	Buzzer_Init();	// 初始化蜂鸣器

	uint8_t ID = MPU6050_GetID();
	OLED_ShowString_Easy(1, 1, "ID:");
	OLED_ShowHexNum_Easy(1, 4, ID, 2);
	int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	OLED_ShowString_Easy(2, 1, "AX");
	OLED_ShowString_Easy(3, 1, "AY");
#endif
#if TEST_W25Q64
	//W25Q64初始化
	W25Q64_Init();
	OLED_ShowString_Easy(1, 1, "MID:   DID:");
	W25Q64_ReadID(&MID, &DID);
	OLED_ShowHexNum_Easy(1, 5, MID, 2);
	OLED_ShowHexNum_Easy(1, 12, DID, 4);

	// 擦除写入1
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x000000, ArrayWrite1, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString_Easy(2, 1, "W1:");
	OLED_ShowString_Easy(3, 1, "R1:");
	Show_W(ArrayWrite1); // 01 02 03 04
	Show_R();            // 01 02 03 04
	Delay_s(1);

	// 擦除写入2
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x000000, ArrayWrite2, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString_Easy(2, 1, "W2:");
	OLED_ShowString_Easy(3, 1, "R2:");
	Show_W(ArrayWrite2); // AA BB CC DD
	Show_R();            // AA BB CC DD
	Delay_s(1);

	// 不擦除写入
	W25Q64_PageProgram(0x000000, ArrayWrite3, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString_Easy(2, 1, "W3:");
	OLED_ShowString_Easy(3, 1, "R3:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // 00 22 44 88 = AA BB CC DD & 55 66 77 88
	Delay_s(1);

	// 擦除不写入
	W25Q64_SectorErase(0x000000);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString_Easy(2, 1, "W3:");
	OLED_ShowString_Easy(3, 1, "R4:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // FF FF FF FF
	Delay_s(1);

	// 跨页读写
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x0000FF, ArrayWrite3, 4);
	W25Q64_ReadData(0x0000FF, ArrayRead, 4);
	OLED_ShowString_Easy(2, 1, "W3:");
	OLED_ShowString_Easy(3, 1, "R5:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // 55 FF FF FF = 写没有跨页，读跨页
	Delay_s(1);
	OLED_ShowString_Easy(3, 1, "R6:");
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	Show_R();            // 66 77 88 FF = 写覆盖页首
	Delay_s(1);
#endif
#if TEST_BUZZER
	Buzzer_Init();
#endif
#if TEST_DHT11
	OLED_ShowString_Easy(1, 1, "Humi:"); 	// 温度 Temperature
	OLED_ShowString_Easy(2, 1, "Temp:");	// 湿度 Relative Humidity
	OLED_ShowString_Easy(1, 8, ".");
	OLED_ShowString_Easy(2, 8, ".");
	OLED_ShowChinese_Easy(1, 11 / 2 + 1, "％");		// 中文是16x16的，所以要减半，并向上取整
	OLED_ShowChinese_Easy(2, 11 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整
#endif
#if TEST_TCRT5000
	MyAD_Init();
	OLED_ShowString_Easy(1, 1, "AD4:");
	uint16_t* pADValue;
#endif
#if TEST_TIM
	Timer_Init();
	OLED_ShowString_Easy(3, 1, "TIM2:");
#endif
#if TEST_KEY
	Key_Init();
	Timer_Init();
	Encoder_Init();
	OLED_ShowString_Easy(1, 1, "Init");
	Key_CBRegister_P(KEY_ENCODER_PRESS, Key_CB_Press);
	Key_CBRegister_R(KEY_ENCODER_PRESS, Key_CB_Release);
	Key_CBRegister_LP(KEY_ENCODER_PRESS, Key_CB_LongPress);
	Key_CBRegister_LP_Cont(KEY_ENCODER_PRESS, Key_CB_LongPress_Continuos);
	Key_CBRegister_LP_R(KEY_ENCODER_PRESS, Key_CB_LongPress_Release);
#endif
#if TEST_MENU
	Key_Init();
	Timer_Init();
	Encoder_Init();
	Menu_Init();
#endif
#if TEST_MENU_WASHER
	Key_Init();
	Timer_Init();
	Encoder_Init();
	Menu_Init();
	Menu_Washer_Power_On();
#endif
#if TEST_MENU_WASHER_V2
	Key_Init();
	Timer_Init();
	Encoder_Init();
	Menu_Init();
	Menu_Washer_Power_On();
#endif



	//-------Main Loop-------
	while (1)
	{

#if TEST_ENCODER
		OLED_ShowSignedNum_Easy(1, 5, Encoder_GetCount(), 5);
		OLED_ShowNum_Easy(2, 10, (uint32_t)Encoder_Pressed(), 1);
#endif
#if TEST_TB6612
		TB6612_Motor_Breathe();
#endif
#if TEST_SERVO_MOTOR
		Servo_Motorr_Breathe();
#endif
#if TEST_MPU6050
		MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
		OLED_ShowSignedNum_Easy(2, 3, AccX, 4);
		OLED_ShowSignedNum_Easy(3, 3, AccY, 4);
		OLED_ShowSignedNum_Easy(4, 3, AccZ, 4);
		OLED_ShowSignedNum_Easy(2, 11, GyroX, 4);
		OLED_ShowSignedNum_Easy(3, 11, GyroY, 4);
		OLED_ShowSignedNum_Easy(4, 11, GyroZ, 4);
		Delay_ms(100);
#endif
#if TEST_MPU6050_BUG
		MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
		OLED_ShowSignedNum_Easy(2, 3, AccX, 4);
		OLED_ShowSignedNum_Easy(3, 3, AccY, 4);
		Delay_ms(100);

		static int16_t AccX_Abs, AccY_Abs;
		AccX_Abs = AccX > 0 ? AccX : -AccX;
		AccY_Abs = AccY > 0 ? AccY : -AccY;
		if (AccX_Abs > 50 || AccY_Abs > 50) // 瞬时加速度大于50
		{
			Buzzer_Revert();
			TB6612_Motor_SetSpeed(0);
			Delay_ms(2000);
			Buzzer_Revert();
		}
		else
		{
			// 电机全速运转，导致电压不稳定
			// 最终导致 MPU6050 的所有输出均为 -1823.
			// 拔掉电机扇叶更容易复现
			TB6612_Motor_SetSpeed(100);
		}
#endif
#if TEST_BUZZER
		Buzzer_Breathe();
#endif
#if TEST_DHT11
		DHT11_Data_t DHT11_Data;
		DHT11_Recive_Data(&DHT11_Data); //接收温度和湿度的数据

		OLED_ShowNum_Easy(1, 6, (uint32_t)DHT11_Data.Humi, 2);
		OLED_ShowNum_Easy(1, 9, (uint32_t)DHT11_Data.Humi_Dec, 2);

		OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Temp, 2);
		OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Temp_Dec, 2);

		Delay_s(1);
#endif
#if TEST_TCRT5000
		pADValue = MyAD_GetValue();
		OLED_ShowNum_Easy(1, 5, pADValue[AD_Comp_TCRT5000], 4);
		Delay_ms(500);
#endif
#if TEST_TIM
		OLED_ShowNum_Easy(3, 6, (uint32_t)i, 5);
		OLED_ShowNum_Easy(4, 1, (uint32_t)Timer_GetCounter(), 5);
#endif
#if TEST_MENU
		Main_Menu();
#endif
#if TEST_WAKE_UP
		OLED_ShowString_Easy(1, 1, "Wake");
		Delay_ms(1000);
		OLED_ShowString_Easy(1, 1, "Sleep");
		Delay_ms(1000);
		OLED_Clear();
		OLED_Update();
		Delay_ms(100);
		// 开始待机
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);    //使能PWR外设时钟
		PWR_WakeUpPinCmd(ENABLE);  //使能唤醒管脚功能,在WkUp的上升沿进行
		PWR_EnterSTANDBYMode();
#endif
#if TEST_MENU_WASHER
		Menu_Washer_Run();
#endif
#if TEST_MENU_WASHER_V2
		Menu_Washer_Run();
#endif

	}
}

// -------Interrupts-------

#if TEST_TIM
void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		i++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif
#if TEST_KEY
void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Scan();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif
#if TEST_MENU
void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Scan();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif
#if TEST_MENU_WASHER
void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Scan();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif
#if TEST_MENU_WASHER_V2
void TIM2_IRQHandler(void) //1ms
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Scan();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif
