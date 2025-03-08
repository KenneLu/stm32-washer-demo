#include "Task_Manager.h"
#include "Menu.h"


static TaskHandle_t Task_MainMenu_Handle = NULL;

static void Task_MainMenu(void* pvParameters)
{
    Menu_Washer_Power_On(); // 可能进入掉电恢复模式
    while (1)
    {
        Menu_Washer_Mode_Select(0);
    }
}

void Do_Create_Task_MainMenu(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_MainMenu, "Task_MainMenu", 1024, NULL, 1, &Task_MainMenu_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create_Task_MainMenu Success! \r\n");
    }
    else
    {
        printf("Error:Create_Task_MainMenu! \r\n");
    }
}

TaskHandle_t* Get_Task_MainMenu_Handle(void)
{
    return &Task_MainMenu_Handle;
}
