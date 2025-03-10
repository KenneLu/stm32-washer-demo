#include "stm32f10x.h"
#include <stdio.h>
#include "Drv_USART.h"


//重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE* f)
{
    USART_Device* pDev_USART = Drv_USART_GetDevice(USART_1);
    if (pDev_USART == 0)
        return ch;

    pDev_USART->SendData(pDev_USART, (uint8_t)ch);
    return (ch);
}

//重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE* f)
{
    USART_Device* pDev_USART = Drv_USART_GetDevice(USART_1);
    if (pDev_USART == 0)
        return 0;

    return pDev_USART->ReceiveData(pDev_USART);
}
