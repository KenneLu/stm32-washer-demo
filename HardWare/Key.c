#include "stm32F10x.h"
#include "Key.h"
#include "string.h"
#include "Timer.h"

//按键消抖时间
#define KEY_SCANTIME                SYS_MS * 20     // 20ms

//连续长按时间
#define KEY_PRESS_LONG_TIME         SYS_MS * 800    // 0.5s

//持续长按间隔时间
#define KEY_PRESS_CONTINUE_TIME     SYS_MS * 150    // 150ms

typedef enum
{
    STEP_WAIT,              // 等待按下
    STEP_PRESS,             // 单击按下
    STEP_LONG_PRESS,        // 长按按下
    STEP_CONTINUOUS_PRESS,  // 持续长按
} SCAN_STEP; // 按键检测过程

typedef struct
{
    SCAN_STEP ScanStep;
    uint16_t ShakeTime;        // 消抖
    uint16_t LongPressTime;   // 长按
    uint16_t ContPressTime;   // 连续长按
} KEY_SCAN; // 按键扫描相关

typedef enum
{
    KEY_IDLE,                    // 空闲
    KEY_PRESS,                   // 单击按下
    KEY_RELEASE,                 // 单击释放
    KEY_LONG_PRESS,              // 长按按下
    KEY_LONG_PRESS_CONTINUOUS,   // 长按持续
    KEY_LONG_PRESS_RELEASE,      // 长按释放
} KEY_STATUS;

typedef struct
{
    KEY_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN;
    GPIOMode_TypeDef MODE;
    uint32_t _RCC;
    uint8_t Is_High_Active;        // 高电平有效
} KEY_HARDWARE;

typedef struct
{
    KeyCallBack Press;              // 按下
    KeyCallBack Release;            // 释放
    KeyCallBack LongPress;          // 长按
    KeyCallBack LongPress_Cont;     // 持续长按
    KeyCallBack LongPress_Release;  // 长按释放
} KEY_CALLBACK;

typedef struct {
    KEY_ID ID;
    KEY_STATUS Status;
    KEY_CALLBACK Callback;
    KEY_HARDWARE HW;
    KEY_SCAN Scan;
} KEY_Data;

uint8_t Key_CBRegister_P(KEY_Device* pDev, KeyCallBack CB);
uint8_t Key_CBRegister_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t Key_CBRegister_LP(KEY_Device* pDev, KeyCallBack CB);
uint8_t Key_CBRegister_LP_Cont(KEY_Device* pDev, KeyCallBack CB);
uint8_t Key_CBRegister_LP_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t Is_Key_Pressed(KEY_Device* pDev);


//--------------------------------------------------


static KEY_HARDWARE g_Key_HWs[KEY_NUM] = {
    //电源按钮
    {
        .ID = KEY_POWER,
        .PORT = GPIOA,
        .PIN = GPIO_Pin_0,
        .MODE = GPIO_Mode_IPD,
        ._RCC = RCC_APB2Periph_GPIOA,
        .Is_High_Active = 1
    },
    //编码器按下
    {
        .ID = KEY_ENCODER,
        .PORT = GPIOA,
        .PIN = GPIO_Pin_5,
        .MODE = GPIO_Mode_IPU,
        ._RCC = RCC_APB2Periph_GPIOA,
        .Is_High_Active = 0
    },
};
static KEY_Data g_Key_Datas[KEY_NUM];
static KEY_Device g_Key_Devs[KEY_NUM];


//--------------------------------------------------


KEY_Device* GetKeyDevice(KEY_ID ID)
{
    for (int i = 0; i < sizeof(g_Key_Devs) / sizeof(g_Key_Devs[0]); i++)
    {
        KEY_Data* data = (KEY_Data*)g_Key_Devs[i].Priv_Data;
        if (!data) return NULL;
        if (data->ID == ID)
            return &g_Key_Devs[i];
    }

    return NULL;
}

void Key_Init(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++)
    {
        // Get Hardware
        KEY_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_Key_HWs) / sizeof(g_Key_HWs[0]); j++)
        {
            if (g_Key_HWs[j].ID == (KEY_ID)i)
                hw = g_Key_HWs[j];
        }

        // Data Init
        g_Key_Datas[i].ID = (KEY_ID)i;
        g_Key_Datas[i].Status = KEY_IDLE;
        g_Key_Datas[i].Callback.Press = 0;
        g_Key_Datas[i].Callback.Release = 0;
        g_Key_Datas[i].Callback.LongPress = 0;
        g_Key_Datas[i].Callback.LongPress_Cont = 0;
        g_Key_Datas[i].Callback.LongPress_Release = 0;
        g_Key_Datas[i].Scan.ScanStep = STEP_WAIT;
        g_Key_Datas[i].Scan.ShakeTime = KEY_SCANTIME;
        g_Key_Datas[i].Scan.LongPressTime = KEY_PRESS_LONG_TIME;
        g_Key_Datas[i].Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
        g_Key_Datas[i].HW = hw;

        // Device Init
        g_Key_Devs[i].Key_CBRegister_P = Key_CBRegister_P;
        g_Key_Devs[i].Key_CBRegister_R = Key_CBRegister_R;
        g_Key_Devs[i].Key_CBRegister_LP = Key_CBRegister_LP;
        g_Key_Devs[i].Key_CBRegister_LP_Cont = Key_CBRegister_LP_Cont;
        g_Key_Devs[i].Key_CBRegister_LP_R = Key_CBRegister_LP_R;
        g_Key_Devs[i].Is_Key_Pressed = Is_Key_Pressed;
        g_Key_Devs[i].Priv_Data = (void*)&g_Key_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(hw._RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = hw.PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = hw.MODE;
        GPIO_Init(hw.PORT, &GPIO_InitStructure);
    }
}

