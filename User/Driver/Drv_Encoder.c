#include "stm32f10x.h"                  //Device header
#include "Drv_Encoder.h"

typedef struct
{
    ENCODER_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN_A;
    uint16_t PIN_B;
    GPIOMode_TypeDef MODE;
    uint32_t _RCC;
    TIM_TypeDef* TIM_PORT;
    uint32_t TIM_RCC;
    uint16_t TIM_CHANNEL_1;
    uint16_t TIM_CHANNEL_2;
    uint16_t TIM_ENCODERMODE;
    uint16_t TIM_IC1POLAROTY;
    uint16_t TIM_IC2POLARITY;
    uint16_t TIM_CLOCKDIVISION;
    uint16_t TIM_PERIOD;
    uint16_t TIM_PRESCALER;
    uint8_t TIM_REPETITIONCOUNTER;
} ENCODER_HARDWARE;

typedef struct {
    ENCODER_ID ID;
    ENCODER_HARDWARE HW;
} ENCODER_Data;

int16_t GetCount(ENCODER_Device* pDev);
int16_t GetSpeed(ENCODER_Device* pDev);
int16_t GetDiv4(ENCODER_Device* pDev);


//--------------------------------------------------


static ENCODER_HARDWARE g_Encoder_HWs[ENCODER_NUM] = {
    {
        .ID = ENCODER,
        .PORT = GPIOA,
        .PIN_A = GPIO_Pin_6,
        .PIN_B = GPIO_Pin_7,
        .MODE = GPIO_Mode_IPU,
        ._RCC = RCC_APB2Periph_GPIOA,
        .TIM_PORT = TIM3,
        .TIM_RCC = RCC_APB1Periph_TIM3,
        .TIM_CHANNEL_1 = TIM_Channel_1,
        .TIM_CHANNEL_2 = TIM_Channel_2,
        .TIM_ENCODERMODE = TIM_EncoderMode_TI12,
        .TIM_IC1POLAROTY = TIM_ICPolarity_Falling,
        .TIM_IC2POLARITY = TIM_ICPolarity_Rising,
        .TIM_CLOCKDIVISION = TIM_CKD_DIV1,
        .TIM_PERIOD = 65536 - 1, //利用补码特性得到负数
        .TIM_PRESCALER = 1 - 1,
        .TIM_REPETITIONCOUNTER = 0,
    },
};
static ENCODER_Data g_Encoder_Datas[ENCODER_NUM];
static ENCODER_Device g_Encoder_Devs[ENCODER_NUM];


//--------------------------------------------------


ENCODER_Device* Drv_Encoder_GetDevice(ENCODER_ID ID)
{
    for (int i = 0; i < sizeof(g_Encoder_Devs) / sizeof(g_Encoder_Devs[0]); i++)
    {
        ENCODER_Data* pData = (ENCODER_Data*)g_Encoder_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_Encoder_Devs[i];
    }
    return 0;
}

void Dev_Encoder_Init(void)
{
    for (uint8_t i = 0; i < ENCODER_NUM; i++)
    {
        // Get Hardware
        ENCODER_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_Encoder_HWs) / sizeof(g_Encoder_HWs[0]); j++)
        {
            if (g_Encoder_HWs[j].ID == (ENCODER_ID)i)
                hw = g_Encoder_HWs[j];
        }

        // Data Init
        g_Encoder_Datas[i].ID = (ENCODER_ID)i;
        g_Encoder_Datas[i].HW = hw;

        // Device Init
        g_Encoder_Devs[i].GetCount = GetCount;
        g_Encoder_Devs[i].GetSpeed = GetSpeed;
        g_Encoder_Devs[i].GetDiv4 = GetDiv4;
        g_Encoder_Devs[i].Priv_Data = (void*)&g_Encoder_Datas[i];

        // Hardware Init

        //开启 GPIO 时钟
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);

        //初始化 GPIO
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = hw.MODE;
        GPIO_InitStructure.GPIO_Pin = hw.PIN_A | hw.PIN_B;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);

        //初始化 IC
        TIM_ICInitTypeDef TIM_ICInitStructure;
        TIM_ICStructInit(&TIM_ICInitStructure);
        TIM_ICInitStructure.TIM_Channel = hw.TIM_CHANNEL_1;
        TIM_ICInitStructure.TIM_ICFilter = 0xF;
        TIM_ICInit(hw.TIM_PORT, &TIM_ICInitStructure);
        TIM_ICInitStructure.TIM_Channel = hw.TIM_CHANNEL_2;
        TIM_ICInitStructure.TIM_ICFilter = 0xF;
        TIM_ICInit(hw.TIM_PORT, &TIM_ICInitStructure);

        //开启 Encoder_TIM_x 时钟
        RCC_APB1PeriphClockCmd(hw.TIM_RCC, ENABLE);

        //初始化编码器接口
        TIM_EncoderInterfaceConfig(
            hw.TIM_PORT,
            hw.TIM_ENCODERMODE,
            hw.TIM_IC1POLAROTY,
            hw.TIM_IC2POLARITY
        );

        //配置时基单元
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
        TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);

        TIM_TimeBaseInitStructure.TIM_ClockDivision = hw.TIM_CLOCKDIVISION;
        TIM_TimeBaseInitStructure.TIM_Period = hw.TIM_PERIOD;
        TIM_TimeBaseInitStructure.TIM_Prescaler = hw.TIM_PRESCALER;
        TIM_TimeBaseInitStructure.TIM_RepetitionCounter = hw.TIM_REPETITIONCOUNTER;
        TIM_TimeBaseInit(hw.TIM_PORT, &TIM_TimeBaseInitStructure);

        //启动定时器
        TIM_Cmd(hw.TIM_PORT, ENABLE);
    }
}


//--------------------------------------------------


int16_t GetCount(ENCODER_Device* pDev)
{
    ENCODER_Data* pData = (ENCODER_Data*)pDev->Priv_Data;

    //编码器计数器的计数值是 4 倍的 TIM_GetCounter() 的值
    return (int16_t)TIM_GetCounter(pData->HW.TIM_PORT) / 4;
}

int16_t GetSpeed(ENCODER_Device* pDev)
{
    ENCODER_Data* pData = (ENCODER_Data*)pDev->Priv_Data;
    int16_t Temp = TIM_GetCounter(pData->HW.TIM_PORT);
    TIM_SetCounter(pData->HW.TIM_PORT, 0);
    return Temp;
}

int16_t GetDiv4(ENCODER_Device* pDev)
{
    ENCODER_Data* pData = (ENCODER_Data*)pDev->Priv_Data;

    //踩坑记录:TIM_GetCounter()返回无符号16位整数, 赋值给有符号整型前不要做运算
    int16_t Temp = TIM_GetCounter(pData->HW.TIM_PORT);

    if (Temp / 4)
    {
        TIM_SetCounter(pData->HW.TIM_PORT, Temp % 4);
        return Temp / 4;
    }
    return 0;
}
