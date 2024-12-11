#ifndef __MPU6050_H
#define __MPU6050_H

//GPIO
#define MPU6050_GPIO_RCC       RCC_APB2Periph_GPIOB
#define MPU6050_GOIO_x         GPIOB
#define MPU6050_GOIO_PIN_SCL   GPIO_Pin_10
#define MPU6050_GOIO_PIN_SDA   GPIO_Pin_11
#define MPU6050_GOIO_MODE      GPIO_Mode_AF_OD	// 复用开漏，GPIO控制权交给I2C外设

//I2C
#define MPU6050_I2C_RCC	       RCC_APB1Periph_I2C2
#define MPU6050_I2C_x	       I2C2

void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_GetData(
	int16_t* AccX, int16_t* AccY, int16_t* AccZ,
	int16_t* GyroX, int16_t* GyroY, int16_t* GyroZ);

#endif