uint8_t Key_CBRegister_P(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->Callback.Press == 0)
    {
        data->Callback.Press = CB;
        return 1;
    }
    return 0;
}

uint8_t Key_CBRegister_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->Callback.Release == 0)
    {
        data->Callback.Release = CB;
        return 1;
    }
    return 0;
}

uint8_t Key_CBRegister_LP(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->Callback.LongPress == 0)
    {
        data->Callback.LongPress = CB;
        return 1;
    }
    return 0;
}

uint8_t Key_CBRegister_LP_Cont(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->Callback.LongPress_Cont == 0)
    {
        data->Callback.LongPress_Cont = CB;
        return 1;
    }
    return 0;
}

uint8_t Key_CBRegister_LP_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->Callback.LongPress_Release == 0)
    {
        data->Callback.LongPress_Release = CB;
        return 1;
    }
    return 0;
}

uint8_t Is_Key_Pressed(KEY_Device* pDev)
{
    KEY_Data* data = (KEY_Data*)pDev->Priv_Data;
    if (data->HW.Is_High_Active)
    {
        return (GPIO_ReadInputDataBit(data->HW.PORT, data->HW.PIN));
    }
    else
    {
        return (!GPIO_ReadInputDataBit(data->HW.PORT, data->HW.PIN));
    }
}

void Key_Scan(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++)
    {
        KEY_Data** data = (KEY_Data**)&(g_Key_Devs[i].Priv_Data);
        uint8_t KeyPressed = Is_Key_Pressed(&g_Key_Devs[i]);
        switch ((*data)->Scan.ScanStep)
        {
        case STEP_WAIT: //等待阶段
            if (KeyPressed)
            {
                (*data)->Scan.ScanStep = STEP_PRESS;
            }
            break;
        case STEP_PRESS: //单击按下阶段
            if (KeyPressed)
            {
                if ((--((*data)->Scan.ShakeTime)) == 0)
                {
                    (*data)->Scan.ShakeTime = KEY_SCANTIME;
                    (*data)->Scan.ScanStep = STEP_LONG_PRESS;
                    (*data)->Status = KEY_PRESS;  //按键单击按下
                    if ((*data)->Callback.Press)
                    {
                        (*data)->Callback.Press();
                    }
                }
            }
            else
            {
                (*data)->Scan.ShakeTime = KEY_SCANTIME;
                (*data)->Scan.ScanStep = STEP_WAIT;
            }
            break;
        case STEP_LONG_PRESS: //长按按下阶段
            if (KeyPressed)
            {
                if ((--((*data)->Scan.LongPressTime)) == 0)
                {
                    (*data)->Scan.LongPressTime = KEY_PRESS_LONG_TIME;
                    (*data)->Scan.ScanStep = STEP_CONTINUOUS_PRESS;
                    (*data)->Status = KEY_LONG_PRESS;  //按键单击按下
                    if ((*data)->Callback.LongPress)
                    {
                        (*data)->Callback.LongPress();
                    }
                }
            }
            else
            {
                (*data)->Scan.LongPressTime = KEY_PRESS_LONG_TIME;
                (*data)->Scan.ScanStep = STEP_WAIT;
                (*data)->Status = KEY_RELEASE;  //按键单击释放
                if ((*data)->Callback.Release)
                {
                    (*data)->Callback.Release();
                }
            }
            break;
        case STEP_CONTINUOUS_PRESS: //持续长按阶段
            if (KeyPressed)
            {
                if ((--((*data)->Scan.ContPressTime)) == 0)
                {
                    (*data)->Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
                    (*data)->Status = KEY_LONG_PRESS_CONTINUOUS;  //按键长按持续
                    if ((*data)->Callback.LongPress_Cont)
                    {
                        (*data)->Callback.LongPress_Cont();
                    }
                }
            }
            else
            {
                (*data)->Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
                (*data)->Scan.ScanStep = STEP_WAIT;
                (*data)->Status = KEY_LONG_PRESS_RELEASE;  //按键长按释放
                if ((*data)->Callback.LongPress_Release)
                {
                    (*data)->Callback.LongPress_Release();
                }
            }
            break;
        default:
            break;
        }
    }
}
