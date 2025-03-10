#include "stm32F10x.h"
#include "Sys_Timer.h"
#include "Drv_Key.h"
#include "stdlib.h"

//按键消抖时间
#define KEY_SCANTIME                SYS_MS * 20     // 20ms

//连续长按时间
#define KEY_PRESS_LONG_TIME         SYS_MS * 700    // 0.7s

//持续长按间隔时间
#define KEY_PRESS_CONTINUE_TIME     SYS_MS * 150    // 150ms

//一个触发模式可绑回调个数
#define KEY_CB_MAX_NUM              3         // 3个回调函数

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
    KeyCallBack* Press;              // 按下
    KeyCallBack* Release;            // 释放
    KeyCallBack* LongPress;          // 长按
    KeyCallBack* LongPress_Cont;     // 持续长按
    KeyCallBack* LongPress_Release;  // 长按释放
} KEY_CB_LIST;

typedef struct {
    KEY_ID ID;
    KEY_STATUS Status;
    KEY_CB_LIST CBList;
    KEY_HARDWARE HW;
    KEY_SCAN Scan;
} KEY_Data;

uint8_t CBRegister_P(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBRegister_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBRegister_LP(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBRegister_LP_Cont(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBRegister_LP_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBUnregister_P(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBUnregister_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBUnregister_LP(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBUnregister_LP_Cont(KEY_Device* pDev, KeyCallBack CB);
uint8_t CBUnregister_LP_R(KEY_Device* pDev, KeyCallBack CB);
uint8_t Is_Press(KEY_Device* pDev);


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


KEY_Device* Drv_Key_GetDevice(KEY_ID ID)
{
    for (int i = 0; i < sizeof(g_Key_Devs) / sizeof(g_Key_Devs[0]); i++)
    {
        KEY_Data* pData = (KEY_Data*)g_Key_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_Key_Devs[i];
    }
    return 0;
}

void Drv_Key_Init(void)
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

        g_Key_Datas[i].Scan.ScanStep = STEP_WAIT;
        g_Key_Datas[i].Scan.ShakeTime = KEY_SCANTIME;
        g_Key_Datas[i].Scan.LongPressTime = KEY_PRESS_LONG_TIME;
        g_Key_Datas[i].Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
        g_Key_Datas[i].HW = hw;

        // Device Init
        g_Key_Devs[i].CBRegister_P = CBRegister_P;
        g_Key_Devs[i].CBRegister_R = CBRegister_R;
        g_Key_Devs[i].CBRegister_LP = CBRegister_LP;
        g_Key_Devs[i].CBRegister_LP_Cont = CBRegister_LP_Cont;
        g_Key_Devs[i].CBRegister_LP_R = CBRegister_LP_R;
        g_Key_Devs[i].CBUnregister_P = CBUnregister_P;
        g_Key_Devs[i].CBUnregister_R = CBUnregister_R;
        g_Key_Devs[i].CBUnregister_LP = CBUnregister_LP;
        g_Key_Devs[i].CBUnregister_LP_Cont = CBUnregister_LP_Cont;
        g_Key_Devs[i].CBUnregister_LP_R = CBUnregister_LP_R;
        g_Key_Devs[i].Is_Press = Is_Press;
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


//--------------------------------------------------


uint8_t Register(KeyCallBack** ppCB, KeyCallBack CB)
{
    if (*ppCB == 0)
        *ppCB = malloc(sizeof(KeyCallBack) * KEY_CB_MAX_NUM);
    for (uint8_t i = 0; i < KEY_CB_MAX_NUM; i++)
    {
        if ((*ppCB)[i] != 0)
        {
            if ((*ppCB)[i] == CB)
                return 0; //已经注册过了
        }
        else
        {
            (*ppCB)[i] = CB;
            return 1;
        }
    }
    return 0; //没位置了，注册失败
}

uint8_t Unregister(KeyCallBack* pCB, KeyCallBack CB)
{
    if (pCB == 0)
        return 0;
    for (uint8_t i = 0; i < KEY_CB_MAX_NUM; i++)
    {
        if (pCB[i] != 0)
        {
            if (pCB[i] == CB)
            {
                pCB[i] = 0;
                return 1; //找到了，注销成功
            }
        }
    }
    return 0; //没有找到该回调函数
}

void Brodcase_Callback(KeyCallBack* pCB)
{
    // 广播回调函数
    for (uint8_t i = 0; i < KEY_CB_MAX_NUM; i++)
    {
        if (pCB[i] != 0) pCB[i]();
        else break;
    }
}

// CBList Register
uint8_t CBRegister_P(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Register(&(pData->CBList.Press), CB);
}

uint8_t CBRegister_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Register(&(pData->CBList.Release), CB);
}

uint8_t CBRegister_LP(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Register(&(pData->CBList.LongPress), CB);
}

uint8_t CBRegister_LP_Cont(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Register(&(pData->CBList.LongPress_Cont), CB);
}

uint8_t CBRegister_LP_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Register(&(pData->CBList.LongPress_Release), CB);
}

// CBList Unregister
uint8_t CBUnregister_P(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Unregister(pData->CBList.Press, CB);
}

uint8_t CBUnregister_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Unregister(pData->CBList.Release, CB);
}

uint8_t CBUnregister_LP(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Unregister(pData->CBList.LongPress, CB);
}

uint8_t CBUnregister_LP_Cont(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Unregister(pData->CBList.LongPress_Cont, CB);
}

uint8_t CBUnregister_LP_R(KEY_Device* pDev, KeyCallBack CB)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData == 0) return 0;
    return Unregister(pData->CBList.LongPress_Release, CB);
}


