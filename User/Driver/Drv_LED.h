#ifndef __LED_H
#define __LED_H


typedef enum
{
    LED_RED,
    LED_BLUE,
    LED_NUM,
} LED_ID;

typedef struct LED_Device {
    void(*On)(struct LED_Device* pDev);
    void(*Off)(struct LED_Device* pDev);
    void(*Revert)(struct LED_Device* pDev);
    uint8_t(*Is_On)(struct LED_Device* pDev);
    void* Priv_Data;
} LED_Device;

LED_Device* Drv_LED_GetDevice(LED_ID ID);

void Drv_LED_Init(void);


#endif
