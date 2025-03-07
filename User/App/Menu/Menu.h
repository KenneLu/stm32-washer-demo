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

// typedef struct __attribute__((packed)) // 这种写法可以减少结构体的内存对齐，以减少对 RAM 的使用
typedef struct
{
	char* Name;				//选项名字
	int8_t(*Func)(void*);	//函数指针，可以最后选中的下标，适配设置选项值的功能。如果无需读取子菜单下标，也可以传递(void)(*Func)(void)，只会报警告。
	void* pFuncParam;		//函数参数，可以传递任意类型的数据，适配设置选项值的功能。
} Option_Class;

typedef void (*MenuPowerOffCallBack) (void);
uint8_t Menu_Power_Off_CBRegister(MenuPowerOffCallBack CB);

void Menu_Init(void);
int8_t Menu_Run(Option_Class* Option, int8_t Choose);

int8_t Setting_Menu(void* Param);

void Menu_Washer_Power_On(void);
void Menu_Washer_Power_Off(void);
int8_t Menu_Washer_Mode_Select(void* Param);
int8_t Menu_Washer_Mode_Detail(void* Param);
int8_t Menu_Washer_Mode_Setting(void* Param);

#endif
