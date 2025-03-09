#include "stm32f10x.h"                  // Device header

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "Sys_Delay.h"
#include "Drv_OLED.h"
#include "Drv_Encoder.h"
#include "Drv_W25Q64.h"

#include "Washer_Data.h"
#include "Washer.h"
#include "Menu.h"
#include "Task_Manager.h"


char* Uchar2Str(int Num);
void DexNum_2_Str(char** Str, int DexNum);
void StrNum_2_Str(char** Str, char* Val, uint8_t Len);
void Insert_SubString(char** Str, char* SubStr, int Index);
uint8_t GetIndex(uint8_t* List, uint8_t Val, uint8_t ListSize);
int8_t Menu_Washer_Start_Washer(void* Param);


//List
uint8_t List_Set_Wash_Cnt[] = { 1, 2, 3, 4, 5 }; //次
uint8_t List_Set_Wash_Time[] = { 3, 5, 10, 15, 20 };	//分
uint8_t List_Set_Spin_Dry_Time[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };	//分
uint8_t List_Set_Water_Volume[] = { 3, 4, 5, 6, 7, 8, 9, 10 };	//升
uint8_t List_Set_Water_Temp[] = { 0, 20, 30, 40, 50 };	//℃，0度就是常温
uint8_t List_Set_Heat_Temp[] = { 40, 45, 50, 60, 70 };	//℃

//Options
Option_Class Opt_Set_Wash_Cnt[sizeof(List_Set_Wash_Cnt) + 1];
Option_Class Opt_Set_Wash_Time[sizeof(List_Set_Wash_Time) + 1];
Option_Class Opt_Set_Spin_Dry_Time[sizeof(List_Set_Spin_Dry_Time) + 1];
Option_Class Opt_Set_Water_Volume[sizeof(List_Set_Water_Volume) + 1];
Option_Class Opt_Set_Water_Temp[sizeof(List_Set_Water_Temp) + 1];
Option_Class Opt_Set_Heat_Temp[sizeof(List_Set_Heat_Temp) + 1];


static uint8_t g_Washer_Mode[M_NUM] = { 0, 1, 2, 3, 4 };

Option_Class Opt_Mode_Select[] = {
	{ "快速洗", Menu_Washer_Mode_Detail, &g_Washer_Mode[M_FAST_WASH]},
	{ "标准洗", Menu_Washer_Mode_Detail, &g_Washer_Mode[M_STANDARD_WASH]},
	{ "强力洗", Menu_Washer_Mode_Detail, &g_Washer_Mode[M_HARD_WASH]},
	{ "甩干", Menu_Washer_Mode_Detail, &g_Washer_Mode[M_SPIN_DRY]},
	{ "烘干", Menu_Washer_Mode_Detail, &g_Washer_Mode[M_HEAT_DRY]},
	{ ".." }	//结尾标志,方便自动计算数量
};

Option_Class Opt_Detail_Wash[] = {
		{ "开始运行", Menu_Washer_Start_Washer, 0},
		{ "设置次数[次]", Menu_Washer_Mode_Setting, &Opt_Set_Wash_Cnt},
		{ "设置时长[分]", Menu_Washer_Mode_Setting, &Opt_Set_Wash_Time},
		{ "设置脱水[分]", Menu_Washer_Mode_Setting, &Opt_Set_Spin_Dry_Time},
		{ "设置水量[升]", Menu_Washer_Mode_Setting, &Opt_Set_Water_Volume},
		{ "设置水温[℃]", Menu_Washer_Mode_Setting, &Opt_Set_Water_Temp},
		{ ".." }
};
Option_Class Opt_Detail_Wash_Cur[sizeof(Opt_Detail_Wash)];

Option_Class Opt_Detail_Spin[] = {
		{ "开始运行", Menu_Washer_Start_Washer, 0},
		{ "脱水时长[分]", Menu_Washer_Mode_Setting, &Opt_Set_Spin_Dry_Time},
		{ ".." }
};
Option_Class Opt_Detail_Spin_Cur[sizeof(Opt_Detail_Spin)];

Option_Class Opt_Detail_Heat[] = {
		{ "开始运行", Menu_Washer_Start_Washer, 0},
		{ "烘干温度[℃]", Menu_Washer_Mode_Setting, &Opt_Set_Heat_Temp},
		{ ".." }
};
Option_Class Opt_Detail_Heat_Cur[sizeof(Opt_Detail_Heat)];


void Menu_Washer_Task_Init(void)
{
	if (*Get_Task_MainMenu_Handle() == NULL)
		Do_Create_Task_MainMenu();
	else
		vTaskResume(*Get_Task_MainMenu_Handle());
}

