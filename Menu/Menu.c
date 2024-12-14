#include "stm32f10x.h"                  // Device header
#include "Menu.h"
#include "OLED.h"
#include "Key.h"
#include "Encoder.h"

/**
  *多级菜单框架v1.2
  *添加OLED补丁,打印字符串函数支持中英文同时打印,识别换行符和超出屏幕边界自动换行;
  *添加帧率显示,可以跳转到 OLED_Update 函数解除注释开启;
  *
  *增加返回键和上下方向键, 没有编码器也可以使用
  *菜单用到的按键函数独立出来,方便移植和修改,比如没有编码器可以用上下两个按键代替;
  */

uint8_t Key_Enter = 0;	//确认键
uint8_t Key_Back = 0;	//返回键

uint8_t Key_Enter_Get(void);
uint8_t Key_Back_Get(void);
void Change_Enter_Key(void);
void Change_Back_Key(void);

/**
  * 函    数：菜单初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：把菜单相关按钮功能进行注册
  */
void Menu_Init(void)
{
	Key_CBRegister_R(KEY_ENCODER_PRESS, Change_Enter_Key);
	Key_CBRegister_LP(KEY_ENCODER_PRESS, Change_Back_Key);
}

/**
  * 函    数：菜单运行
  * 参    数：选项列表
  * 返 回 值：无
  * 说    明：把选项列表显示出来,并根据按键事件执行相应操作
  */
void Menu_Run(option_class* option)
{
	int8_t Catch_i = 1;		//选中下标
	int8_t Cursor_i = 0;	//光标下标
	int8_t Show_i = 0; 		//显示起始下标
	int8_t Max = 0;			//选项数量
	int8_t Roll_Event = 0;	//编码器事件

	while (option[++Max].Name[0] != '.');// {Max++;}	//获取条目数量,如果文件名开头为'.'则为结尾;
	Max--;											//不打印".."

	for (int8_t i = 0; i <= Max; i++)				//计算选项名宽度;
	{
		option[i].NameLen = Get_NameLen(option[i].Name);
	}

	static float Cursor_len_d0 = 0, Cursor_len_d1 = 0, Cursor_i_d0 = 0, Cursor_i_d1 = 0; 			//光标位置和长度的起点终点

	int8_t Show_d = 0, Show_i_temp = Max;				//显示动画相关;

	while (1)
	{
		OLED_Clear();

		Roll_Event = Menu_Roll_Event();				//获取滚动事件
		if (Roll_Event)								//如果有按键事件;
		{
			Cursor_i += Roll_Event;					//更新下标
			Catch_i += Roll_Event;

			if (Catch_i < 0) { Catch_i = 0; }			//限制选中下标
			if (Catch_i > Max) { Catch_i = Max; }

			if (Cursor_i < 0) { Cursor_i = 0; }		//限制光标位置
			if (Cursor_i > 3) { Cursor_i = 3; }
			if (Cursor_i > Max) { Cursor_i = Max; }
		}

		/**********************************************************/
		/*显示相关*/

		Show_i = Catch_i - Cursor_i;				//计算显示起始下标

		if (1)	//加显示动画
		{
			if (Show_i - Show_i_temp)				//如果下标有偏移
			{
				Show_d = (Show_i - Show_i_temp) * WORD_H;	//
				Show_i_temp = Show_i;
			}
			if (Show_d) { Show_d /= Roll_Speed; }		//滚动变化量: 2 快速, 1.26 较平滑;

			/*如果菜单向下移动,Show_d = -16往0移动期间由于显示字符串函数不支持Y坐标为负数,跳过了打印,所以首行是空的,所以在首行打印Show_i - ((Show_d)/WORD_H)的选项名字,达到覆盖效果;((Show_d)/WORD_H)代替0,兼容Show_d <= -16的情况(菜单开始动画)*/
			//if(Show_d < 0) {OLED_ShowString(2, 0, option[Show_i - ((Show_d)/WORD_H)].Name, OLED_8X16);}
			/*如果菜单向上移动,Show_d = 16往0移动期间首行是空的,所以在首行打印Show_i - 1的选项名字,达到覆盖效果;*/
			//if(Show_d > 0) {OLED_ShowString(2, 0, option[Show_i - 1].Name, OLED_8X16);}
		}
		for (int8_t i = 0; i < 5; i++)				//遍历显示选项名
		{
			if (Show_i + i > Max) { break; }
			OLED_ShowString(2, (i * WORD_H) + Show_d, option[Show_i + i].Name, OLED_8X16);
		}

		/**********************************************************/
		/*光标相关*/

		if (1)	//加光标动画
		{
			Cursor_i_d1 = Cursor_i * WORD_H;						//轮询光标目标位置
			Cursor_len_d1 = option[Catch_i].NameLen * 8 + 4;		//轮询光标目标宽度

			/*计算此次循环光标位置*///如果当前位置不是目标位置,当前位置向目标位置移动;
			if ((Cursor_i_d1 - Cursor_i_d0) > 1) { Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed_Factor + 1; }
			else if ((Cursor_i_d1 - Cursor_i_d0) < -1) { Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed_Factor - 1; }
			else { Cursor_i_d0 = Cursor_i_d1; }

			/*计算此次循环光标宽度*/
			if ((Cursor_len_d1 - Cursor_len_d0) > 1) { Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed_Factor + 1; }
			else if ((Cursor_len_d1 - Cursor_len_d0) < -1) { Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed_Factor - 1; }
			else { Cursor_len_d0 = Cursor_len_d1; }
		}
		else { Cursor_i_d0 = Cursor_i * WORD_H; Cursor_len_d0 = 128; }

		//显示光标
		if (CurStyle == reverse)
		{
			OLED_ReverseArea(0, Cursor_i_d0, Cursor_len_d0, WORD_H);			//反相光标
		}
		if (CurStyle == mouse)
		{
			OLED_ShowString(Cursor_len_d0, Cursor_i_d0 + 5, "<-", OLED_6X8);		//尾巴光标
			//OLED_ShowImage(Cursor_len_d0, Cursor_i_d0+6, 8, 8, Cursor);		//图片光标
		}
		if (CurStyle == frame)
		{
			OLED_DrawRectangle(0, Cursor_i_d0, Cursor_len_d0, WORD_H, 0);		//矩形光标
		}

		OLED_ShowNum(116, 56, Catch_i, 2, OLED_6X8);							//右下角显示选中下标;
		OLED_Update();
		//int delay = 1000000; while(delay--);
	/**********************************************************/

		if (Menu_Enter_Event())			//获取按键
		{
			/*如果功能不为空则执行功能,否则返回*/
			if (option[Catch_i].func) { option[Catch_i].func(); }
			else { return; }
		}
		if (Menu_Back_Event()) { return; }	//获取按键
	}
}

