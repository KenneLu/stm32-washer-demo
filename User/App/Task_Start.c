#include "FreeRTOS.h"
#include "task.h"

#include "Drv_OLED.h"
#include "Drv_Key.h"
#include "Drv_Encoder.h"
#include "Drv_W25Q64.h"
#include "Drv_DHT11.h"
#include "Drv_TB6612.h"
#include "Drv_AD.h"
#include "Drv_ServoMotor.h"
#include "Drv_MPU6050.h"
#include "Drv_Buzzer.h"
#include "Drv_W25Q64.h"
#include "Drv_LED.h"

#include "Task_Manager.h"
#include "Washer_Data.h"
#include "Menu.h"


static TaskHandle_t Task_Start_Handle = NULL;

static void Task_Start(void* pvParameters)
{
    //显示、控制初始化
    OLED_Init();
    Drv_Key_Init();
    Dev_Encoder_Init();
    Do_Create_Task_KeyScan();

    //数据初始化
    Drv_W25Q64_Init();
    Drv_Washer_Data_Init();
    Do_Create_Task_Washer_Data();
    TASK_WASHER_DATA_RESTORE;

    //菜单初始化
    Menu_Init();
    Menu_Washer_Init();

    //洗衣机硬件初始化
    Drv_DHT11_Init();
    Drv_TB6612_Init();
    Drv_AD_Init();
    Drv_ServoMotor_Init();
    Drv_LED_Init();
    Drv_Buzzer_Init();
    Drv_MPU6050_Init();

    //向 Task_MainMenu 发送初始化完成的消息
    BaseType_t xReturn = pdFAIL;
    while (pdPASS != xReturn)
    {
        xReturn = xTaskNotifyGive(*Get_Task_MainMenu_Handle());
    }

    vTaskDelete(Task_Start_Handle);
}

void Do_Create_Task_Start(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(Task_Start, "Task_Start", 128, NULL, 1, &Task_Start_Handle);
    if (pdPASS == xReturn)
    {
        printf("Do_Create_Task_Start Success! \r\n");
    }
    else
    {
        printf("Error:Do_Create_Task_Start! \r\n");
    }
}

TaskHandle_t* Get_Task_Start_Handle(void)
{
    return &Task_Start_Handle;
}
