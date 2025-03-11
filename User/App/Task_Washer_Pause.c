#include "Task_Manager.h"
#include "Washer.h"
#include "Washer_Data.h"


static TaskHandle_t Task_Washer_Pause_Handle = NULL;

static void Task_Washer_Pause(void* pvParameters)
{
    while (1)
    {
        vTaskSuspendAll();	//关调度器
        printf("Task_Washer_Pause Running!\r\n");
        xTaskResumeAll();	//开调度器

        //等待被其他任务唤醒
        uint32_t Value = 0;
        xTaskNotifyWait(pdTRUE, pdFALSE, &Value, portMAX_DELAY);
        switch ((Washer_Pause_Logic)Value)
        {
        case W_P_PAUSE:
            Washer_Pause();
            break;

        case W_P_CONTINUE:
            Washer_Resume();
            break;

        default:
            break;
        }
    }
}

void Do_Create_Task_Washer_Pause(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Pause, "Task_Washer_Pause", 128, NULL, 1, &Task_Washer_Pause_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Pause] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Pause] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Pause_Handle(void)
{
    return &Task_Washer_Pause_Handle;
}
