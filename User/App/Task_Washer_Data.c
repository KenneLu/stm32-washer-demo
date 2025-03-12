#include "Task_Manager.h"
#include "Washer_Data.h"


static TaskHandle_t Task_Washer_Data_Handle = NULL;

static void Task_Washer_Data(void* pvParameters)
{
    while (1)
    {
        printf("Task_Washer_Data Running! \r\n");

        //任务通知阻塞
        uint32_t Value = 0;
        xTaskNotifyWait(pdTRUE, pdFALSE, &Value, portMAX_DELAY);
        switch (Value)
        {
        case W_DAT_INIT:
            printf("W_DAT_INIT\r\n");
            g_pWDat->DataInit(&g_pWDat);
            break;
        case W_DAT_STORE:
            printf("W_DAT_STORE\r\n");
            g_pWDat->DataStore(g_pWDat);
            break;
        case W_DAT_RESTORE:
            printf("W_DAT_RESTORE\r\n");
            g_pWDat->DataRestore(&g_pWDat);
            break;
        default:
            break;
        }
    }
}

void Do_Create_Task_Washer_Data(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Washer_Data, "Task_Washer_Data", 128, NULL, 2, &Task_Washer_Data_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_Washer_Data] Success!\r\n");
    }
    else
    {
        printf("Create [Task_Washer_Data] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_Washer_Data_Handle(void)
{
    return &Task_Washer_Data_Handle;
}
