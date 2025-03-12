#ifndef __I2C_SW_H
#define __I2C_SW_H

#include "stm32f10x.h"


typedef enum
{
    I2C_SW,
    I2C_SW_NUM,
} I2C_SW_ID;

typedef struct I2C_SW_Device {
    void(*Start)(struct I2C_SW_Device* pDev);
    void(*Stop)(struct I2C_SW_Device* pDev);
    void(*SendAck)(struct I2C_SW_Device* Dev, uint8_t bNo);
    void(*SendByte)(struct I2C_SW_Device* pDev, uint8_t Data);
    uint8_t(*RecieveAck)(struct I2C_SW_Device* Dev);
    uint8_t(*RecieveByte)(struct I2C_SW_Device* pDev);
    void(*WriteI2C)(struct I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t Data);
    uint8_t(*ReadI2C)(struct I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t RegAddr);
    void(*Lock)(struct I2C_SW_Device* Dev);
    void(*UnLock)(struct I2C_SW_Device* Dev);
    void* Priv_Data;
} I2C_SW_Device;

I2C_SW_Device* Drv_I2C_SW_GetDevice(I2C_SW_ID ID);

void Drv_I2C_SW_Init(void);


#endif
