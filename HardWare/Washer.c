#include "Washer.h"
#include "Key.h"
#include "Delay.h"
#include "OLED.h"
#include "Washer.h"

typedef enum
{
	WASHER_STATUS_ENUM(ENUM_ITEM)
}WASHER_STATUS;

const char* Washer_Status[] = {
	WASHER_STATUS_ENUM(ENUM_STRING)
};

WASHER_STATUS g_Status_Next = S_INIT;
WASHER_STATUS g_Status_Cur = S_INIT;
WASHER_STATUS g_Status_Last = S_INIT;

const Washer* g_Washer;

uint8_t g_Loop_Cnt;
uint8_t g_Wash_Cnt_Cur;
uint8_t g_Security_Monitor_On;

int8_t Menu_Enter_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Menu_Back_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Menu_Power_Event(void); // 临时这样解决一下 warning，后续再优化

void Get_Pause(void)
{

}

void Washer_Init(const Washer* pWasher)
{
	if (!pWasher)
	{
		g_Status_Next = S_QUIT;
		return; // 失败，直接退出
	}

	OLED_Clear();
	g_Washer = pWasher;
	g_Status_Last = S_INIT;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;
	g_Security_Monitor_On = 1; // 启动安全监测

	// 根据模式选择状态机
	switch (pWasher->Mode)
	{
	case M_FAST_WASH:
		g_Status_Next = S_HEAT_WATER;
		break;
	case M_STANDERD_WASH:
		g_Status_Next = S_HEAT_WATER;
		break;
	case M_HARD_WASH:
		g_Status_Next = S_HEAT_WATER;
		break;
	case M_SPIN_DRY:
		g_Status_Next = S_DRAIN_WATER;
		break;
	case M_HEAT_DRY:
		g_Status_Next = S_DRAIN_WATER;
		break;
	default:
		break;
	}
}

void Washer_Pause()
{
	OLED_Printf_Easy(3, 1, "pause...");

	if (Menu_Enter_Event())
	{
		OLED_ShowString_Easy(3, 1, "        ");
		switch (g_Status_Last)
		{
		case S_HEAT_WATER:
			g_Status_Next = S_HEAT_WATER;
			break;
		case S_ADD_WATER:
			g_Status_Next = S_ADD_WATER;
			break;
		case S_WASH:
			g_Status_Next = S_WASH;
			break;
		case S_DRAIN_WATER:
			g_Status_Next = S_DRAIN_WATER;
			break;
		case S_SPIN_DRY:
			g_Status_Next = S_SPIN_DRY;
			break;
		case S_HEAT_DRY:
			g_Status_Next = S_HEAT_DRY;
			break;
		default:
			break;
		}
	}
}

void Washer_Error()
{
	OLED_Printf_Easy(3, 1, "ERROR!!!");

	if (Menu_Power_Event()) //异常解除
	{
		OLED_ShowString_Easy(3, 1, "        ");
		switch (g_Status_Last)
		{
		case S_INIT:
			g_Status_Next = S_INIT;
			break;
		case S_HEAT_WATER:
			g_Status_Next = S_HEAT_WATER;
			break;
		case S_ADD_WATER:
			g_Status_Next = S_ADD_WATER;
			break;
		case S_WASH:
			g_Status_Next = S_WASH;
			break;
		case S_DRAIN_WATER:
			g_Status_Next = S_DRAIN_WATER;
			break;
		case S_SPIN_DRY:
			g_Status_Next = S_SPIN_DRY;
			break;
		case S_HEAT_DRY:
			g_Status_Next = S_HEAT_DRY;
			break;
		default:
			break;
		}
	}
}

void Washer_Heat_Water()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "heat water...");
	// Delay_ms(2000);

	Get_Pause();  // 捕获暂停
	if (1) //烘干结束
	{
		OLED_ShowString_Easy(1, 1, "heat water[DONE]");
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_ADD_WATER;
	}

}

void Washer_Add_Water()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "add water[  /  ]");
	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
	OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);

	Get_Pause();  // 捕获暂停
	g_Loop_Cnt++;
	if (g_Washer->Water_Volume == (g_Loop_Cnt / 10)) //加水结束
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_WASH;
	}
}

