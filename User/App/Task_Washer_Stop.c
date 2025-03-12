#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Stop_Handle = NULL;

static void Task_Washer_Stop(void* pvParameters)
{
    while (1)
    {
        printf("Task_Washer_Stop Running! \r\n");

        //等待被其他任务唤醒
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Washer_Stop();
    }
}

void Do_Create_Task_Washer_Stop(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Stop, "Task_Washer_Stop", 32, NULL, 3, &Task_Washer_Stop_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Stop] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Stop] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Stop_Handle(void)
{
    return &Task_Washer_Stop_Handle;
}
