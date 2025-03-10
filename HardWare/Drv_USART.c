#include "stm32f10x.h"                  // Device header
#include "Drv_USART.h"
#include <stdio.h>


typedef struct
{
    USART_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN_TX;
    uint16_t PIN_RX;
    GPIOMode_TypeDef MODE_TX;
    GPIOMode_TypeDef MODE_RX;
    uint32_t _RCC;
    USART_TypeDef* USARTx;
    uint32_t USART_RCC;
    uint32_t USART_BaudRate;
    uint16_t USART_WordLength;
    uint16_t USART_StopBits;
    uint16_t USART_Parity;
    uint16_t USART_Mode;
    uint16_t USART_HardwareFlowControl;
    uint32_t NVIC_PriorityGroup;
    uint8_t NVIC_IRQChannel;
    uint8_t NVIC_IRQChannelPreemptionPriority;
    uint8_t NVIC_IRQChannelSubPriority;
    FunctionalState NVIC_IRQChannelCmd;
} USART_HARDWARE;

typedef struct {
    USART_ID ID;
    USART_HARDWARE HW;
} USART_Data;

void SendData(struct USART_Device* pDev, uint8_t ch);
int ReceiveData(struct USART_Device* pDev);


//--------------------------------------------------


static USART_HARDWARE g_USART_HWs[USART_NUM] = {
    {
        .ID = USART_1,
        .PORT = GPIOA,
        .PIN_TX = GPIO_Pin_9,
        .PIN_RX = GPIO_Pin_10,
        .MODE_TX = GPIO_Mode_AF_PP,
        .MODE_RX = GPIO_Mode_IN_FLOATING,
        ._RCC = RCC_APB2Periph_GPIOA,
        .USARTx = USART1,
        .USART_RCC = RCC_APB2Periph_USART1,
        .USART_BaudRate = 115200,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .NVIC_PriorityGroup = NVIC_PriorityGroup_4,
        .NVIC_IRQChannel = USART1_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 7,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE,
    },
};
static USART_Data g_USART_Datas[USART_NUM];
static USART_Device g_USART_Devs[USART_NUM];


//--------------------------------------------------


USART_Device* Drv_USART_GetDevice(USART_ID ID)
{
    for (int i = 0; i < sizeof(g_USART_Devs) / sizeof(g_USART_Devs[0]); i++)
    {
        USART_Data* pData = (USART_Data*)g_USART_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_USART_Devs[i];
    }
    return 0;
}

void Drv_USART_Init(void)
{
    for (uint8_t i = 0; i < USART_NUM; i++)
    {
        // Get Hardware
        USART_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_USART_HWs) / sizeof(g_USART_HWs[0]); j++)
        {
            if (g_USART_HWs[j].ID == (USART_ID)i)
                hw = g_USART_HWs[j];
        }

        // Data Init
        g_USART_Datas[i].ID = (USART_ID)i;
        g_USART_Datas[i].HW = hw;

        // Device Init
        g_USART_Devs[i].SendData = SendData;
        g_USART_Devs[i].ReceiveData = ReceiveData;
        g_USART_Devs[i].Priv_Data = (void*)&g_USART_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = hw.PIN_TX;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = hw.MODE_TX;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = hw.PIN_RX;
        GPIO_InitStructure.GPIO_Mode = hw.MODE_RX;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);

        RCC_APB2PeriphClockCmd(hw.USART_RCC, ENABLE);
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = hw.USART_BaudRate;
        USART_InitStructure.USART_WordLength = hw.USART_WordLength;
        USART_InitStructure.USART_StopBits = hw.USART_StopBits;
        USART_InitStructure.USART_Parity = hw.USART_Parity;
        USART_InitStructure.USART_Mode = hw.USART_Mode;
        USART_InitStructure.USART_HardwareFlowControl = hw.USART_HardwareFlowControl;
        USART_Init(hw.USARTx, &USART_InitStructure);

        NVIC_PriorityGroupConfig(hw.NVIC_PriorityGroup);
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = hw.NVIC_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = hw.NVIC_IRQChannelPreemptionPriority;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = hw.NVIC_IRQChannelSubPriority;
        NVIC_InitStructure.NVIC_IRQChannelCmd = hw.NVIC_IRQChannelCmd;
        NVIC_Init(&NVIC_InitStructure);

        USART_ITConfig(hw.USARTx, USART_IT_RXNE, ENABLE);
        USART_Cmd(hw.USARTx, ENABLE);
    }
}


//--------------------------------------------------


static void SendData(USART_Device* pDev, uint8_t ch)
{
    USART_Data* pData = (USART_Data*)pDev->Priv_Data;
    if (pData == 0)
        return;

    /* 发送一个字节数据到USART */
    USART_SendData(pData->HW.USARTx, ch);

    /* 等待发送数据寄存器为空 */
    while (USART_GetFlagStatus(pData->HW.USARTx, USART_FLAG_TXE) == RESET);
}

static int ReceiveData(struct USART_Device* pDev)
{
    USART_Data* pData = (USART_Data*)pDev->Priv_Data;
    if (pData == 0)
        return 0;

    /* 等待串口输入数据 */
    while (USART_GetFlagStatus(pData->HW.USARTx, USART_FLAG_RXNE) == RESET);

    return (int)USART_ReceiveData(pData->HW.USARTx);
}
