#include "Task_Manager.h"
#include "Washer.h"


static TaskHandle_t Task_Washer_Safety_Handle = NULL;

static void Task_Washer_Safety(void* pvParameters)
{
    while (1)
    {
        // vTaskSuspendAll();	//关调度器
        // printf("Task_Washer_Safety Running! \r\n");
        // xTaskResumeAll();	//开调度器

        Washer_Safety();

        Delay_ms(100); //每100ms做一次安全检查
    }
}

void Do_Create_Task_Washer_Safety(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Safety, "Task_Washer_Safety", 128, NULL, 2, &Task_Washer_Safety_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Safety] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Safety] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Safety_Handle(void)
{
    return &Task_Washer_Safety_Handle;
}
