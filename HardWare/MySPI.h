#ifndef __SPI_H
#define __SPI_H


typedef enum
{
    SPI_DEVICE_W25Q64,
    SPI_DEVICE_NUM,
} SPI_DEVICE_ID;

typedef struct SPI_Device {
    void(*Start)(struct SPI_Device* pDev);
    void(*Stop)(struct SPI_Device* pDev);
    uint8_t(*SwapData)(struct SPI_Device* pDev, uint8_t DataSend);
    void* Priv_Data;
} SPI_Device;

SPI_Device* Drv_SPI_GetDevice(SPI_DEVICE_ID ID);

void Drv_SPI_Init(void);


#endif
