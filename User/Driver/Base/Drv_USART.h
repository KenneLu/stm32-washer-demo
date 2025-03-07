#ifndef __USART_H
#define __USART_H


typedef enum
{
    USART_1,
    USART_NUM,
} USART_ID;

typedef struct USART_Device {
    void(*SendData)(struct USART_Device* pDev, uint8_t ch);
    int(*ReceiveData)(struct USART_Device* pDev);
    void* Priv_Data;
} USART_Device;

USART_Device* Drv_USART_GetDevice(USART_ID ID);

void Drv_USART_Init(void);


#endif
