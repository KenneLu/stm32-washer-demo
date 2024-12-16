#include "Menu.h" 
#include <string.h>

int8_t Set_CursorStyle(void* Param);
int8_t Set_animation_speed(void* Param);


Option_Class option_list[] = {
	{"退出"},
	{"光标风格[反相]", Set_CursorStyle},
	{"动画速度[快]", Set_animation_speed},
	{".."}
};

int8_t Set_CursorStyle(void* Param)
{
	if (CurStyle == reverse)
	{
		CurStyle = mouse;
		strcpy(option_list[1].Name, "光标风格[鼠标]");
	}
	else if (CurStyle == mouse)
	{
		CurStyle = frame;
		strcpy(option_list[1].Name, "光标风格[矩形]");
	}
	else //if(CurStyle == frame)
	{
		CurStyle = reverse;
		strcpy(option_list[1].Name, "光标风格[反相]");
	}
	return -1;
}

int8_t Set_animation_speed(void* Param)
{
	if (Speed_Factor == 8)
	{
		Speed_Factor = 16;
		Roll_Speed = 1.1;
		strcpy(option_list[2].Name, "动画速度[慢]");
	}
	else if (Speed_Factor == 16)
	{
		Speed_Factor = 1;
		Roll_Speed = 16;
		strcpy(option_list[2].Name, "动画速度[关]");
	}
	else //if(Speed_Factor == 16)
	{
		Speed_Factor = 8;
		Roll_Speed = 2;
		strcpy(option_list[2].Name, "动画速度[快]");
	}
	return -1;
}

int8_t Setting_Menu(void* Param)
{
	return Menu_Run(option_list, 0);
}
