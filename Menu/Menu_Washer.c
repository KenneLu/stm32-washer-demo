#include "Menu.h"
#include "OLED.h"
#include "Encoder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	FAST,
	STANDERD,
	HARD,
	SWING,
	HEAT
} Wash_Mode;

typedef struct
{
	Wash_Mode Mode;			//洗衣模式
	uint8_t Wash_Count;		//漂洗次数（次）
	uint8_t Wash_Time;		//洗涤时间（分）
	uint8_t Swing_Dry_Time;	//脱水时间（分）
	uint8_t Water_Volume;	//水量(升)
	uint8_t Water_Temp;		//水温（℃）
	uint8_t Heat_Temp;		//烘干温度（℃）
	uint8_t Total_Time;		//洗衣时间总和（小时-分）
} Washer;

void Wash_Menu_Reset(Washer Washer);
int8_t Fast_Menu(void);
int8_t Standard_Wash_Menu(void);
int8_t Hard_Wash_Menu(void);
int8_t Swing_Dry_Menu(void);
int8_t Heat_Dry_Menu(void);
char* Uchar2Str(unsigned char Num);
void Insert_SubString(char* Str, char* SubStr, int Index);

int8_t Wash_Count_Setting(void);
int8_t Wash_Time_Setting(void);
int8_t Swing_Dry_Time_Setting(void);
int8_t Water_Volume_Setting(void);
int8_t Water_Temp_Setting(void);
int8_t Heat_Temp_Setting(void);

//Wash Mode
Wash_Mode CurMode;

//Washer
Washer Fast, Standard, Hard, Swing_Dry, Heat_Dry;

//List
uint8_t List_Wash_Count[] = { 2, 3, 4, 5 }; //次
uint8_t List_Wash_Time[] = { 5, 10, 15, 20 };	//分
uint8_t List_Swing_Dry_Time[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };	//分
uint8_t List_Water_Volume[] = { 3, 4, 5, 6, 7, 8, 9, 10 };	//升
uint8_t List_Water_Temp[] = { 0, 20, 30, 40, 50 };	//℃，0度就是常温
uint8_t List_Heat_Temp[] = { 40, 45, 50, 60, 70 };	//℃

//pointer
uint8_t* pL_Wash_Count = List_Wash_Count;
uint8_t* pL_Wash_Time = List_Wash_Time;
uint8_t* pL_Swing_Dry_Time = List_Swing_Dry_Time;
uint8_t* pL_Water_Volume = List_Water_Volume;
uint8_t* pL_Water_Temp = List_Water_Temp;
uint8_t* pL_Heat_Temp = List_Heat_Temp;

//Options
option_class Options_Wash_Count[sizeof(List_Wash_Count) + 1];
option_class Options_Wash_Time[sizeof(List_Wash_Time) + 1];
option_class Options_Swing_Dry_Time[sizeof(List_Swing_Dry_Time) + 1];
option_class Options_Water_Volume[sizeof(List_Water_Volume) + 1];
option_class Options_Water_Temp[sizeof(List_Water_Temp) + 1];
option_class Options_Heat_Temp[sizeof(List_Heat_Temp) + 1];
option_class Option_Main_Menu[] = {
		{ "快速洗", Fast_Menu},
		{ "标准洗", Standard_Wash_Menu},
		{ "强力洗", Hard_Wash_Menu},
		{ "脱水", Swing_Dry_Menu},
		{ "烘干", Heat_Dry_Menu},
		{ ".." }	//结尾标志,方便自动计算数量
};

option_class Options_Wash[] = {
		{ "启动", },
		{ "漂洗[次]", Wash_Count_Setting},
		{ "洗涤[分]", Wash_Time_Setting},
		{ "脱水[分]", Swing_Dry_Time_Setting},
		{ "水量[升]", Water_Volume_Setting},
		{ "水温[℃]", Water_Temp_Setting},
		{ ".." }
};
option_class Options_Wash_Cur[sizeof(Options_Wash)];

