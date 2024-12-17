#include "Menu.h"
#include "OLED.h"
#include "Encoder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Washer.h"


char* Uchar2Str(int Num);
void DexNum_2_Str(char** Str, int DexNum);
void StrNum_2_Str(char** Str, char* Val, uint8_t Len);
void Insert_SubString(char** Str, char* SubStr, int Index);
uint8_t GetIndex(uint8_t* List, uint8_t Val, uint8_t ListSize);

//Washer
Washer Fast, Standard, Hard, Spin_Dry, Heat_Dry, Washer_Cur;

//List
uint8_t List_Setting_Wash_Cnt[] = { 2, 3, 4, 5 }; //次
uint8_t List_Setting_Wash_Time[] = { 5, 10, 15, 20 };	//分
uint8_t List_Setting_Spin_Dry_Time[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };	//分
uint8_t List_Setting_Water_Volume[] = { 3, 4, 5, 6, 7, 8, 9, 10 };	//升
uint8_t List_Setting_Water_Temp[] = { 0, 20, 30, 40, 50 };	//℃，0度就是常温
uint8_t List_Setting_Heat_Temp[] = { 40, 45, 50, 60, 70 };	//℃

//Options
Option_Class Opt_Setting_Wash_Cnt[sizeof(List_Setting_Wash_Cnt) + 1];
Option_Class Opt_Setting_Wash_Time[sizeof(List_Setting_Wash_Time) + 1];
Option_Class Opt_Setting_Spin_Dry_Time[sizeof(List_Setting_Spin_Dry_Time) + 1];
Option_Class Opt_Setting_Water_Volume[sizeof(List_Setting_Water_Volume) + 1];
Option_Class Opt_Setting_Water_Temp[sizeof(List_Setting_Water_Temp) + 1];
Option_Class Opt_Setting_Heat_Temp[sizeof(List_Setting_Heat_Temp) + 1];

Option_Class Opt_Mode_Select[] = {
		{ "快速洗", Menu_Washer_Mode_Detail, &Fast},
		{ "标准洗", Menu_Washer_Mode_Detail, &Standard},
		{ "强力洗", Menu_Washer_Mode_Detail, &Hard},
		{ "脱水", Menu_Washer_Mode_Detail, &Spin_Dry},
		{ "烘干", Menu_Washer_Mode_Detail, &Heat_Dry},
		{ ".." }	//结尾标志,方便自动计算数量
};

Option_Class Opt_Detail_Wash[] = {
		{ "启动", Washer_Run, &Washer_Cur},
		{ "漂洗[次]", Menu_Washer_Mode_Setting, &Opt_Setting_Wash_Cnt},
		{ "洗涤[分]", Menu_Washer_Mode_Setting, &Opt_Setting_Wash_Time},
		{ "脱水[分]", Menu_Washer_Mode_Setting, &Opt_Setting_Spin_Dry_Time},
		{ "水量[升]", Menu_Washer_Mode_Setting, &Opt_Setting_Water_Volume},
		{ "水温[℃]", Menu_Washer_Mode_Setting, &Opt_Setting_Water_Temp},
		{ ".." }
};
Option_Class Opt_Detail_Wash_Cur[sizeof(Opt_Detail_Wash)];

Option_Class Opt_Detail_Spin[] = {
		{ "启动", Washer_Run, &Washer_Cur},
		{ "脱水[分]", Menu_Washer_Mode_Setting, &Opt_Setting_Spin_Dry_Time},
		{ ".." }
};
Option_Class Opt_Detail_Spin_Cur[sizeof(Opt_Detail_Spin)];

Option_Class Opt_Detail_Heat[] = {
		{ "启动", Washer_Run, &Washer_Cur},
		{ "烘干[℃]", Menu_Washer_Mode_Setting, &Opt_Setting_Heat_Temp},
		{ ".." }
};
Option_Class Opt_Detail_Heat_Cur[sizeof(Opt_Detail_Heat)];

uint8_t Wash_Opt_Inited = 0;

