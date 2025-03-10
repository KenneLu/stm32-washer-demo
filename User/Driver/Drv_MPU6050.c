#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include "Drv_MPU6050_Reg.h"
#include "Drv_MPU6050.h"
#include "Drv_I2C_HW.h"


typedef struct
{
	MPU6050_ID ID;
	uint16_t ADDRESS;
	GPIO_TypeDef* PORT;
	uint16_t PIN_SCL;
	uint16_t PIN_SDA;
	GPIOMode_TypeDef MODE;
	uint32_t _RCC;
} MPU6050_HARDWARE;

typedef struct {
	MPU6050_ID ID;
	MPU6050_HARDWARE HW;
} MPU6050_Data;

void MPU6050_WriteReg(MPU6050_Device* pDev, uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(MPU6050_Device* pDev, uint8_t RegAddress);
uint8_t GetID(MPU6050_Device* pDev);
void GetData(MPU6050_Device* pDev,
	int16_t* AccX, int16_t* AccY, int16_t* AccZ,
	int16_t* GyroX, int16_t* GyroY, int16_t* GyroZ);


//--------------------------------------------------


I2C_HW_Device* g_pDev_I2C_HW;

static MPU6050_HARDWARE g_MPU6050_HWs[MPU6050_NUM] = {
	{
		.ID = MPU6050,
		.ADDRESS = 0xD0,
		.PORT = GPIOB,
		.PIN_SCL = GPIO_Pin_10,
		.PIN_SDA = GPIO_Pin_11,
		.MODE = GPIO_Mode_AF_OD,
		._RCC = RCC_APB2Periph_GPIOB,
	},
};
static MPU6050_Data g_MPU6050_Datas[MPU6050_NUM];
static MPU6050_Device g_MPU6050_Devs[MPU6050_NUM];


//--------------------------------------------------


MPU6050_Device* Drv_MPU6050_GetDevice(MPU6050_ID ID)
{
	for (int i = 0; i < sizeof(g_MPU6050_Devs) / sizeof(g_MPU6050_Devs[0]); i++)
	{
		MPU6050_Data* pData = (MPU6050_Data*)g_MPU6050_Devs[i].Priv_Data;
		if (pData == 0)
			return 0;
		if (pData->ID == ID)
			return &g_MPU6050_Devs[i];
	}
	return 0;
}

void Drv_MPU6050_Init(void)
{
	Drv_I2C_HW_Init();
	g_pDev_I2C_HW = Drv_I2C_HW_GetDevice(I2C_HW);

	for (uint8_t i = 0; i < MPU6050_NUM; i++)
	{
		// Get Hardware
		MPU6050_HARDWARE hw;
		for (uint8_t j = 0; j < sizeof(g_MPU6050_HWs) / sizeof(g_MPU6050_HWs[0]); j++)
		{
			if (g_MPU6050_HWs[j].ID == (MPU6050_ID)i)
				hw = g_MPU6050_HWs[j];
		}

		// Data Init
		g_MPU6050_Datas[i].ID = (MPU6050_ID)i;
		g_MPU6050_Datas[i].HW = hw;

		// Device Init
		g_MPU6050_Devs[i].GetID = GetID;
		g_MPU6050_Devs[i].GetData = GetData;
		g_MPU6050_Devs[i].Priv_Data = (void*)&g_MPU6050_Datas[i];

		// Hardware Init
		RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = hw.PIN_SCL | hw.PIN_SDA;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = hw.MODE;
		GPIO_Init(hw.PORT, &GPIO_InitStructure);

		// MPU6050 Init
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_PWR_MGMT_1, 0x01); // 解除休眠状态，选择陀螺仪时钟
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_PWR_MGMT_2, 0x00); // 无需循环唤醒，无需待机
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_SMPLRT_DIV, 0x09); // 10分频
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_CONFIG, 0x06); // 无需外部同步，低通滤波器模式设为最平滑
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_GYRO_CONFIG, 0x18); // 无需自测，最大量程
		MPU6050_WriteReg(&g_MPU6050_Devs[i], MPU6050_ACCEL_CONFIG, 0x18); // 无需自测，最大量程
	}
}