void Menu_Washer_Task_DeInit(void)
{
	if (*Get_Task_MainMenu_Handle())
		vTaskSuspend(*Get_Task_MainMenu_Handle());
}

void Menu_Washer_Init(void)
{
	Menu_Washer_Task_Init();
}

void Menu_Washer_DeInit(void)
{
	Menu_Washer_Task_DeInit();
}

int8_t Menu_Washer_Start_Washer(void* Param)
{
	Washer_Init();
	Menu_Washer_DeInit();

	return -1;
}

void Menu_Washer_Power_On(void)
{
	OLED_Clear_Easy();
	OLED_ShowString_Easy(1, 1, "Power On");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power On .");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power On ..");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power On ...");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power On ....");
	Delay_ms(200);

	//等待 Task_Start 完成初始化
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	// //意外掉电重启
	// if (g_pWDat->Shutdown_Type == ACCIDENT_SHUTDOWN)
	// {
	// 	OLED_ShowString_Easy(1, 1, "Restore Last");
	// 	Delay_ms(200);
	// 	OLED_ShowString_Easy(1, 1, "Restore Last .");
	// 	Delay_ms(200);
	// 	OLED_ShowString_Easy(1, 1, "Restore Last ..");
	// 	Delay_ms(200);
	// 	OLED_ShowString_Easy(1, 1, "Restore Last ...");
	// 	Delay_ms(200);
	// 	Menu_Washer_Start_Washer(0);
	// }
	// else
	// {
	// 	//正常启动
	// 	TASK_WASHER_DATA_INIT;
	// 	TASK_WASHER_DATA_STORE;
	// 	OLED_Clear_Easy();
	// }

	//正常启动
	TASK_WASHER_DATA_INIT;
	TASK_WASHER_DATA_STORE;
	OLED_Clear_Easy();
}

void Menu_Washer_Power_Off(void)
{
	// 客户主动关机
	OLED_Clear_Easy();
	OLED_ShowString_Easy(1, 1, "Power Off");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power Off .");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power Off ..");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power Off ...");
	Delay_ms(200);
	OLED_ShowString_Easy(1, 1, "Power Off ....");
	Delay_ms(200);
	OLED_Clear_Easy();

	// 开始待机
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);    //使能PWR外设时钟
	PWR_WakeUpPinCmd(ENABLE);  //使能唤醒管脚功能,在WkUp的上升沿进行
	PWR_EnterSTANDBYMode();
}

//按模式初始化Washer
void Wahser_Data_Set(WASHER_MODE Mode)
{
	// g_pWDat->DataInit(&g_pWDat);
	TASK_WASHER_DATA_INIT;

	if (Mode == M_FAST_WASH)
	{
		g_pWDat->Mode = M_FAST_WASH;
		g_pWDat->Wash_Cnt = List_Set_Wash_Cnt[1];
		g_pWDat->Wash_Time = List_Set_Wash_Time[1];
		g_pWDat->Spin_Dry_Time = List_Set_Spin_Dry_Time[2];
		g_pWDat->Water_Volume = List_Set_Water_Volume[0];
		g_pWDat->Water_Temp = List_Set_Water_Temp[0];
		g_pWDat->Total_Time =
			g_pWDat->Wash_Cnt * (
				g_pWDat->Wash_Time +
				g_pWDat->Spin_Dry_Time +
				g_pWDat->Spin_Dry_Time
				);
	}

	if (Mode == M_STANDARD_WASH)
	{
		g_pWDat->Mode = M_STANDARD_WASH;
		g_pWDat->Wash_Cnt = List_Set_Wash_Cnt[2];
		g_pWDat->Wash_Time = List_Set_Wash_Time[2];
		g_pWDat->Spin_Dry_Time = List_Set_Spin_Dry_Time[4];
		g_pWDat->Water_Volume = List_Set_Water_Volume[2];
		g_pWDat->Water_Temp = List_Set_Water_Temp[0];
		g_pWDat->Total_Time =
			g_pWDat->Wash_Cnt * (
				g_pWDat->Wash_Time +
				g_pWDat->Spin_Dry_Time +
				g_pWDat->Spin_Dry_Time
				);
	}

	if (Mode == M_HARD_WASH)
	{
		g_pWDat->Mode = M_HARD_WASH;
		g_pWDat->Wash_Cnt = List_Set_Wash_Cnt[3];
		g_pWDat->Wash_Time = List_Set_Wash_Time[3];
		g_pWDat->Spin_Dry_Time = List_Set_Spin_Dry_Time[8];
		g_pWDat->Water_Volume = List_Set_Water_Volume[5];
		g_pWDat->Water_Temp = List_Set_Water_Temp[0];
		g_pWDat->Total_Time =
			g_pWDat->Wash_Cnt * (
				g_pWDat->Wash_Time +
				g_pWDat->Spin_Dry_Time +
				g_pWDat->Spin_Dry_Time
				);
	}

	if (Mode == M_SPIN_DRY)
	{
		g_pWDat->Mode = M_SPIN_DRY;
		g_pWDat->Wash_Cnt = 0;
		g_pWDat->Wash_Time = 0;
		g_pWDat->Spin_Dry_Time = List_Set_Spin_Dry_Time[2];
		g_pWDat->Water_Volume = 0;
		g_pWDat->Water_Temp = 0;
		g_pWDat->Total_Time = g_pWDat->Spin_Dry_Time;
	}

	if (Mode == M_HEAT_DRY)
	{
		g_pWDat->Mode = M_HEAT_DRY;
		g_pWDat->Wash_Cnt = 0;
		g_pWDat->Wash_Time = 0;
		g_pWDat->Spin_Dry_Time = 0;
		g_pWDat->Water_Volume = 0;
		g_pWDat->Heat_Temp = List_Set_Heat_Temp[1];
		g_pWDat->Total_Time = 0;
	}
}

