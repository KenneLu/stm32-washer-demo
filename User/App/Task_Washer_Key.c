#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Key_Handle = NULL;

static void Task_Washer_Key(void* pvParameters)
{
    while (1)
    {
        // vTaskSuspendAll();	//关调度器
        // printf("Task_Washer_Key Running! \r\n");
        // xTaskResumeAll();	//开调度器

        Washer_Key();

        static TickType_t xLastWakeTime;
        xLastWakeTime = xTaskGetTickCount();
        Delay_Until_ms(&xLastWakeTime, 1);
    }
}

void Do_Create_Task_Washer_Key(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Key, "Task_Washer_Key", 128, NULL, 2, &Task_Washer_Key_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_Washer_Key Success! \r\n");
    }
    else
    {
        printf("Key:Create_Task_Washer_Key! \r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Key_Handle(void)
{
    return &Task_Washer_Key_Handle;
}
