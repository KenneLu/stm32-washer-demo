#ifndef __ENCODER_H
#define __ENCODER_H


typedef enum
{
    ENCODER,
    ENCODER_NUM,
} ENCODER_ID;

typedef struct ENCODER_Device {
    int16_t(*Encoder_GetCount)(struct ENCODER_Device* pDev);
    int16_t(*Encoder_GetSpeed)(struct ENCODER_Device* pDev);
    int16_t(*Encoder_Get_Div4)(struct ENCODER_Device* pDev);
    void* Priv_Data;
} ENCODER_Device;

ENCODER_Device* Drv_Encoder_GetDevice(ENCODER_ID ID);

void Dev_Encoder_Init(void);


#endif
