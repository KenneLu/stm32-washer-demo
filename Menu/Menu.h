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
	char* Name;				//选项名字
	void (*func)(void);		//函数指针
	uint8_t NameLen;		//由于中文占三个字节,用strlen计算名字宽度不再准确,故需额外储存名字宽度
} option_class;

void Menu_Init(void);
void Menu_Run(option_class* option);
int8_t Menu_Roll_Event(void);
int8_t Menu_Enter_Event(void);
int8_t Menu_Back_Event(void);
uint8_t Get_NameLen(char* String);

void Main_Menu(void);
void Tools_Menu(void);
void Games_Menu(void);
void Setting_Menu(void);
void Information(void);

void Washer_Menu(void);
void Fast_Wash_Menu(void);
void Standard_Wash_Menu(void);
void Hard_Wash_Menu(void);
void Swing_Dry_Menu(void);
void Heat_Dry_Menu(void);

#endif
