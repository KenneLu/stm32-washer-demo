
#ifndef __TASK_MANAGER_H__
#define __TASK_MANAGER_H__

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "Sys_Delay.h"


//Task_Start.c
extern void Do_Create_Task_Start(void);
extern TaskHandle_t* Get_Task_Start_Handle(void);


//Task_KeyScan.c
extern void Do_Create_Task_KeyScan(void);
extern TaskHandle_t* Get_Task_KeyScan_Handle(void);


//Task_MainMenu.c
extern void Do_Create_Task_MainMenu(void);
extern TaskHandle_t* Get_Task_MainMenu_Handle(void);


//Task_Washer_Data.c
extern void Do_Create_Task_Washer_Data(void);
extern TaskHandle_t* Get_Task_Washer_Data_Handle(void);
typedef enum
{
    W_DAT_INIT = 10,
    W_DAT_STORE = 11,
    W_DAT_RESTORE = 12,
} Washer_Data_Logic;
#define TASK_WASHER_DATA_INIT   xTaskNotify(*Get_Task_Washer_Data_Handle(), W_DAT_INIT, eSetValueWithOverwrite);
#define TASK_WASHER_DATA_STORE   xTaskNotify(*Get_Task_Washer_Data_Handle(), W_DAT_STORE, eSetValueWithOverwrite);
#define TASK_WASHER_DATA_RESTORE   xTaskNotify(*Get_Task_Washer_Data_Handle(), W_DAT_RESTORE, eSetValueWithOverwrite);


//Task_Washer_Stop.c
extern void Do_Create_Task_Washer_Stop(void);
extern TaskHandle_t* Get_Task_Washer_Stop_Handle(void);


//Task_Washer_Key.c
extern void Do_Create_Task_Washer_Key(void);
extern TaskHandle_t* Get_Task_Washer_Key_Handle(void);


//Task_Washer_Safety.c
extern void Do_Create_Task_Washer_Safety(void);
extern TaskHandle_t* Get_Task_Washer_Safety_Handle(void);


//Task_Washer_Pause.c
extern void Do_Create_Task_Washer_Pause(void);
extern TaskHandle_t* Get_Task_Washer_Pause_Handle(void);
typedef enum
{
    W_P_PAUSE = 10,
    W_P_CONTINUE = 11,
} Washer_Pause_Logic;
#define TASK_WASHER_PAUSE_PAUSE   xTaskNotify(*Get_Task_Washer_Pause_Handle(), W_P_PAUSE, eSetValueWithOverwrite);
#define TASK_WASHER_PAUSE_CONTINUE   xTaskNotify(*Get_Task_Washer_Pause_Handle(), W_P_CONTINUE, eSetValueWithOverwrite);


//Task_Washer_Error.c
extern void Do_Create_Task_Washer_Error(void);
extern TaskHandle_t* Get_Task_Washer_Error_Handle(void);
typedef enum
{
    W_E_OCCUR = 10,
    W_E_FIXED = 11,
} Washer_Error_Logic;
#define TASK_WASHER_ERROR_OCCUR   xTaskNotify(*Get_Task_Washer_Error_Handle(), W_E_OCCUR, eSetValueWithOverwrite);
#define TASK_WASHER_ERROR_FIXED   xTaskNotify(*Get_Task_Washer_Error_Handle(), W_E_FIXED, eSetValueWithOverwrite);


//Task_Washer_Run.c
extern void Do_Create_Task_Washer_Run(void);
extern TaskHandle_t* Get_Task_Washer_Run_Handle(void);


//Task_OLED.c
extern void Do_Create_Task_OLED(void);
extern TaskHandle_t* Get_Task_OLED_Handle(void);


#endif