void Menu_Washer_Param_Init(void)
{
	//初始化设置
	int i;
	for (i = 0; i < sizeof(List_Set_Wash_Cnt); i++)
	{
		DexNum_2_Str(&Opt_Set_Wash_Cnt[i].Name, List_Set_Wash_Cnt[i]);
	}
	StrNum_2_Str(&Opt_Set_Wash_Cnt[i].Name, "..", 2); // 结尾标志,方便自动计算数量

	for (i = 0; i < sizeof(List_Set_Wash_Time); i++)
	{
		DexNum_2_Str(&Opt_Set_Wash_Time[i].Name, List_Set_Wash_Time[i]);
	}
	StrNum_2_Str(&Opt_Set_Wash_Time[i].Name, "..", 2); // 结尾标志,方便自动计算数量

	for (i = 0; i < sizeof(List_Set_Spin_Dry_Time); i++)
	{
		DexNum_2_Str(&Opt_Set_Spin_Dry_Time[i].Name, List_Set_Spin_Dry_Time[i]);
	}
	StrNum_2_Str(&Opt_Set_Spin_Dry_Time[i].Name, "..", 2); // 结尾标志,方便自动计算数量

	for (i = 0; i < sizeof(List_Set_Water_Volume); i++)
	{
		DexNum_2_Str(&Opt_Set_Water_Volume[i].Name, List_Set_Water_Volume[i]);
	}
	StrNum_2_Str(&Opt_Set_Water_Volume[i].Name, "..", 2); // 结尾标志,方便自动计算数量

	for (i = 0; i < sizeof(List_Set_Water_Temp); i++)
	{
		DexNum_2_Str(&Opt_Set_Water_Temp[i].Name, List_Set_Water_Temp[i]);
	}
	StrNum_2_Str(&Opt_Set_Water_Temp[i].Name, "..", 2); // 结尾标志,方便自动计算数量

	for (i = 0; i < sizeof(List_Set_Heat_Temp); i++)
	{
		DexNum_2_Str(&Opt_Set_Heat_Temp[i].Name, List_Set_Heat_Temp[i]);
	}
	StrNum_2_Str(&Opt_Set_Heat_Temp[i].Name, "..", 2); // 结尾标志,方便自动计算数量


	//初始化详细
	for (i = 0; i < sizeof(Opt_Detail_Wash) / sizeof(Option_Class); i++)
	{
		Opt_Detail_Wash_Cur[i].Name = malloc(strlen(Opt_Detail_Wash[i].Name) * sizeof(char));
		if (Opt_Detail_Wash_Cur[i].Name == NULL) while (1) OLED_ShowString_Easy(1, 1, "NULLLLL1");
		strcpy(Opt_Detail_Wash_Cur[i].Name, Opt_Detail_Wash[i].Name);
		Opt_Detail_Wash_Cur[i].Func = Opt_Detail_Wash[i].Func;
		Opt_Detail_Wash_Cur[i].pFuncParam = Opt_Detail_Wash[i].pFuncParam;
	}

	for (i = 0; i < sizeof(Opt_Detail_Spin) / sizeof(Option_Class); i++)
	{
		Opt_Detail_Spin_Cur[i].Name = malloc(strlen(Opt_Detail_Spin[i].Name) * sizeof(char) + 1);
		if (Opt_Detail_Spin_Cur[i].Name == NULL) while (1) OLED_ShowString_Easy(1, 1, "NULLLLL2");
		strcpy(Opt_Detail_Spin_Cur[i].Name, Opt_Detail_Spin[i].Name);
		Opt_Detail_Spin_Cur[i].Func = Opt_Detail_Spin[i].Func;
		Opt_Detail_Spin_Cur[i].pFuncParam = Opt_Detail_Spin[i].pFuncParam;
	}

	for (i = 0; i < sizeof(Opt_Detail_Heat) / sizeof(Option_Class); i++)
	{
		Opt_Detail_Heat_Cur[i].Name = malloc(strlen(Opt_Detail_Heat[i].Name) * sizeof(char) + 1);
		if (Opt_Detail_Heat_Cur[i].Name == NULL) while (1) OLED_ShowString_Easy(1, 1, "NULLLLL3");
		strcpy(Opt_Detail_Heat_Cur[i].Name, Opt_Detail_Heat[i].Name);
		Opt_Detail_Heat_Cur[i].Func = Opt_Detail_Heat[i].Func;
		Opt_Detail_Heat_Cur[i].pFuncParam = Opt_Detail_Heat[i].pFuncParam;
	}
}

