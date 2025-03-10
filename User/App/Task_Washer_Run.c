#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Run_Handle = NULL;

static void Task_Washer_Run(void* pvParameters)
{
    while (1)
    {
        // vTaskSuspendAll();	//关调度器
        // printf("Task_Washer_Run Running! \r\n");
        // xTaskResumeAll();	//开调度器

        Washer_State_Machine();
    }
}

void Do_Create_Task_Washer_Run(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Run, "Task_Washer_Run", 512, NULL, 0, &Task_Washer_Run_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_Washer_Run Success! \r\n");
    }
    else
    {
        printf("Error:Create_Task_Washer_Run! \r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Run_Handle(void)
{
    return &Task_Washer_Run_Handle;
}
