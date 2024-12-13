#include "stm32F10x.h"
#include "Key.h"

KeyEvent_CallBack_t KeyScanCB;

static void KeyConfig(void);
static uint8_t GetKeyS1Sta(void);
static uint8_t GetKeyS2Sta(void);

uint8_t(*GetKeysStatus[]) () = { GetKeyS1Sta, GetKeyS2Sta };
uint8_t KeyStep[KEYNUM];              //按键检测流程
uint16_t KeyScanTime[KEYNUM];         //消抖
uint16_t KeyPressLongTimer[KEYNUM];   //长按
uint16_t KeyContPressTimer[KEYNUM];   //连续长按


void KeyInit(void)
{
    KeyScanCB = 0;
    KeyConfig();

    for (uint8_t i = 0; i < KEYNUM; i++)
    {
        KeyStep[i] = KEY_STEP_WAIT;
        KeyScanTime[i] = KEY_SCANTIME;
        KeyPressLongTimer[i] = KEY_PRESS_LONG_TIME;
        KeyContPressTimer[i] = KEY_PRESS_CONTINUE_TIME;
    }
}

void KeyScanCBRegister(KeyEvent_CallBack_t pCB)
{
    if (KeyScanCB == 0)
    {
        KeyScanCB = pCB;
    }
}

void KeyProc(void)
{
    uint8_t KeyStatus[KEYNUM];
    uint8_t Keys;
    uint8_t Status;

    for (uint8_t i = 0; i < KEYNUM; i++)
    {
        Keys = 0;
        Status = 0;
        KeyStatus[i] = GetKeysStatus[i]();
        switch (KeyStep[i])
        {
        case KEY_STEP_WAIT: //等待按键
            if (KeyStatus[i])
            {
                KeyStep[i] = KEY_STEP_CLICK;
            }
            break;
        case KEY_STEP_CLICK: //按键单击按下
            if (KeyStatus[i])
            {
                if (!(--KeyScanTime[i]))
                {
                    KeyScanTime[i] = KEY_SCANTIME;
                    KeyStep[i] = KEY_STEP_LONG_PRESS;
                    Keys = i + 1;   //记录按键ID号
                    Status = KEY_CLICK;  //按键单击按下
                }
            }
            else
            {
                KeyScanTime[i] = KEY_SCANTIME;
                KeyStep[i] = KEY_STEP_WAIT;
            }
            break;
        case KEY_STEP_LONG_PRESS: //按键长按
            if (KeyStatus[i])
            {
                if (!(--KeyPressLongTimer[i]))
                {
                    KeyPressLongTimer[i] = KEY_PRESS_LONG_TIME;
                    KeyStep[i] = KEY_STEP_CONTINUOUS_PRESS;
                    Keys = i + 1;   //记录按键ID号
                    Status = KEY_LONG_PRESS;  //长按确认
                }
            }
            else
            {
                KeyPressLongTimer[i] = KEY_PRESS_LONG_TIME;
                KeyStep[i] = KEY_STEP_WAIT;
                Keys = i + 1;   //记录按键ID号
                Status = KEY_CLICK_RELEASE;  //单击释放
            }
            break;
        case KEY_STEP_CONTINUOUS_PRESS:
            if (KeyStatus[i])
            {
                if (!(--KeyContPressTimer[i]))
                {
                    KeyContPressTimer[i] = KEY_PRESS_CONTINUE_TIME;
                    Keys = i + 1;   //记录按键ID号
                    Status = KEY_LONG_PRESS_CONTINUOUS;  //持续长按
                }
            }
            else
            {
                KeyContPressTimer[i] = KEY_PRESS_CONTINUE_TIME;
                KeyStep[i] = KEY_STEP_WAIT;
                Keys = i + 1;   //记录按键ID号
                Status = KEY_LONG_PRESS_RELEASE;  //长按释放
            }
            break;
        default:
            break;
        }

        if (Keys)
        {
            if (KeyScanCB)
            {
                KeyScanCB((KEY_TYPEDEF)(Keys - 1), (KEY_EVENT_TYPEDEF)Status);
            }
        }
    }
}

static void KeyConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(S1_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = S1_GOIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = S1_GOIO_MODE;
    GPIO_Init(S1_GOIO_x, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(S2_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = S2_GOIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = S2_GOIO_MODE;
    GPIO_Init(S2_GOIO_x, &GPIO_InitStructure);
}

static uint8_t GetKeyS1Sta(void)
{
    return (!GPIO_ReadInputDataBit(S1_GOIO_x, S1_GOIO_PIN));
}

static uint8_t GetKeyS2Sta(void)
{
    return (GPIO_ReadInputDataBit(S2_GOIO_x, S2_GOIO_PIN));
}