//洗衣机的主菜单
int8_t Menu_Washer_Mode_Select(void* Param)
{
	return  Menu_Run(Opt_Mode_Select, 0);
}


//-------------------------------------------------------------------



void Menu_Washer_Disaply_Refresh(void)
{
	if (g_pWDat->Mode == M_SPIN_DRY)
	{
		for (int i = 0; i < sizeof(Opt_Detail_Spin) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Spin_Cur[i].Name, Opt_Detail_Spin[i].Name);
		}
		Insert_SubString(&Opt_Detail_Spin_Cur[1].Name, Uchar2Str(g_pWDat->Spin_Dry_Time), 13); //13 = 中文个数*3 + 英文个数*1
	}
	else if (g_pWDat->Mode == M_HEAT_DRY)
	{
		for (int i = 0; i < sizeof(Opt_Detail_Heat) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Heat_Cur[i].Name, Opt_Detail_Heat[i].Name);
		}
		Insert_SubString(&Opt_Detail_Heat_Cur[1].Name, Uchar2Str(g_pWDat->Heat_Temp), 13);
	}
	else
	{
		for (int i = 0; i < sizeof(Opt_Detail_Wash) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Wash_Cur[i].Name, Opt_Detail_Wash[i].Name);
		}
		Insert_SubString(&Opt_Detail_Wash_Cur[1].Name, Uchar2Str(g_pWDat->Wash_Cnt), 13);
		Insert_SubString(&Opt_Detail_Wash_Cur[2].Name, Uchar2Str(g_pWDat->Wash_Time), 13);
		Insert_SubString(&Opt_Detail_Wash_Cur[3].Name, Uchar2Str(g_pWDat->Spin_Dry_Time), 13);
		Insert_SubString(&Opt_Detail_Wash_Cur[4].Name, Uchar2Str(g_pWDat->Water_Volume), 13);
		Insert_SubString(&Opt_Detail_Wash_Cur[5].Name, Uchar2Str(g_pWDat->Water_Temp), 13);
	}
}

int8_t Menu_Washer_Mode_Detail(void* Param)
{
	uint8_t* pMode = (uint8_t*)Param;
	if (pMode == 0)
		return -1;

	Wahser_Data_Set((WASHER_MODE)*pMode);
	Menu_Washer_Disaply_Refresh();


	int8_t Child_Catch_i = 0;
	while (1)
	{
		if (g_pWDat->Mode == M_FAST_WASH)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (g_pWDat->Mode == M_STANDARD_WASH)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (g_pWDat->Mode == M_HARD_WASH)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (g_pWDat->Mode == M_SPIN_DRY)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Spin_Cur, Child_Catch_i);
		}
		else if (g_pWDat->Mode == M_HEAT_DRY)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Heat_Cur, Child_Catch_i);
		}

		if (Child_Catch_i == -1) { return -1; }
	}
}

