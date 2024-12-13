#include "stm32F10x.h"
#include "Key.h"

MY_KEY KEYS[KEYNUM] =
{
    {
    .ID = KEY_ENCODER_PRESS,
    .Status = KEY_IDLE,
    .GPIO = {
        .GPIO_x = GPIOA,
        .GPIO_RCC = RCC_APB2Periph_GPIOA,
        .GPIO_PIN = GPIO_Pin_5,
        .GPIO_MODE = GPIO_Mode_IPU,
        .High_Active = 0
        }
    }
};


void Key_Init(void)
{
    for (uint8_t i = 0; i < KEYNUM; i++)
    {
        KEYS[i].Status = KEY_IDLE;

        KEYS[i].Callback.Press = 0;
        KEYS[i].Callback.Release = 0;
        KEYS[i].Callback.LongPress = 0;
        KEYS[i].Callback.LongPress_Cont = 0;
        KEYS[i].Callback.LongPress_Release = 0;

        KEYS[i].Scan.ScanStep = STEP_WAIT;
        KEYS[i].Scan.ShakeTime = KEY_SCANTIME;
        KEYS[i].Scan.LongPressTimer = KEY_PRESS_LONG_TIME;
        KEYS[i].Scan.ContPressTimer = KEY_PRESS_CONTINUE_TIME;

        RCC_APB2PeriphClockCmd(KEYS[i].GPIO.GPIO_RCC, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = KEYS[i].GPIO.GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = KEYS[i].GPIO.GPIO_MODE;
        GPIO_Init(KEYS[i].GPIO.GPIO_x, &GPIO_InitStructure);
    }
}

void Key_CBRegister_P(KEY_ID KeyID, KeyCallBack CB)
{
    if (KEYS[KeyID].Callback.Press == 0)
    {
        KEYS[KeyID].Callback.Press = CB;
    }
}

void Key_CBRegister_R(KEY_ID KeyID, KeyCallBack CB)
{
    if (KEYS[KeyID].Callback.Release == 0)
    {
        KEYS[KeyID].Callback.Release = CB;
    }
}

void Key_CBRegister_LP(KEY_ID KeyID, KeyCallBack CB)
{
    if (KEYS[KeyID].Callback.LongPress == 0)
    {
        KEYS[KeyID].Callback.LongPress = CB;
    }
}

void Key_CBRegister_LP_Cont(KEY_ID KeyID, KeyCallBack CB)
{
    if (KEYS[KeyID].Callback.LongPress_Cont == 0)
    {
        KEYS[KeyID].Callback.LongPress_Cont = CB;
    }
}

void Key_CBRegister_LP_R(KEY_ID KeyID, KeyCallBack CB)
{
    if (KEYS[KeyID].Callback.LongPress_Release == 0)
    {
        KEYS[KeyID].Callback.LongPress_Release = CB;
    }
}

uint8_t Key_Get_GPIO_Status(KEY_ID KeyID)
{
    MY_KEY Key = KEYS[KeyID];
    if (Key.GPIO.High_Active)
    {
        return (GPIO_ReadInputDataBit(Key.GPIO.GPIO_x, Key.GPIO.GPIO_PIN));
    }
    else
    {
        return (!GPIO_ReadInputDataBit(Key.GPIO.GPIO_x, Key.GPIO.GPIO_PIN));
    }
}

void Key_Scan(void)
{
    for (uint8_t i = 0; i < KEYNUM; i++)
    {
        MY_KEY Key = KEYS[i];
        uint8_t KeyPressed = Key_Get_GPIO_Status(Key.ID);
        switch (Key.Scan.ScanStep)
        {
        case STEP_WAIT: //等待阶段
            if (KeyPressed)
            {
                Key.Scan.ScanStep = STEP_PRESS;
            }
            break;
        case STEP_PRESS: //单击按下阶段
            if (KeyPressed)
            {
                if ((--Key.Scan.ShakeTime) == 0)
                {
                    Key.Scan.ShakeTime = KEY_SCANTIME;
                    Key.Scan.ScanStep = STEP_LONG_PRESS;
                    Key.Status = KEY_PRESS;  //按键单击按下
                    if (Key.Callback.Press)
                    {
                        Key.Callback.Press();
                    }
                }
            }
            else
            {
                Key.Scan.ShakeTime = KEY_SCANTIME;
                Key.Scan.ScanStep = STEP_WAIT;
            }
            break;
        case STEP_LONG_PRESS: //长按按下阶段
            if (KeyPressed)
            {
                if ((--Key.Scan.LongPressTimer) == 0)
                {
                    Key.Scan.LongPressTimer = KEY_PRESS_LONG_TIME;
                    Key.Scan.ScanStep = STEP_CONTINUOUS_PRESS;
                    Key.Status = KEY_LONG_PRESS;  //按键单击按下
                    if (Key.Callback.LongPress)
                    {
                        Key.Callback.LongPress();
                    }
                }
            }
            else
            {
                Key.Scan.LongPressTimer = KEY_PRESS_LONG_TIME;
                Key.Scan.ScanStep = STEP_WAIT;
                Key.Status = KEY_RELEASE;  //按键单击释放
                if (Key.Callback.Release)
                {
                    Key.Callback.Release();
                }
            }
            break;
        case STEP_CONTINUOUS_PRESS: //持续长按阶段
            if (KeyPressed)
            {
                if ((--Key.Scan.ContPressTimer) == 0)
                {
                    Key.Scan.ContPressTimer = KEY_PRESS_CONTINUE_TIME;
                    Key.Status = KEY_LONG_PRESS_CONTINUOUS;  //按键长按持续
                    if (Key.Callback.LongPress_Cont)
                    {
                        Key.Callback.LongPress_Cont();
                    }
                }
            }
            else
            {
                Key.Scan.ContPressTimer = KEY_PRESS_CONTINUE_TIME;
                Key.Scan.ScanStep = STEP_WAIT;
                Key.Status = KEY_LONG_PRESS_RELEASE;  //按键长按释放
                if (Key.Callback.LongPress_Release)
                {
                    Key.Callback.LongPress_Release();
                }
            }
            break;
        default:
            break;
        }

        KEYS[i] = Key; //更新按键数据
    }
}
