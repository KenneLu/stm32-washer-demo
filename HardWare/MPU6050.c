#include "stm32f10x.h"                  // Device header
#include "MPU6050_Reg.h"
#include "MPU6050.h"
#include "Drv_I2C_HW.h"

#define MPU6050_ADDRESS		0xD0

I2C_HW_Device* g_pDev_I2C_HW;

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);


void MPU6050_Init(void)
{
	Drv_I2C_HW_Init();
	g_pDev_I2C_HW = Drv_I2C_HW_GetDevice(I2C_HW);

	RCC_APB2PeriphClockCmd(MPU6050_GPIO_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = MPU6050_GOIO_MODE;
	GPIO_InitStructure.GPIO_Pin = MPU6050_GOIO_PIN_SCL | MPU6050_GOIO_PIN_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MPU6050_GOIO_x, &GPIO_InitStructure);

	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01); // 解除休眠状态，选择陀螺仪时钟
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00); // 无需循环唤醒，无需待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09); // 10分频
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06); // 无需外部同步，低通滤波器模式设为最平滑
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18); // 无需自测，最大量程
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18); // 无需自测，最大量程
}

uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

void MPU6050_GetData(int16_t* AccX, int16_t* AccY, int16_t* AccZ,
	int16_t* GyroX, int16_t* GyroY, int16_t* GyroZ)
{
	uint8_t DataH, DataL;

	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
	*AccX = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
	*AccY = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
	*AccZ = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
	*GyroX = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
	*GyroY = (DataH << 8) | DataL;

	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
	*GyroZ = (DataH << 8) | DataL;
}

void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000; // 随便填的，实际项目需要严格一些
	while (g_pDev_I2C_HW->I2C_HW_CheckEvent(g_pDev_I2C_HW, I2C_EVENT) != SUCCESS)
	{
		Timeout--;
		if (Timeout == 0)
		{
			break;
		}
	}
}

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	g_pDev_I2C_HW->I2C_HW_GenerateSTART(g_pDev_I2C_HW, ENABLE); // START
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_MODE_SELECT); // EV5

	// 发送和接收都硬件自带应答，无需软件额外处理。如果应答错误，硬件会自动触发相关中断	
	g_pDev_I2C_HW->I2C_HW_Send7bitAddress(g_pDev_I2C_HW, MPU6050_ADDRESS, I2C_Direction_Transmitter); // MPU6050_ADDRESS
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->I2C_HW_SendData(g_pDev_I2C_HW, RegAddress); // MPU6050_RegAddress
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING); // EV8

	g_pDev_I2C_HW->I2C_HW_SendData(g_pDev_I2C_HW, Data); // Send Data
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_BYTE_TRANSMITTED); // EV8_2

	g_pDev_I2C_HW->I2C_HW_GenerateSTOP(g_pDev_I2C_HW, ENABLE); // STOP
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	g_pDev_I2C_HW->I2C_HW_GenerateSTART(g_pDev_I2C_HW, ENABLE); // START
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_MODE_SELECT); // EV5

	g_pDev_I2C_HW->I2C_HW_Send7bitAddress(g_pDev_I2C_HW, MPU6050_ADDRESS, I2C_Direction_Transmitter); // MPU6050_ADDRESS
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->I2C_HW_SendData(g_pDev_I2C_HW, RegAddress); // MPU6050_RegAddress
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_BYTE_TRANSMITTED); // EV8 TRANSMITTED

	g_pDev_I2C_HW->I2C_HW_GenerateSTART(g_pDev_I2C_HW, ENABLE); // RESTART
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_MODE_SELECT); // EV5

	g_pDev_I2C_HW->I2C_HW_Send7bitAddress(g_pDev_I2C_HW, MPU6050_ADDRESS, I2C_Direction_Receiver); // MPU6050_ADDRESS
	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED); // EV6

	g_pDev_I2C_HW->I2C_HW_AcknowledgeConfig(g_pDev_I2C_HW, DISABLE); // Ack(0) // EV6_1
	g_pDev_I2C_HW->I2C_HW_GenerateSTOP(g_pDev_I2C_HW, ENABLE); // STOP // EV6_1

	MPU6050_WaitEvent(MPU6050_I2C_x, I2C_EVENT_MASTER_BYTE_RECEIVED); // EV7
	uint8_t Data = g_pDev_I2C_HW->I2C_HW_ReceiveData(g_pDev_I2C_HW); // Receive Data

	g_pDev_I2C_HW->I2C_HW_AcknowledgeConfig(g_pDev_I2C_HW, ENABLE); // Ack(1) // 复位，便于以后接收多个字节

	return Data;
}
