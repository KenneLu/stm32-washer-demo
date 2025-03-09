#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Stop_Handle = NULL;

static void Task_Washer_Stop(void* pvParameters)
{
    while (1)
    {
        vTaskSuspendAll();	//关调度器
        printf("Task_Washer_Stop Running! \r\n");
        xTaskResumeAll();	//开调度器
        
        //等待被其他任务唤醒
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Washer_Stop();
    }
}

void Do_Create_Task_Washer_Stop(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Stop, "Task_Washer_Stop", 128, NULL, 3, &Task_Washer_Stop_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_Washer_Stop Success! \r\n");
    }
    else
    {
        printf("Stop:Create_Task_Washer_Stop! \r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Stop_Handle(void)
{
    return &Task_Washer_Stop_Handle;
}