option_class Options_Swing[] = {
		{ "启动", },
		{ "脱水[分]", Swing_Dry_Time_Setting},
		{ ".." }
};
option_class Options_Swing_Cur[sizeof(Options_Swing)];

option_class Options_Heat[] = {
		{ "启动", },
		{ "烘干[℃]", Heat_Temp_Setting},
		{ ".." }
};
option_class Options_Heat_Cur[sizeof(Options_Heat)];

uint8_t Wash_Options_Inited = 0;
void Washer_Init(void)
{
	if (Wash_Options_Inited == 0)
	{
		//初始化选项
		for (int i = 0; i < sizeof(List_Wash_Count); i++)
		{
			strcpy(Options_Wash_Count[i].Name, Uchar2Str(List_Wash_Count[i]));
		}
		for (int i = 0; i < sizeof(List_Wash_Time); i++)
		{
			strcpy(Options_Wash_Time[i].Name, Uchar2Str(List_Wash_Time[i]));
		}
		for (int i = 0; i < sizeof(List_Swing_Dry_Time); i++)
		{
			strcpy(Options_Swing_Dry_Time[i].Name, Uchar2Str(List_Swing_Dry_Time[i]));
		}
		for (int i = 0; i < sizeof(List_Water_Volume); i++)
		{
			strcpy(Options_Water_Volume[i].Name, Uchar2Str(List_Water_Volume[i]));
		}
		for (int i = 0; i < sizeof(List_Water_Temp); i++)
		{
			strcpy(Options_Water_Temp[i].Name, Uchar2Str(List_Water_Temp[i]));
		}
		for (int i = 0; i < sizeof(List_Heat_Temp); i++)
		{
			strcpy(Options_Heat_Temp[i].Name, Uchar2Str(List_Heat_Temp[i]));
		}
		strcpy(Options_Wash_Count[sizeof(List_Wash_Count)].Name, ".."); // 结尾标志,方便自动计算数量
		strcpy(Options_Wash_Time[sizeof(List_Wash_Time)].Name, ".."); // 结尾标志,方便自动计算数量
		strcpy(Options_Swing_Dry_Time[sizeof(List_Swing_Dry_Time)].Name, ".."); // 结尾标志,方便自动计算数量
		strcpy(Options_Water_Volume[sizeof(List_Water_Volume)].Name, ".."); // 结尾标志,方便自动计算数量
		strcpy(Options_Water_Temp[sizeof(List_Water_Temp)].Name, ".."); // 结尾标志,方便自动计算数量
		strcpy(Options_Heat_Temp[sizeof(List_Heat_Temp)].Name, ".."); // 结尾标志,方便自动计算数量

		Wash_Options_Inited = 1;
	}

	//初始化Washer
	Fast.Mode = FAST;
	Fast.Wash_Count = *pL_Wash_Count;
	Fast.Wash_Time = *pL_Wash_Time;
	Fast.Swing_Dry_Time = *(pL_Swing_Dry_Time + 2);
	Fast.Water_Volume = *pL_Water_Volume;
	Fast.Water_Temp = *(pL_Water_Temp + 1);
	Fast.Total_Time = *pL_Wash_Count * (Fast.Wash_Time + Fast.Swing_Dry_Time + Fast.Swing_Dry_Time);

	Standard.Mode = STANDERD;
	Standard.Wash_Count = *(pL_Wash_Count + 1);
	Standard.Wash_Time = *(pL_Wash_Time + 1);
	Standard.Swing_Dry_Time = *(pL_Swing_Dry_Time + 4);
	Standard.Water_Volume = *(pL_Water_Volume + 2);
	Standard.Water_Temp = *(pL_Water_Temp + 1);
	Standard.Total_Time = *pL_Wash_Count * (Standard.Wash_Time + Standard.Swing_Dry_Time + Standard.Swing_Dry_Time);

	Hard.Mode = HARD;
	Hard.Wash_Count = *(pL_Wash_Count + 2);
	Hard.Wash_Time = *(pL_Wash_Time + 2);
	Hard.Swing_Dry_Time = *(pL_Swing_Dry_Time + 8);
	Hard.Water_Volume = *(pL_Water_Volume + 5);
	Hard.Water_Temp = *(pL_Water_Temp + 1);
	Hard.Total_Time = *pL_Wash_Count * (Hard.Wash_Time + Hard.Swing_Dry_Time + Hard.Swing_Dry_Time);

	Swing_Dry.Mode = SWING;
	Swing_Dry.Swing_Dry_Time = *(pL_Swing_Dry_Time + 2);
	Swing_Dry.Total_Time = Swing_Dry.Swing_Dry_Time;

	Heat_Dry.Mode = HEAT;
	Heat_Dry.Heat_Temp = *(pL_Heat_Temp + 1);
}

