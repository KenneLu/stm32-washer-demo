#ifndef __W25Q64_H
#define __W25Q64_H


typedef enum
{
    W25Q64,
    W25Q64_NUM,
} W25Q64_ID;

typedef struct W25Q64_Device {
    void(*ReadID)(struct W25Q64_Device* pDev, uint8_t* MID, uint16_t* DID);
    void(*ReadData)(struct W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint32_t Count);
    void(*PageProgram)(struct W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint16_t Count);
    void(*SectorErase)(struct W25Q64_Device* pDev, uint32_t Address);
    void* Priv_Data;
} W25Q64_Device;

W25Q64_Device* Drv_W25Q64_GetDevice(W25Q64_ID ID);

void Drv_W25Q64_Init(void);


#endif