void Menu_Washer_Init(void)
{
	if (Wash_Opt_Inited == 0)
	{
		//初始化选项
		int i;
		for (i = 0; i < sizeof(List_Setting_Wash_Cnt); i++)
		{
			DexNum_2_Str(&Opt_Setting_Wash_Cnt[i].Name, List_Setting_Wash_Cnt[i]);
		}
		StrNum_2_Str(&Opt_Setting_Wash_Cnt[i].Name, "..", 2); // 结尾标志,方便自动计算数量

		for (i = 0; i < sizeof(List_Setting_Wash_Time); i++)
		{
			DexNum_2_Str(&Opt_Setting_Wash_Time[i].Name, List_Setting_Wash_Time[i]);
		}
		StrNum_2_Str(&Opt_Setting_Wash_Time[i].Name, "..", 2); // 结尾标志,方便自动计算数量

		for (i = 0; i < sizeof(List_Setting_Spin_Dry_Time); i++)
		{
			DexNum_2_Str(&Opt_Setting_Spin_Dry_Time[i].Name, List_Setting_Spin_Dry_Time[i]);
		}
		StrNum_2_Str(&Opt_Setting_Spin_Dry_Time[i].Name, "..", 2); // 结尾标志,方便自动计算数量

		for (i = 0; i < sizeof(List_Setting_Water_Volume); i++)
		{
			DexNum_2_Str(&Opt_Setting_Water_Volume[i].Name, List_Setting_Water_Volume[i]);
		}
		StrNum_2_Str(&Opt_Setting_Water_Volume[i].Name, "..", 2); // 结尾标志,方便自动计算数量

		for (i = 0; i < sizeof(List_Setting_Water_Temp); i++)
		{
			DexNum_2_Str(&Opt_Setting_Water_Temp[i].Name, List_Setting_Water_Temp[i]);
		}
		StrNum_2_Str(&Opt_Setting_Water_Temp[i].Name, "..", 2); // 结尾标志,方便自动计算数量

		for (i = 0; i < sizeof(List_Setting_Heat_Temp); i++)
		{
			DexNum_2_Str(&Opt_Setting_Heat_Temp[i].Name, List_Setting_Heat_Temp[i]);
		}
		StrNum_2_Str(&Opt_Setting_Heat_Temp[i].Name, "..", 2); // 结尾标志,方便自动计算数量



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
		Wash_Opt_Inited = 1;
	}

	//初始化Washer
	Fast.Mode = FAST;
	Fast.Wash_Cnt = List_Setting_Wash_Cnt[0];
	Fast.Wash_Time = List_Setting_Wash_Time[0];
	Fast.Spin_Dry_Time = List_Setting_Spin_Dry_Time[2];
	Fast.Water_Volume = List_Setting_Water_Volume[0];
	Fast.Water_Temp = List_Setting_Water_Temp[1];
	Fast.Total_Time = List_Setting_Wash_Cnt[0] * (Fast.Wash_Time + Fast.Spin_Dry_Time + Fast.Spin_Dry_Time);

	Standard.Mode = STANDERD;
	Standard.Wash_Cnt = List_Setting_Wash_Cnt[1];
	Standard.Wash_Time = List_Setting_Wash_Time[1];
	Standard.Spin_Dry_Time = List_Setting_Spin_Dry_Time[4];
	Standard.Water_Volume = List_Setting_Water_Volume[2];
	Standard.Water_Temp = List_Setting_Water_Temp[1];
	Standard.Total_Time = List_Setting_Wash_Cnt[0] * (Standard.Wash_Time + Standard.Spin_Dry_Time + Standard.Spin_Dry_Time);

	Hard.Mode = HARD;
	Hard.Wash_Cnt = List_Setting_Wash_Cnt[2];
	Hard.Wash_Time = List_Setting_Wash_Time[2];
	Hard.Spin_Dry_Time = List_Setting_Spin_Dry_Time[8];
	Hard.Water_Volume = List_Setting_Water_Volume[5];
	Hard.Water_Temp = List_Setting_Water_Temp[1];
	Hard.Total_Time = List_Setting_Wash_Cnt[0] * (Hard.Wash_Time + Hard.Spin_Dry_Time + Hard.Spin_Dry_Time);

	Spin_Dry.Mode = SWING;
	Spin_Dry.Spin_Dry_Time = List_Setting_Spin_Dry_Time[2];
	Spin_Dry.Total_Time = Spin_Dry.Spin_Dry_Time;

	Heat_Dry.Mode = HEAT;
	Heat_Dry.Heat_Temp = List_Setting_Heat_Temp[1];
}

void Menu_Washer(void)
{
	Menu_Washer_Mode_Select(0);
}

//洗衣机的主菜单
int8_t Menu_Washer_Mode_Select(void* Param)
{
	Menu_Washer_Init();
	return  Menu_Run(Opt_Mode_Select, 0);
}


//-------------------------------------------------------------------


