#ifndef __MENU_H
#define __MENU_H

#include "stdint.h"

#define		WORD_H 		16				//字高word height

enum CursorStyle
{
	reverse,
	mouse,
	frame,
};

extern enum CursorStyle CurStyle;
extern int8_t Speed_Factor;				//光标动画速度;
extern float Roll_Speed;				//滚动动画速度;

typedef struct
{
	char Name[24];			//选项名字，为了解决指针浅拷贝而采用数组，但是会导致内存占用过多，如果报错Error: L6406E: No space in execution regions with .ANY selector matching main.o(.bss).则继续减小这个数组的大小
	int8_t(*func)(void);	//函数指针，可以最后选中的下标，适配设置选项值的功能。如果无需读取子菜单下标，也可以传递(void)(*func)(void)，只会报警告。
	uint8_t NameLen;		//由于中文占三个字节,用strlen计算名字宽度不再准确,故需额外储存名字宽度
} Option_Class;

void Menu_Init(void);
int8_t Menu_Run(Option_Class* Option, int8_t Choose);
int8_t Menu_Roll_Event(void);
int8_t Menu_Enter_Event(void);
int8_t Menu_Back_Event(void);
uint8_t Get_NameLen(char* String);

int8_t Main_Menu(void);
int8_t Tools_Menu(void);
int8_t Games_Menu(void);
int8_t Setting_Menu(void);
int8_t Information_Menu(void);

int8_t Washer_Menu(void);
int8_t Fast_Wash_Menu(void);
int8_t Standard_Wash_Menu(void);
int8_t Hard_Wash_Menu(void);
int8_t Swing_Dry_Menu(void);
int8_t Heat_Dry_Menu(void);

#endif
