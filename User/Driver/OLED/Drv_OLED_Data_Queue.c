#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Drv_OLED_Data.h"
#include "Drv_OLED.h"



#define OLED_QUEUE_LENGTH 50

static QueueHandle_t g_OLED_Data_Queue;
static uint8_t g_Is_Init = 0;


void Drv_OLED_Data_Queue_Init(void)
{
    g_OLED_Data_Queue = xQueueCreate(OLED_QUEUE_LENGTH, sizeof(OLED_DATA_DISPLAY));
    g_Is_Init = 1;
}

void OLED_Send(OLED_DATA_DISPLAY Display)
{
    if (g_Is_Init == 0)
        return;

    xQueueSend(g_OLED_Data_Queue, &Display, 0);
}

void OLED_Display(void)
{
    if (g_Is_Init == 0)
        return;

    OLED_DATA_DISPLAY Display;

    uint8_t ret = 0;
    ret = xQueueReceive(g_OLED_Data_Queue, &Display, portMAX_DELAY);
    if (ret != 0)
    {
        switch (Display.Cmd)
        {
        case OLED_CMD_CLEAR:
            OLED_Clear();
            break;

        case OLED_CMD_UPDATE:
            OLED_Update_Pure();
            break;

        case OLED_CMD_PUSH:
            switch (Display.Type)
            {
            case OLED_NUM:
                OLED_ShowNum(
                    Display.Union.Num.Line,
                    Display.Union.Num.Column,
                    Display.Union.Num.Number,
                    Display.Union.Num.Length,
                    OLED_6X8);
                break;

            case OLED_STR:
                OLED_ShowString(
                    Display.Union.Str.Line,
                    Display.Union.Str.Column,
                    Display.Union.Str.String,
                    OLED_8X16);
                break;

            case OLED_RECT:
                OLED_DrawRectangle(
                    Display.Union.Rect.X,
                    Display.Union.Rect.Y,
                    Display.Union.Rect.Width,
                    Display.Union.Rect.Height,
                    Display.Union.Rect.IsFilled);
                break;

            case OLED_NUM_E:
                OLED_ShowNum(
                    --Display.Union.Num.Column * 8,
                    --Display.Union.Num.Line * 16,
                    Display.Union.Num.Number,
                    Display.Union.Num.Length,
                    OLED_8X16);
                break;

            case OLED_STR_E:
                OLED_ShowString(
                    --Display.Union.Str.Column * 8,
                    --Display.Union.Str.Line * 16,
                    Display.Union.Str.String,
                    OLED_8X16);
                break;

            case OLED_CN_E:
                OLED_ShowChinese(
                    --Display.Union.CN.Column * 16,
                    --Display.Union.CN.Line * 16,
                    Display.Union.CN.Chinese);
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }
}