//--------------------------------------------------


uint8_t GetID(MPU6050_Device* pDev)
{
	return MPU6050_ReadReg(pDev, MPU6050_WHO_AM_I);
}

void GetData(MPU6050_Device* pDev,
	int16_t* AccX, int16_t* AccY, int16_t* AccZ,
	int16_t* GyroX, int16_t* GyroY, int16_t* GyroZ)
{
	uint8_t DataH, DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_ACCEL_XOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_ACCEL_XOUT_L);
	*AccX = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_ACCEL_YOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_ACCEL_YOUT_L);
	*AccY = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_ACCEL_ZOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_ACCEL_ZOUT_L);
	*AccZ = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_GYRO_XOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_GYRO_XOUT_L);
	*GyroX = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_GYRO_YOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_GYRO_YOUT_L);
	*GyroY = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(pDev, MPU6050_GYRO_ZOUT_H);
	DataL = MPU6050_ReadReg(pDev, MPU6050_GYRO_ZOUT_L);
	*GyroZ = (DataH << 8) | DataL;
}

void MPU6050_WaitEvent(uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000; // 随便填的，实际项目需要严格一些
	while (g_pDev_I2C_HW->CheckEvent(g_pDev_I2C_HW, I2C_EVENT) != SUCCESS)
	{
		Timeout--;
		if (Timeout == 0)
		{
			printf("I2C Timeout\r\n");
			break;
		}
	}
}

void MPU6050_WriteReg(MPU6050_Device* pDev, uint8_t RegAddress, uint8_t Data)
{
	MPU6050_Data* pData = (MPU6050_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	g_pDev_I2C_HW->GenerateSTART(g_pDev_I2C_HW, ENABLE); // START
	MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT); // EV5

	// 发送和接收都硬件自带应答，无需软件额外处理。如果应答错误，硬件会自动触发相关中断	
	g_pDev_I2C_HW->Send7bitAddress(g_pDev_I2C_HW, pData->HW.ADDRESS, I2C_Direction_Transmitter); // MPU6050_ADDRESS
	MPU6050_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->SendData(g_pDev_I2C_HW, RegAddress); // MPU6050_RegAddress
	MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING); // EV8

	g_pDev_I2C_HW->SendData(g_pDev_I2C_HW, Data); // Send Data
	MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED); // EV8_2

	g_pDev_I2C_HW->GenerateSTOP(g_pDev_I2C_HW, ENABLE); // STOP
}

uint8_t MPU6050_ReadReg(MPU6050_Device* pDev, uint8_t RegAddress)
{
	MPU6050_Data* pData = (MPU6050_Data*)pDev->Priv_Data;
	if (pData == 0) return 0;

	g_pDev_I2C_HW->GenerateSTART(g_pDev_I2C_HW, ENABLE); // START
	MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT); // EV5

	g_pDev_I2C_HW->Send7bitAddress(g_pDev_I2C_HW, pData->HW.ADDRESS, I2C_Direction_Transmitter); // MPU6050_ADDRESS
	MPU6050_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->SendData(g_pDev_I2C_HW, RegAddress); // MPU6050_RegAddress
	MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED); // EV8 TRANSMITTED

	g_pDev_I2C_HW->GenerateSTART(g_pDev_I2C_HW, ENABLE); // RESTART
	MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT); // EV5

	g_pDev_I2C_HW->Send7bitAddress(g_pDev_I2C_HW, pData->HW.ADDRESS, I2C_Direction_Receiver); // MPU6050_ADDRESS
	MPU6050_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->AcknowledgeConfig(g_pDev_I2C_HW, DISABLE); // Ack(0) // EV6_1
	g_pDev_I2C_HW->GenerateSTOP(g_pDev_I2C_HW, ENABLE); // STOP // EV6_1

	MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED); // EV7
	uint8_t Data = g_pDev_I2C_HW->ReceiveData(g_pDev_I2C_HW); // Receive Data

	g_pDev_I2C_HW->AcknowledgeConfig(g_pDev_I2C_HW, ENABLE); // Ack(1) // 复位，便于以后接收多个字节

	return Data;
}
