#include "stm32f10x.h"                  // Device header
#include "MyAD.h"

uint16_t g_ADs_Value[16];

typedef struct
{
    AD_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN;
    GPIOMode_TypeDef MODE;
    uint32_t _RCC;
    uint8_t ADC_CHANNEL;
} AD_HARDWARE;

typedef struct {
    AD_ID ID;
    AD_HARDWARE HW;
} AD_Data;

uint16_t GetValue(AD_Device* pDev);


//--------------------------------------------------


static AD_HARDWARE g_AD_HWs[AD_NUM] = {
    {
        .ID = AD_TCRT5000,
        .PORT = GPIOA,
        .PIN = GPIO_Pin_4,
        .MODE = GPIO_Mode_AIN,
        ._RCC = RCC_APB2Periph_GPIOA,
        .ADC_CHANNEL = ADC_Channel_4,
    },
};
static AD_Data g_AD_Datas[AD_NUM];
static AD_Device g_AD_Devs[AD_NUM];


//--------------------------------------------------


AD_Device* Drv_AD_GetDevice(AD_ID ID)
{
    for (int i = 0; i < sizeof(g_AD_Devs) / sizeof(g_AD_Devs[0]); i++)
    {
        AD_Data* pData = (AD_Data*)g_AD_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_AD_Devs[i];
    }
    return 0;
}

static uint8_t g_AD_Init_Finish = 0;
void Drv_AD_Init(void)
{
    if (g_AD_Init_Finish == 1) return;

    for (uint8_t i = 0; i < AD_NUM; i++)
    {
        // Get Hardware
        AD_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_AD_HWs) / sizeof(g_AD_HWs[0]); j++)
        {
            if (g_AD_HWs[j].ID == (AD_ID)i)
                hw = g_AD_HWs[j];
        }

        // Data Init
        g_AD_Datas[i].ID = (AD_ID)i;
        g_AD_Datas[i].HW = hw;

        // Device Init
        g_AD_Devs[i].GetValue = GetValue;
        g_AD_Devs[i].Priv_Data = (void*)&g_AD_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = hw.PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = hw.MODE;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);
    }

    //配置DMA
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_ADs_Value;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_BufferSize = AD_NUM; // ADC通道数
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 硬件触发
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    //使能DMA
    DMA_Cmd(DMA1_Channel1, ENABLE); // ADC1的硬件只触发DMA1_Channel1


    //配置ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

    //软件触发，无需外部触发源
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_NbrOfChannel = AD_NUM; // ADC通道数
    ADC_Init(ADC1, &ADC_InitStructure);

    //配置通道
    for (uint8_t i = 0; i < AD_NUM; i++)
    {
        ADC_RegularChannelConfig(ADC1, g_AD_HWs[i].ADC_CHANNEL, g_AD_HWs[i].ID + 1, ADC_SampleTime_55Cycles5);
    }

    //使能ADC
    ADC_DMACmd(ADC1, ENABLE);   // 使能ADC的DMA转换
    ADC_Cmd(ADC1, ENABLE); // 使能ADC

    //校准ADC
    ADC_ResetCalibration(ADC1); // 复位校准
    while (ADC_GetResetCalibrationStatus(ADC1)); // 等待复位完成
    ADC_StartCalibration(ADC1); // 开始校准
    while (ADC_GetCalibrationStatus(ADC1)); // 等待校准完成

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 开始转换

    g_AD_Init_Finish = 1;
}


//--------------------------------------------------


uint16_t GetValue(AD_Device* pDev)
{
    AD_Data* pData = (AD_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;

    return g_ADs_Value[pData->ID];
}
