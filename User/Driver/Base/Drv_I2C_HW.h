#ifndef __I2C_HW_H
#define __I2C_HW_H

#include "stm32f10x.h"                  // Device header


typedef enum
{
    I2C_HW,
    I2C_HW_NUM,
} I2C_HW_ID;

typedef struct I2C_HW_Device {
    void(*GenerateSTART)(struct I2C_HW_Device* pDev, FunctionalState NewState);
    void(*GenerateSTOP)(struct I2C_HW_Device* pDev, FunctionalState NewState);
    void(*AcknowledgeConfig)(struct I2C_HW_Device* pDev, FunctionalState NewState);
    void(*Send7bitAddress)(struct I2C_HW_Device* pDev, uint8_t Address, uint8_t I2C_Direction);
    void(*SendData)(struct I2C_HW_Device* pDev, uint8_t Data);
    uint8_t(*ReceiveData)(struct I2C_HW_Device* pDev);
    ErrorStatus(*CheckEvent)(struct I2C_HW_Device* pDev, uint32_t I2C_EVENT);
    void* Priv_Data;
} I2C_HW_Device;

I2C_HW_Device* Drv_I2C_HW_GetDevice(I2C_HW_ID ID);

void Drv_I2C_HW_Init(void);


#endif
