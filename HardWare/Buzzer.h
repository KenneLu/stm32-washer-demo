#ifndef __BUZZER_H
#define __BUZZER_H


typedef enum
{
    BUZZER,
    BUZZER_NUM,
} BUZZER_ID;

typedef struct BUZZER_Device {
    void(*Buzzer_On)(struct BUZZER_Device* pDev);
    void(*Buzzer_Off)(struct BUZZER_Device* pDev);
    void(*Buzzer_Revert)(struct BUZZER_Device* pDev);
    uint8_t(*Is_Buzzer_On)(struct BUZZER_Device* pDev);
    void* Priv_Data;
} BUZZER_Device;

BUZZER_Device* Drv_Buzzer_GetDevice(BUZZER_ID ID);

void Drv_Buzzer_Init(void);


#endif
