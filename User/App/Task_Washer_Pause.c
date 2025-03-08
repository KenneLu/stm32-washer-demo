#include "Task_Manager.h"
#include "Washer.h"
#include "Washer_Data.h"


static TaskHandle_t Task_Washer_Pause_Handle = NULL;

static void Task_Washer_Pause(void* pvParameters)
{
    while (1)
    {
        //等待被其他任务唤醒
        uint32_t Value = 0;
        xTaskNotifyWait(pdTRUE, pdFALSE, &Value, portMAX_DELAY);
        switch ((Washer_Pause_Logic)Value)
        {
        case W_P_PAUSE:
            Washer_Pause();
            if (*Get_Task_Washer_Run_Handle() != NULL)
                vTaskSuspend(*Get_Task_Washer_Run_Handle());
            break;

        case W_P_CONTINUE:
            Washer_Resume();
            if (*Get_Task_Washer_Run_Handle() != NULL)
                vTaskResume(*Get_Task_Washer_Run_Handle());
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
        printf("Create_Task_Washer_Pause Success! \r\n");
    }
    else
    {
        printf("Error:Create_Task_Washer_Pause! \r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Pause_Handle(void)
{
    return &Task_Washer_Pause_Handle;
}
