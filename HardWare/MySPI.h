#ifndef __SPI_H
#define __SPI_H


typedef enum
{
    SPI_HW,
    SPI_HW_NUM,
} SPI_HW_ID;

typedef struct SPI_HW_Device {
    void(*Start)(struct SPI_HW_Device* pDev);
    void(*Stop)(struct SPI_HW_Device* pDev);
    uint8_t(*SwapData)(struct SPI_HW_Device* pDev, uint8_t DataSend);
    void* Priv_Data;
} SPI_HW_Device;

SPI_HW_Device* Drv_SPI_HW_GetDevice(SPI_HW_ID ID);

void Drv_SPI_HW_Init(void);



#endif
