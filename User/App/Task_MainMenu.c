#include "Task_Manager.h"
#include "Menu.h"


static TaskHandle_t Task_MainMenu_Handle = NULL;

static void Task_MainMenu(void* pvParameters)
{
    Menu_Washer_Power_On(); // 可能进入掉电恢复模式
    Menu_Washer_Param_Init(); // 初始化菜单参数
    Menu_Init(); // 初始化菜单
    while (1)
    {
        printf("Welcome to Washer Demo!\r\n");

        Menu_Washer_Mode_Select(0);
    }
}

void Do_Create_Task_MainMenu(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_MainMenu, "Task_MainMenu", 256, NULL, 0, &Task_MainMenu_Handle);
    if (pdPASS == xReturn)
    {
        printf("Create [Task_MainMenu] Success!\r\n");
    }
    else
    {
        printf("Create [Task_MainMenu] Failed!\r\n");
    }
}

TaskHandle_t* Get_Task_MainMenu_Handle(void)
{
    return &Task_MainMenu_Handle;
}
