#ifndef __TB6612_H
#define __TB6612_H


typedef enum
{
    TB6612,
    TB6612_NUM,
} TB6612_ID;

typedef struct TB6612_Device {
    void(*SetSpeed)(struct TB6612_Device* pDev, int8_t Speed);
    void* Priv_Data;
} TB6612_Device;

TB6612_Device* Drv_TB6612_GetDevice(TB6612_ID ID);

void Drv_TB6612_Init(void);


#endif
