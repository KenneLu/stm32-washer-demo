#include "Task_Manager.h"
#include "Drv_OLED.h"
#include "Drv_Encoder.h"
#include "Drv_Key.h"
#include "Menu.h"


static TaskHandle_t Task_Start_Handle = NULL;

static void Task_Start(void* pvParameters)
{
    OLED_Init();
    Drv_Key_Init();
    Dev_Encoder_Init();
    Menu_Init();

    taskENTER_CRITICAL();
    Do_Create_Task_MainMenu();
    Do_Create_Task_KeyScan();
    vTaskDelete(Task_Start_Handle);
    taskEXIT_CRITICAL();
}

void Do_Create_Task_Start(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Start, "Task_Start", 1024, NULL, 1, &Task_Start_Handle);
    if (pdPASS == xReturn)
    {
        printf("Do_Create_Task_Start Success! \r\n");
        vTaskStartScheduler();
    }
    else
    {
        printf("Error:Do_Create_Task_Start! \r\n");
    }
}

TaskHandle_t* Get_Task_Start_Handle(void)
{
    return &Task_Start_Handle;
}
