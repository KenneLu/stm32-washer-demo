#include "Menu.h"
#include "OLED.h"
#include "Game_Snake.h" 
#include "Game_Of_Life.h" 

void Tools_Menu(void)
{
	option_class option_list[] = {
		{"退出"},
		{"定时器", },			//6-1 定时器定时中断
		{"输入捕获", },			//6-6 输入捕获模式测频率
		{"PWM输出", },			//6-3 PWM驱动LED呼吸灯
		{"PWM输入", },			//6-7 PWMI模式测频率占空比
		{"编码器", },			//6-8 编码器接口测速f
		{"串口", },				//9-3 串口收发HEX数据包
		{"示波器", },			//示波器
		{"ADC", },				//8-2 DMA+AD多通道
		{".."}
	};

	Menu_Run(option_list);
}

void Games_Menu(void)
{
	option_class option_list[] = {
		{"<<<"},
		{"Snake", Game_Snake_Init},				//贪吃蛇
		{"康威生命游戏", Game_Of_Life_Play},	//康威生命游戏,元胞自动机
		{".."}
	};

	Menu_Run(option_list);
}

void Information(void)
{
	int16_t Angle = 350;
	while (1)
	{
		OLED_Clear();

		OLED_ShowImage(88, 8, 32, 32, goutou);
		OLED_Rotation_Block(88 + 16, 8 + 16, 16, Angle);

		Angle += Menu_Roll_Event() * 9;
		Angle += 2;
		Angle %= 360;

		OLED_ShowString(2, 0, "Menu V1.2", OLED_8X16);
		OLED_ShowString(2, 32, "By:Adam", OLED_8X16);
		OLED_ShowString(2, 48, "UP:加油哦大灰狼", OLED_8X16);

		OLED_Update();
		if (Menu_Enter_Event()) { return; }
		if (Menu_Back_Event()) { return; }
	}
}

/**********************************************************/