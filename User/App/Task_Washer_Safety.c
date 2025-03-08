#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Safety_Handle = NULL;

static void Task_Washer_Safety(void* pvParameters)
{
    while (1)
    {
        Washer_Safety();

        static TickType_t xLastWakeTime;
        xLastWakeTime = xTaskGetTickCount();
        Delay_Until_ms(&xLastWakeTime, 1);
    }
}

void Do_Create_Task_Washer_Safety(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Safety, "Task_Washer_Safety", 128, NULL, 2, &Task_Washer_Safety_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_Washer_Safety Success! \r\n");
    }
    else
    {
        printf("Safety:Create_Task_Washer_Safety! \r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Safety_Handle(void)
{
    return &Task_Washer_Safety_Handle;
}
