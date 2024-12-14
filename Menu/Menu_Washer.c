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
	uint8_t Wash_Temp;		//水温（℃）
	uint8_t Heat_Temp;		//烘干温度（℃）
	uint8_t Total_Time;		//洗衣时间总和（小时-分）
} Washer_Attribute;

//List
uint8_t Wash_Count_L[] = { 2, 3, 4, 5 }; //次
uint8_t Wash_Time_L[] = { 5, 10, 15, 20 };	//分
uint8_t Swing_Dry_Time_L[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };	//分
uint8_t Water_Volume_L[] = { 3, 4, 5, 6, 7, 8, 9, 10 };	//升
uint8_t Wash_Temp_L[] = { 0, 20, 30, 40, 50 };	//℃，0度就是常温
uint8_t Heat_Temp_L[] = { 40, 45, 50, 60, 70 };	//℃

//pointer
uint8_t* pWash_Count_L = Wash_Count_L;
uint8_t* pWash_Time_L = Wash_Time_L;
uint8_t* pSwing_Dry_Time_L = Swing_Dry_Time_L;
uint8_t* pWater_Volume_L = Water_Volume_L;
uint8_t* pWash_Temp_L = Wash_Temp_L;
uint8_t* pHeat_Temp_L = Heat_Temp_L;

Washer_Attribute Fast, Standard, Hard, Swing_Dry, Heat_Dry;
Wash_Mode CurMode;

option_class Wash_Options[] = {
		{ "启动", },
		{ "漂洗[次]",  },
		{ "洗涤[分]",  },
		{ "脱水[分]",  },
		{ "水量[升]",  },
		{ "水温[℃]",  },
		{ ".." }
};

option_class Swing_Options[] = {
		{ "启动", },
		{ "脱水[分]",  },
		{ ".." }
};

option_class Heat_Options[] = {
		{ "启动", },
		{ "烘干[℃]",  },
		{ ".." }
};

void Fast_Menu(void);
void Standard_Wash_Menu(void);
void Hard_Wash_Menu(void);
void Swing_Dry_Menu(void);
void Heat_Dry_Menu(void);
char* Uchar2Str(unsigned char Num);
void Insert_SubString(char* Str, char* SubStr, int Index);

option_class Cur_Wash_Options[sizeof(Wash_Options)];

uint8_t Wash_Options_Inited = 0;
void Washer_Init(void)
{
	Fast.Wash_Count = *pWash_Count_L;
	Fast.Wash_Time = *pWash_Time_L;
	Fast.Swing_Dry_Time = *(pSwing_Dry_Time_L + 2);
	Fast.Water_Volume = *pWater_Volume_L;
	Fast.Total_Time = *pWash_Count_L * (Fast.Wash_Time + Fast.Swing_Dry_Time + Fast.Swing_Dry_Time);

	Standard.Wash_Count = *(pWash_Count_L + 1);
	Standard.Wash_Time = *(pWash_Time_L + 1);
	Standard.Swing_Dry_Time = *(pSwing_Dry_Time_L + 4);
	Standard.Water_Volume = *(pWater_Volume_L + 2);
	Standard.Total_Time = *pWash_Count_L * (Standard.Wash_Time + Standard.Swing_Dry_Time + Standard.Swing_Dry_Time);

	Hard.Wash_Count = *(pWash_Count_L + 2);
	Hard.Wash_Time = *(pWash_Time_L + 2);
	Hard.Swing_Dry_Time = *(pSwing_Dry_Time_L + 8);
	Hard.Water_Volume = *(pWater_Volume_L + 5);
	Hard.Total_Time = *pWash_Count_L * (Hard.Wash_Time + Hard.Swing_Dry_Time + Hard.Swing_Dry_Time);

	Swing_Dry.Swing_Dry_Time = *(pSwing_Dry_Time_L + 2);
	Swing_Dry.Total_Time = Swing_Dry.Swing_Dry_Time;

	Heat_Dry.Heat_Temp = *(pHeat_Temp_L + 1);
}

//洗衣机的主菜单
void Washer_Menu(void)
{
	Washer_Init();
	option_class option_list[] = {
		{ "快速洗", Fast_Menu},
		{ "标准洗", Standard_Wash_Menu},
		{ "强力洗", Hard_Wash_Menu},
		{ "脱水", Swing_Dry_Menu},
		{ "烘干", Heat_Dry_Menu},
		{ ".." }	//结尾标志,方便自动计算数量
	};
	Menu_Run(option_list);
}

void Wash_Menu_Reset(Washer_Attribute Washer)
{
	memcpy(Cur_Wash_Options, Wash_Options, sizeof(Wash_Options) / sizeof(Wash_Options[0]) * sizeof(option_class));
	Insert_SubString(Cur_Wash_Options[1].Name, Uchar2Str(Washer.Wash_Count), 7);
	Insert_SubString(Cur_Wash_Options[2].Name, Uchar2Str(Washer.Wash_Time), 7);
	Insert_SubString(Cur_Wash_Options[3].Name, Uchar2Str(Washer.Swing_Dry_Time), 7);
	Insert_SubString(Cur_Wash_Options[4].Name, Uchar2Str(Washer.Water_Volume), 7);
	Insert_SubString(Cur_Wash_Options[5].Name, Uchar2Str(Washer.Total_Time), 7);
}

void Fast_Menu(void)
{
	CurMode = FAST;
	Wash_Menu_Reset(Fast);
	Menu_Run(Cur_Wash_Options);
}

void Standard_Wash_Menu(void)
{
	CurMode = STANDERD;
	Wash_Menu_Reset(Standard);
	Menu_Run(Cur_Wash_Options);
}

void Hard_Wash_Menu(void)
{
	CurMode = HARD;
	Wash_Menu_Reset(Hard);
	Menu_Run(Cur_Wash_Options);
}

void Swing_Dry_Menu(void)
{
	CurMode = SWING;
	option_class option_list[] = {
		{ "启动", },
		{ "脱水[05分]",  },
		{ ".." }
	};
	Menu_Run(option_list);
}

void Heat_Dry_Menu(void)
{
	CurMode = HEAT;
	option_class option_list[] = {
		{ "启动", },
		{ "烘干[40℃]",  },
		{ ".." }
	};
	Menu_Run(option_list);
}

void Wash_Count_Setting(void)
{
	uint8_t Cur_Wash_Count;
	if (CurMode == FAST)
	{
		Cur_Wash_Count = Fast.Wash_Count;
		while (1)
		{
			int8_t Offset = Encoder_Get_Div4();
			if (Offset != 0)
			{
				Cur_Wash_Count = *(pWash_Count_L + Offset);
				//TODO: 显示当前设置值
				// option_list[1].Name = "光标风格[矩形]";
			}
			if (Menu_Enter_Event())
			{
				Fast.Wash_Count = Cur_Wash_Count;
				return;
			}
			if (Menu_Back_Event()) return;
		}
	}
}

char* Uchar2Str(unsigned char Num) {
	// 分配足够的内存来存储转换后的字符串和一个终止符
	char* Str = (char*)malloc(11 * sizeof(char)); // 10 digits for max unsigned char + 1 for null terminator
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
