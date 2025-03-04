#ifndef __MPU6050_H
#define __MPU6050_H


typedef enum
{
	MPU6050,
	MPU6050_NUM,
} MPU6050_ID;

typedef struct MPU6050_Device {
	uint8_t(*GetID)(struct MPU6050_Device* pDev);
	void(*GetData)(struct MPU6050_Device* pDev,
		int16_t* AccX, int16_t* AccY, int16_t* AccZ,
		int16_t* GyroX, int16_t* GyroY, int16_t* GyroZ);
	void* Priv_Data;
} MPU6050_Device;

MPU6050_Device* Drv_MPU6050_GetDevice(MPU6050_ID ID);

void Drv_MPU6050_Init(void);


#endif