//洗衣机的主菜单
int8_t Washer_Menu(void)
{
	Washer_Init();
	return  Menu_Run(Option_Main_Menu);
}


//-------------------------------------------------------------------


void Wash_Menu_Reset(Washer Washer)
{
	if (Washer.Mode == SWING)
	{
		memcpy(Options_Swing_Cur, Options_Swing, sizeof(Options_Swing) * sizeof(option_class));
		Insert_SubString(Options_Swing_Cur[1].Name, Uchar2Str(Washer.Swing_Dry_Time), 7);
	}
	else if (Washer.Mode == HEAT)
	{
		memcpy(Options_Heat_Cur, Options_Heat, sizeof(Options_Heat) * sizeof(option_class));
		Insert_SubString(Options_Heat_Cur[1].Name, Uchar2Str(Washer.Heat_Temp), 7);
	}
	else
	{
		memcpy(Options_Wash_Cur, Options_Wash, sizeof(Options_Wash) * sizeof(option_class));
		Insert_SubString(Options_Wash_Cur[1].Name, Uchar2Str(Washer.Wash_Count), 7);
		Insert_SubString(Options_Wash_Cur[2].Name, Uchar2Str(Washer.Wash_Time), 7);
		Insert_SubString(Options_Wash_Cur[3].Name, Uchar2Str(Washer.Swing_Dry_Time), 7);
		Insert_SubString(Options_Wash_Cur[4].Name, Uchar2Str(Washer.Water_Volume), 7);
		Insert_SubString(Options_Wash_Cur[5].Name, Uchar2Str(Washer.Water_Temp), 7);
	}
}

int8_t Fast_Menu(void)
{
	CurMode = FAST;
	Wash_Menu_Reset(Fast);
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Standard_Wash_Menu(void)
{
	CurMode = STANDERD;
	Wash_Menu_Reset(Standard);
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Hard_Wash_Menu(void)
{
	CurMode = HARD;
	Wash_Menu_Reset(Hard);
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Swing_Dry_Menu(void)
{
	CurMode = SWING;
	Wash_Menu_Reset(Swing_Dry);
	return  Menu_Run(Options_Swing_Cur);
}

int8_t Heat_Dry_Menu(void)
{
	CurMode = HEAT;
	Wash_Menu_Reset(Heat_Dry);
	return  Menu_Run(Options_Heat_Cur);
}


//-------------------------------------------------------------------


int8_t Wash_Count_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Wash_Count);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Wash_Count[Catch_i];
		switch (CurMode)
		{
		case FAST:
			Fast.Wash_Count = New_Val;
			Wash_Menu_Reset(Fast);
			break;
		case STANDERD:
			Standard.Wash_Count = New_Val;
			Wash_Menu_Reset(Standard);
			break;
		case HARD:
			Hard.Wash_Count = New_Val;
			Wash_Menu_Reset(Hard);
			break;
		default:
			break;
		}
	}
	return Menu_Run(Options_Wash_Cur);
}

