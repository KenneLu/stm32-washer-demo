
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

//Task_KeyScan.c
extern void Do_Create_Task_KeyScan(void);
extern TaskHandle_t* Get_Task_KeyScan_Handle(void);

//Task_MainMenu.c
extern void Do_Create_Task_MainMenu(void);
extern TaskHandle_t* Get_Task_MainMenu_Handle(void);

#endif

