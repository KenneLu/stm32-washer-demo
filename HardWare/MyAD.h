#pragma once

#ifndef __AD_H
#define __AD_H

#define TCRT5000_Channel ADC_Channel_4

typedef enum {
    AD_Comp_TCRT5000,
    AD_Comp_Count
} AD_Componemts;  // 添加设备的类型

void MyAD_Init(void);
uint16_t* MyAD_GetValue(void);

#endif