int8_t Wash_Time_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Wash_Time);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Wash_Time[Catch_i];
		switch (CurMode)
		{
		case FAST:
			Fast.Wash_Time = New_Val;
			Wash_Menu_Reset(Fast);
			break;
		case STANDERD:
			Standard.Wash_Time = New_Val;
			Wash_Menu_Reset(Standard);
			break;
		case HARD:
			Hard.Wash_Time = New_Val;
			Wash_Menu_Reset(Hard);
			break;
		default:
			break;
		}
	}
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Swing_Dry_Time_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Swing_Dry_Time);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Swing_Dry_Time[Catch_i];
		switch (CurMode)
		{
		case FAST:
			Fast.Swing_Dry_Time = New_Val;
			Wash_Menu_Reset(Fast);
			break;
		case STANDERD:
			Standard.Swing_Dry_Time = New_Val;
			Wash_Menu_Reset(Standard);
			break;
		case HARD:
			Hard.Swing_Dry_Time = New_Val;
			Wash_Menu_Reset(Hard);
			break;
		case SWING:
			Swing_Dry.Swing_Dry_Time = New_Val;
			Wash_Menu_Reset(Swing_Dry);
			break;
		default:
			break;
		}
	}
	if (CurMode == SWING) return  Menu_Run(Options_Swing_Cur);
	else return  Menu_Run(Options_Wash_Cur);
}

int8_t Water_Volume_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Water_Volume);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Water_Volume[Catch_i];
		switch (CurMode)
		{
		case FAST:
			Fast.Water_Volume = New_Val;
			Wash_Menu_Reset(Fast);
			break;
		case STANDERD:
			Standard.Water_Volume = New_Val;
			Wash_Menu_Reset(Standard);
			break;
		case HARD:
			Hard.Water_Volume = New_Val;
			Wash_Menu_Reset(Hard);
			break;
		default:
			break;
		}
	}
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Water_Temp_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Water_Temp);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Water_Temp[Catch_i];
		switch (CurMode)
		{
		case FAST:
			Fast.Water_Temp = New_Val;
			Wash_Menu_Reset(Fast);
			break;
		case STANDERD:
			Standard.Water_Temp = New_Val;
			Wash_Menu_Reset(Standard);
			break;
		case HARD:
			Hard.Water_Temp = New_Val;
			Wash_Menu_Reset(Hard);
			break;
		default:
			break;
		}
	}
	return  Menu_Run(Options_Wash_Cur);
}

int8_t Heat_Temp_Setting(void)
{
	int8_t Catch_i = Menu_Run(Options_Heat_Temp);
	if (Catch_i >= 0)
	{
		uint8_t New_Val = List_Heat_Temp[Catch_i];
		switch (CurMode)
		{
		case HEAT:
			Heat_Dry.Heat_Temp = New_Val;
			Wash_Menu_Reset(Heat_Dry);
			break;
		default:
			break;
		}
	}
	return  Menu_Run(Options_Heat_Cur);
}


//-------------------------------------------------------------------


char* Uchar2Str(unsigned char Num) {
	// 分配足够的内存来存储转换后的字符串和一个终止符
	char* Str = (char*)malloc(1 * sizeof(char)); // 10 digits for max unsigned char + 1 for null terminator
	if (Str == NULL) {
		return 0;
	}
	sprintf(Str, "%u", Num);
	return Str;
}

void Insert_SubString(char* Str, char* SubStr, int Index)
{
	// 计算新字符串的长度
	size_t SubStrLen = strlen(SubStr);
	size_t NewStrLen = strlen(Str) + SubStrLen;

	// 为新字符串分配内存
	char* NewStr = (char*)malloc(NewStrLen + 1 * sizeof(char)); // +1 for null terminator
	if (NewStr == NULL) return;

	strncpy(NewStr, Str, Index);	// 复制原始字符串的前半部分到新字符串中
	NewStr[Index] = '\0'; 			// 确保前半部分以null结尾

	strcpy(NewStr + Index, SubStr);
	strcpy(NewStr + Index + SubStrLen, Str + Index); // 复制原始字符串的后半部分到新字符串中
	NewStr[NewStrLen] = '\0'; // 确保新字符串以null结尾

	strcpy(Str, NewStr); // 复制新字符串到原字符串中
	Str[strlen(Str)] = '\0'; // 确保新字符串以null结尾

	free(SubStr);	// 释放子字符串
	free(NewStr);	// 释放新字符串
}

/**********************************************************/
