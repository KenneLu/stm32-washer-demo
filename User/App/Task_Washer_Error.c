#include "Task_Manager.h"
#include "Washer.h"
#include "Washer_Data.h"


static TaskHandle_t Task_Washer_Error_Handle = NULL;

static void Task_Washer_Error(void* pvParameters)
{
    while (1)
    {
        vTaskSuspendAll();	//关调度器
        printf("Task_Washer_Error Running! \r\n");
        xTaskResumeAll();	//开调度器

        //等待被其他任务唤醒
        uint32_t Value = 0;
        xTaskNotifyWait(pdFALSE, pdFALSE, &Value, 0);
        switch ((Washer_Error_Logic)Value)
        {
        case W_E_OCCUR:
            printf("W_E_OCCUR\r\n");
            Washer_Error_Occur();
            Washer_Error_Warning();
            printf("Washer_Error_Warning\r\n");
            if (*Get_Task_Washer_Run_Handle())
                vTaskSuspend(*Get_Task_Washer_Run_Handle());
            break;

        case W_E_FIXED:
            printf("W_E_FIXED\r\n");
            Washer_Error_Fixed();
            break;

        default:
            break;
        }
    }
}

void Do_Create_Task_Washer_Error(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Error, "Task_Washer_Error", 128, NULL, 1, &Task_Washer_Error_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Error] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Error] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Error_Handle(void)
{
    return &Task_Washer_Error_Handle;
}
