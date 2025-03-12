#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Run_Handle = NULL;

static void Task_Washer_Run(void* pvParameters)
{
    while (1)
    {
        // printf("Task_Washer_Run Running! \r\n");

        Washer_State_Machine();
    }
}

void Do_Create_Task_Washer_Run(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Run, "Task_Washer_Run", 128, NULL, 0, &Task_Washer_Run_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Run] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Run] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Run_Handle(void)
{
    return &Task_Washer_Run_Handle;
}