int8_t Menu_Roll_Event(void)//菜单滚动
{
	// if (Key_Up_Get()) { return 1; }			//按键上
	// if (Key_Down_Get()) { return -1; }		//按键下
	return Encoder_Get_Div4();			//旋钮编码器
}

int8_t Menu_Enter_Event(void)//菜单确认
{
	return Key_Enter_Get();				//确认键接到PB14;
}

int8_t Menu_Back_Event(void)//菜单返回
{
	return Key_Back_Get();;				//返回键接到PB12;
}

enum CursorStyle CurStyle = reverse;
int8_t Speed_Factor = 8;																		//光标动画速度系数;
float Roll_Speed = 2;																			//滚动动画速度系数;

/**
  * 函    数：主菜单
  * 参    数：无
  * 返 回 值：无
  * 说    明：按照格式添加选项

  */
void Main_Menu(void)
{
	option_class option_list[] = {
		{"关机"},
		{"Tools", Tools_Menu},
		{"Games", Games_Menu},
		{"Setting", Setting_Menu},			//设置
		{"Information", Information},		//信息
		{".."}								//结尾标志,方便自动计算数量
	};

	Menu_Run(option_list);
}

//计算选项名宽度;
uint8_t Get_NameLen(char* String)
{
	uint8_t i = 0, len = 0;
	while (String[i] != '\0')			//遍历字符串的每个字符
	{
		if (String[i] > '~') { len += 2; i += 3; }	//如果不属于英文字符长度加2
		else { len += 1; i += 1; }					//属于英文字符长度加1
	}
	return len;
}

uint8_t Key_Enter_Get(void)
{
	if (Key_Enter)
	{
		Key_Enter = 0;
		return 1;
	}
	return 0;
}

uint8_t Key_Back_Get(void)
{
	if (Key_Back)
	{
		Key_Back = 0;
		return 1;
	}
	return 0;
}

void Change_Enter_Key(void)
{
	Key_Enter = 1;
}

void Change_Back_Key(void)
{
	Key_Back = 1;
}
