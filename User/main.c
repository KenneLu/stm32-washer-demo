#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "Encoder.h"
#include "TB6612.h"
#include "MPU6050.h"
#include "W25Q64.h"

#define TEST_ENCODER 0
#define TEST_TB6612 0
#define TEST_MPU6050 0
#define TEST_W25Q64 0


#if TEST_W25Q64
uint8_t MID;
uint16_t DID;

uint8_t ArrayWrite1[] = { 0x01, 0x02, 0x03, 0x04 };
uint8_t ArrayWrite2[] = { 0xAA, 0xBB, 0xCC, 0xDD };
uint8_t ArrayWrite3[] = { 0x55, 0x66, 0x77, 0x88 };
uint8_t ArrayRead[4];

void Show_W(uint8_t* Array_W)
{
	OLED_ShowHexNum(2, 4, Array_W[0], 2);
	OLED_ShowHexNum(2, 7, Array_W[1], 2);
	OLED_ShowHexNum(2, 10, Array_W[2], 2);
	OLED_ShowHexNum(2, 13, Array_W[3], 2);
}

void Show_R(void)
{
	OLED_ShowHexNum(3, 4, ArrayRead[0], 2);
	OLED_ShowHexNum(3, 7, ArrayRead[1], 2);
	OLED_ShowHexNum(3, 10, ArrayRead[2], 2);
	OLED_ShowHexNum(3, 13, ArrayRead[3], 2);
}
#endif


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
#if TEST_W25Q64
	//W25Q64初始化
	W25Q64_Init();
	OLED_ShowString(1, 1, "MID:   DID:");
	W25Q64_ReadID(&MID, &DID);
	OLED_ShowHexNum(1, 5, MID, 2);
	OLED_ShowHexNum(1, 12, DID, 4);

	// 擦除写入1
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x000000, ArrayWrite1, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString(2, 1, "W1:");
	OLED_ShowString(3, 1, "R1:");
	Show_W(ArrayWrite1); // 01 02 03 04
	Show_R();            // 01 02 03 04
	Delay_s(1);

	// 擦除写入2
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x000000, ArrayWrite2, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString(2, 1, "W2:");
	OLED_ShowString(3, 1, "R2:");
	Show_W(ArrayWrite2); // AA BB CC DD
	Show_R();            // AA BB CC DD
	Delay_s(1);

	// 不擦除写入
	W25Q64_PageProgram(0x000000, ArrayWrite3, 4);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString(2, 1, "W3:");
	OLED_ShowString(3, 1, "R3:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // 00 22 44 88 = AA BB CC DD & 55 66 77 88
	Delay_s(1);

	// 擦除不写入
	W25Q64_SectorErase(0x000000);
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	OLED_ShowString(2, 1, "W3:");
	OLED_ShowString(3, 1, "R4:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // FF FF FF FF
	Delay_s(1);

	// 跨页读写
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x0000FF, ArrayWrite3, 4);
	W25Q64_ReadData(0x0000FF, ArrayRead, 4);
	OLED_ShowString(2, 1, "W3:");
	OLED_ShowString(3, 1, "R5:");
	Show_W(ArrayWrite3); // 55 66 77 88
	Show_R();            // 55 FF FF FF = 写没有跨页，读跨页
	Delay_s(1);
	OLED_ShowString(3, 1, "R6:");
	W25Q64_ReadData(0x000000, ArrayRead, 4);
	Show_R();            // 66 77 88 FF = 写覆盖页首
	Delay_s(1);
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