int8_t Menu_Washer_Mode_Setting(void* Param)
{
	Option_Class* pOption = (Option_Class*)Param;
	if (!pOption) return -1;

	int8_t Child_Catch_i;
	uint8_t New_Val;
	if (pOption == Opt_Set_Wash_Cnt)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Wash_Cnt, GetIndex(List_Set_Wash_Cnt, g_pWDat->Wash_Cnt, sizeof(List_Set_Wash_Cnt)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Wash_Cnt[Child_Catch_i];
		g_pWDat->Wash_Cnt = New_Val;
		Menu_Washer_Disaply_Refresh();
	}
	else if (pOption == Opt_Set_Wash_Time)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Wash_Time, GetIndex(List_Set_Wash_Time, g_pWDat->Wash_Time, sizeof(List_Set_Wash_Time)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Wash_Time[Child_Catch_i];
		g_pWDat->Wash_Time = New_Val;
		Menu_Washer_Disaply_Refresh();
	}
	else if (pOption == Opt_Set_Spin_Dry_Time)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Spin_Dry_Time, GetIndex(List_Set_Spin_Dry_Time, g_pWDat->Spin_Dry_Time, sizeof(List_Set_Spin_Dry_Time)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Spin_Dry_Time[Child_Catch_i];
		g_pWDat->Spin_Dry_Time = New_Val;
		Menu_Washer_Disaply_Refresh();
	}
	else if (pOption == Opt_Set_Water_Volume)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Water_Volume, GetIndex(List_Set_Water_Volume, g_pWDat->Water_Volume, sizeof(List_Set_Water_Volume)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Water_Volume[Child_Catch_i];
		g_pWDat->Water_Volume = New_Val;
		Menu_Washer_Disaply_Refresh();
	}
	else if (pOption == Opt_Set_Water_Temp)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Water_Temp, GetIndex(List_Set_Water_Temp, g_pWDat->Water_Temp, sizeof(List_Set_Water_Temp)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Water_Temp[Child_Catch_i];
		g_pWDat->Water_Temp = New_Val;
		Menu_Washer_Disaply_Refresh();
	}
	else if (pOption == Opt_Set_Heat_Temp)
	{
		Child_Catch_i = Menu_Run(Opt_Set_Heat_Temp, GetIndex(List_Set_Heat_Temp, g_pWDat->Heat_Temp, sizeof(List_Set_Heat_Temp)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Set_Heat_Temp[Child_Catch_i];
		g_pWDat->Heat_Temp = New_Val;
		Menu_Washer_Disaply_Refresh();
	}

	return  Child_Catch_i;
}


//-------------------------------------------------------------------

void Insert_SubString(char** Str, char* SubStr, int Index)
{
	// 计算新字符串的长度
	size_t SubStrLen = strlen(SubStr);
	size_t NewStrLen = strlen(*Str) + SubStrLen;

	// 为新字符串分配内存
	char* NewStr = (char*)malloc(NewStrLen + 1 * sizeof(char)); // +1 for null terminator
	if (NewStr == NULL) while (1) OLED_ShowString_Easy(1, 1, "NULLLLLL0");

	strncpy(NewStr, *Str, Index);	// 复制原始字符串的前半部分到新字符串中
	NewStr[Index] = '\0'; 			// 确保前半部分以null结尾

	strcpy(NewStr + Index, SubStr);
	strcpy(NewStr + Index + SubStrLen, *Str + Index); // 复制原始字符串的后半部分到新字符串中
	NewStr[NewStrLen] = '\0'; // 确保新字符串以null结尾

	if (*SubStr) free(SubStr);	// 释放子字符串
	if (*Str) free(*Str); // 释放原字符串
	*Str = NewStr; // 指向新字符串
}

uint8_t GetIndex(uint8_t* List, uint8_t Val, uint8_t ListSize)
{
	for (int i = 0; i < ListSize; i++)
	{
		if (List[i] == Val) return i;
	}
	return 0;
}

char* Uchar2Str(int Num) {
	// 分配足够的内存来存储转换后的字符串和一个终止符
	char* Str = (char*)malloc(1 * sizeof(char)); // 10 digits for max unsigned char + 1 for null terminator
	if (Str == NULL) {
		return 0;
	}
	sprintf(Str, "%u", Num);
	return Str;
}

int CountDigits(int Num) {
	if (Num == 0) return 1;
	int count = 0;
	Num = abs(Num); // 处理负数
	while (Num != 0) {
		Num /= 10;
		count++;
	}
	return count;
}

void DexNum_2_Str(char** Str, int DexNum)
{
	uint8_t Len = CountDigits(DexNum);
	if (*Str) free(*Str);
	*Str = (char*)malloc((Len + 1) * sizeof(char)); //十位数
	char* Num_Str = Uchar2Str(DexNum);
	strcpy(*Str, Num_Str);
	(*Str)[Len] = '\0';
	free(Num_Str);
}

void StrNum_2_Str(char** Str, char* Val, uint8_t Len)
{
	if (*Str) free(*Str);
	*Str = (char*)malloc(Len * sizeof(char) + 1);
	strncpy(*Str, Val, Len);
	(*Str)[Len] = '\0';
}

/**********************************************************/
