#include "Task_Manager.h"
#include "Drv_Key.h"


static TaskHandle_t Task_KeyScan_Handle = NULL;

static void Task_KeyScan(void* pvParameters)
{
    while (1)
    {
        // vTaskSuspendAll();	//关调度器
        // printf("Task_KeyScan Running! \r\n");
        // xTaskResumeAll();	//开调度器

        Drv_Key_Scan();

        static TickType_t xLastWakeTime;
        xLastWakeTime = xTaskGetTickCount();
        Delay_Until_ms(&xLastWakeTime, 1);
    }
}

void Do_Create_Task_KeyScan(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_KeyScan, "Task_KeyScan", 128, NULL, 5, &Task_KeyScan_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_KeyScan Success! \r\n");
    }
    else
    {
        printf("Error:Create_Task_KeyScan! \r\n");
    }
}

TaskHandle_t* Get_Task_KeyScan_Handle(void)
{
    return &Task_KeyScan_Handle;
}