void Washer_Wash()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "washing..[  /  ]");
	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
	OLED_ShowNum_Easy(1, 14, g_Washer->Wash_Time, 2);

	Get_Pause();  // 捕获暂停
	g_Loop_Cnt++;
	if (g_Washer->Wash_Time == (g_Loop_Cnt / 10)) //清洗结束
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowNum_Easy(1, 14, g_Washer->Wash_Time, 2);
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_DRAIN_WATER;
	}

}

void Washer_Drain_Water()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "draing...[  /  ]");
	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
	OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);

	Get_Pause();  // 捕获暂停
	g_Loop_Cnt++;
	if (g_Washer->Water_Volume == (g_Loop_Cnt / 10)) //排水结束
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_SPIN_DRY;
	}

}

void Washer_Spin_Dry()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "spin dry.[  /  ]");
	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
	OLED_ShowNum_Easy(1, 14, g_Washer->Spin_Dry_Time, 2);

	Get_Pause();  // 捕获暂停
	g_Loop_Cnt++;
	if (g_Washer->Spin_Dry_Time == (g_Loop_Cnt / 10)) //甩干结束
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowNum_Easy(1, 14, g_Washer->Spin_Dry_Time, 2);
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_HEAT_DRY;
	}
}

void Washer_Heat_Dry()
{
	OLED_ShowString_Easy(1, 1, "                ");
	OLED_ShowString_Easy(1, 1, "heat dry.[  /  ]");
	// Delay_ms(2000);

	Get_Pause();  // 捕获暂停
	if (1) //烘干结束
	{
		OLED_ShowString_Easy(1, 1, "heat dry.[DONE ]");

		//完成一次清洗, 记录次数
		g_Wash_Cnt_Cur++;
		OLED_ShowString_Easy(2, 1, "Wash Count[ / ]");
		OLED_ShowNum_Easy(2, 12, g_Wash_Cnt_Cur, 1);
		OLED_ShowNum_Easy(2, 14, g_Washer->Wash_Cnt, 1);
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		if (g_Washer->Wash_Cnt == g_Wash_Cnt_Cur)
		{
			g_Status_Next = S_FINISH;
		}
		else
		{
			g_Status_Next = S_HEAT_WATER;
		}
	}

}

void Washer_Finish()
{
	OLED_Clear();
	OLED_ShowString_Easy(1, 1, "WASH FINISH!");
	g_Security_Monitor_On = 0; // 关闭安全监测

	if (Menu_Enter_Event())
	{
		g_Status_Next = S_QUIT;
	}
}

int8_t Washer_Run(void* Param)
{
	g_Status_Next = S_INIT;	// 首次进入，初始化状态机
	while (1)
	{
		switch (g_Status_Next)
		{
		case S_INIT:
			Washer_Init((Washer*)Param);
			break;

		case S_PAUSE:
			Washer_Pause();
			break;

		case S_ERROR:
			Washer_Error();
			break;

		case S_HEAT_WATER:
			Washer_Heat_Water();
			break;

		case S_ADD_WATER:
			Washer_Add_Water();
			break;

		case S_WASH:
			Washer_Wash();
			break;

		case S_DRAIN_WATER:
			Washer_Drain_Water();
			break;

		case S_SPIN_DRY:
			Washer_Spin_Dry();
			break;

		case S_HEAT_DRY:
			Washer_Heat_Dry();
			break;

		case S_FINISH:
			Washer_Finish();
			break;

		case S_QUIT:
			return -1; // 退出运行，返回模式选择

		default:
			break;
		}

		// 状态机更新
		if (g_Status_Cur != g_Status_Next)
		{
			g_Status_Last = g_Status_Cur;
			g_Status_Cur = g_Status_Next;
		}

		// 安全异常监测
		if (g_Security_Monitor_On && Menu_Power_Event())
		{
			g_Status_Next = S_ERROR;
			if (g_Status_Cur != S_PAUSE) // 异常解除后，直接继续原状态机，取消暂停
			{
				g_Status_Last = g_Status_Cur; // 保存异常前的状态机
			}
			g_Status_Cur = S_ERROR;
		}

		// 暂停捕获，ERROR 状态下不暂停
		if (Menu_Enter_Event() && g_Status_Cur != S_ERROR)
		{
			g_Status_Next = S_PAUSE;
		}

		Delay_ms(100);
		if (Menu_Back_Event()) return -1;
	}
}
