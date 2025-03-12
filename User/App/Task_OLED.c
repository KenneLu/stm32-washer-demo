#include "Task_Manager.h"
#include "Drv_OLED.h"


static TaskHandle_t Task_OLED_Handle = NULL;

static void Task_OLED(void* pvParameters)
{
    while (1)
    {
        // printf("Task_OLED Running! \r\n");

        OLED_Display();
    }
}

void Do_Create_Task_OLED(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_OLED, "Task_OLED", 256, NULL, 1, &Task_OLED_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_OLED] Success!\r\n");
    }
    else
    {
        printf("Create [Task_OLED] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_OLED_Handle(void)
{
    return &Task_OLED_Handle;
}
