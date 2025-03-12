#include "stm32f10x.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "Drv_USART.h"

SemaphoreHandle_t g_LOCK;

void Drv_Debug_Init(void)
{
    g_LOCK = xSemaphoreCreateRecursiveMutex();
}

//重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE* f)
{
    USART_Device* pDev_USART = Drv_USART_GetDevice(USART_1);
    if (pDev_USART == 0)
        return ch;

    xSemaphoreTakeRecursive(g_LOCK, portMAX_DELAY);
    pDev_USART->SendData(pDev_USART, (uint8_t)ch);
    xSemaphoreGiveRecursive(g_LOCK);
    return (ch);
}

//重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE* f)
{
    USART_Device* pDev_USART = Drv_USART_GetDevice(USART_1);
    if (pDev_USART == 0)
        return 0;

    int data = 0;
    xSemaphoreTakeRecursive(g_LOCK, portMAX_DELAY);
    data = pDev_USART->ReceiveData(pDev_USART);
    xSemaphoreGiveRecursive(g_LOCK);
    return data;
}
