#pragma once

#ifndef __AD_H
#define __AD_H


typedef enum
{
    AD_TCRT5000,
    AD_NUM,
} AD_ID;

typedef struct AD_Device {
    uint16_t(*GetValue)(struct AD_Device* pDev);
    void* Priv_Data;
} AD_Device;

AD_Device* Drv_AD_GetDevice(AD_ID ID);

void Drv_AD_Init(void);


#endif