uint8_t Is_Press(KEY_Device* pDev)
{
    KEY_Data* pData = (KEY_Data*)pDev->Priv_Data;
    if (pData->HW.Is_High_Active)
    {
        return (GPIO_ReadInputDataBit(pData->HW.PORT, pData->HW.PIN));
    }
    else
    {
        return (!GPIO_ReadInputDataBit(pData->HW.PORT, pData->HW.PIN));
    }
}

void Drv_Key_Scan(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++)
    {
        KEY_Data** pData = (KEY_Data**)&(g_Key_Devs[i].Priv_Data);
        uint8_t KeyPressed = Is_Press(&g_Key_Devs[i]);
        switch ((*pData)->Scan.ScanStep)
        {
        case STEP_WAIT: //等待阶段
            if (KeyPressed)
            {
                (*pData)->Scan.ScanStep = STEP_PRESS;
            }
            break;
        case STEP_PRESS: //单击按下阶段
            if (KeyPressed)
            {
                if ((--((*pData)->Scan.ShakeTime)) == 0)
                {
                    (*pData)->Scan.ShakeTime = KEY_SCANTIME;
                    (*pData)->Scan.ScanStep = STEP_LONG_PRESS;
                    (*pData)->Status = KEY_PRESS;  //按键单击按下
                    if ((*pData)->CBList.Press)
                    {
                        Brodcase_Callback((*pData)->CBList.Press);
                    }
                }
            }
            else
            {
                (*pData)->Scan.ShakeTime = KEY_SCANTIME;
                (*pData)->Scan.ScanStep = STEP_WAIT;
            }
            break;
        case STEP_LONG_PRESS: //长按按下阶段
            if (KeyPressed)
            {
                if ((--((*pData)->Scan.LongPressTime)) == 0)
                {
                    (*pData)->Scan.LongPressTime = KEY_PRESS_LONG_TIME;
                    (*pData)->Scan.ScanStep = STEP_CONTINUOUS_PRESS;
                    (*pData)->Status = KEY_LONG_PRESS;  //按键单击按下
                    if ((*pData)->CBList.LongPress)
                    {
                        Brodcase_Callback((*pData)->CBList.LongPress);
                    }
                }
            }
            else
            {
                (*pData)->Scan.LongPressTime = KEY_PRESS_LONG_TIME;
                (*pData)->Scan.ScanStep = STEP_WAIT;
                (*pData)->Status = KEY_RELEASE;  //按键单击释放
                if ((*pData)->CBList.Release)
                {
                    Brodcase_Callback((*pData)->CBList.Release);
                }
            }
            break;
        case STEP_CONTINUOUS_PRESS: //持续长按阶段
            if (KeyPressed)
            {
                if ((--((*pData)->Scan.ContPressTime)) == 0)
                {
                    (*pData)->Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
                    (*pData)->Status = KEY_LONG_PRESS_CONTINUOUS;  //按键长按持续
                    if ((*pData)->CBList.LongPress_Cont)
                    {
                        Brodcase_Callback((*pData)->CBList.LongPress_Cont);
                    }
                }
            }
            else
            {
                (*pData)->Scan.ContPressTime = KEY_PRESS_CONTINUE_TIME;
                (*pData)->Scan.ScanStep = STEP_WAIT;
                (*pData)->Status = KEY_LONG_PRESS_RELEASE;  //按键长按释放
                if ((*pData)->CBList.LongPress_Release)
                {
                    Brodcase_Callback((*pData)->CBList.LongPress_Release);
                }
            }
            break;
        default:
            break;
        }
    }
}