void Menu_Washer_Disaply_Refresh(Washer Washer)
{
	if (Washer.Mode == SWING)
	{
		for (int i = 0; i < sizeof(Opt_Detail_Spin) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Spin_Cur[i].Name, Opt_Detail_Spin[i].Name);
		}
		Insert_SubString(&Opt_Detail_Spin_Cur[1].Name, Uchar2Str(Washer.Spin_Dry_Time), 7);
	}
	else if (Washer.Mode == HEAT)
	{
		for (int i = 0; i < sizeof(Opt_Detail_Heat) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Heat_Cur[i].Name, Opt_Detail_Heat[i].Name);
		}
		Insert_SubString(&Opt_Detail_Heat_Cur[1].Name, Uchar2Str(Washer.Heat_Temp), 7);
	}
	else
	{
		for (int i = 0; i < sizeof(Opt_Detail_Wash) / sizeof(Option_Class); i++)
		{
			strcpy(Opt_Detail_Wash_Cur[i].Name, Opt_Detail_Wash[i].Name);
		}
		Insert_SubString(&Opt_Detail_Wash_Cur[1].Name, Uchar2Str(Washer.Wash_Cnt), 7);
		Insert_SubString(&Opt_Detail_Wash_Cur[2].Name, Uchar2Str(Washer.Wash_Time), 7);
		Insert_SubString(&Opt_Detail_Wash_Cur[3].Name, Uchar2Str(Washer.Spin_Dry_Time), 7);
		Insert_SubString(&Opt_Detail_Wash_Cur[4].Name, Uchar2Str(Washer.Water_Volume), 7);
		Insert_SubString(&Opt_Detail_Wash_Cur[5].Name, Uchar2Str(Washer.Water_Temp), 7);

		Washer_Cur = Washer;
	}
}

int8_t Menu_Washer_Mode_Detail(void* Param)
{
	Washer* pWasher = (Washer*)Param;
	if (!pWasher) return -1;
	Washer_Cur = *pWasher;
	Menu_Washer_Disaply_Refresh(*pWasher);
	int8_t Child_Catch_i = 0;
	while (1)
	{
		if (pWasher->Mode == FAST)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (pWasher->Mode == STANDERD)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (pWasher->Mode == HARD)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Wash_Cur, Child_Catch_i);
		}
		else if (pWasher->Mode == SWING)
		{
			Child_Catch_i = Menu_Run(Opt_Detail_Spin_Cur, Child_Catch_i);
		}
		else if (pWasher->Mode == HEAT)
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
	if (pOption == Opt_Setting_Wash_Cnt)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Wash_Cnt, GetIndex(List_Setting_Wash_Cnt, Washer_Cur.Wash_Cnt, sizeof(List_Setting_Wash_Cnt)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Wash_Cnt[Child_Catch_i];
		Washer_Cur.Wash_Cnt = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
	}
	else if (pOption == Opt_Setting_Wash_Time)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Wash_Time, GetIndex(List_Setting_Wash_Time, Washer_Cur.Wash_Time, sizeof(List_Setting_Wash_Time)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Wash_Time[Child_Catch_i];
		Washer_Cur.Wash_Time = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
	}
	else if (pOption == Opt_Setting_Spin_Dry_Time)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Spin_Dry_Time, GetIndex(List_Setting_Spin_Dry_Time, Washer_Cur.Spin_Dry_Time, sizeof(List_Setting_Spin_Dry_Time)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Spin_Dry_Time[Child_Catch_i];
		Washer_Cur.Spin_Dry_Time = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
	}
	else if (pOption == Opt_Setting_Water_Volume)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Water_Volume, GetIndex(List_Setting_Water_Volume, Washer_Cur.Water_Volume, sizeof(List_Setting_Water_Volume)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Water_Volume[Child_Catch_i];
		Washer_Cur.Water_Volume = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
	}
	else if (pOption == Opt_Setting_Water_Temp)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Water_Temp, GetIndex(List_Setting_Water_Temp, Washer_Cur.Water_Temp, sizeof(List_Setting_Water_Temp)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Water_Temp[Child_Catch_i];
		Washer_Cur.Water_Temp = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
	}
	else if (pOption == Opt_Setting_Heat_Temp)
	{
		Child_Catch_i = Menu_Run(Opt_Setting_Heat_Temp, GetIndex(List_Setting_Heat_Temp, Washer_Cur.Heat_Temp, sizeof(List_Setting_Heat_Temp)));
		if (Child_Catch_i == -1) return -1;
		New_Val = List_Setting_Heat_Temp[Child_Catch_i];
		Washer_Cur.Heat_Temp = New_Val;
		Menu_Washer_Disaply_Refresh(Washer_Cur);
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
