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

WASHER_STATUS g_Status_Next = WS_CHECK;
WASHER_STATUS g_Status_Cur = WS_CHECK;
WASHER_STATUS g_Status_Last = WS_CHECK;

const Washer* g_Washer;

uint8_t g_Loop_Cnt;
uint8_t g_Wash_Cnt_Cur;

int8_t Menu_Enter_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Menu_Back_Event(void); // 临时这样解决一下 warning，后续再优化

int8_t Washer_Init(const Washer* pWasher)
{
	OLED_Clear();
	g_Status_Next = WS_CHECK;
	g_Status_Last = WS_CHECK;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;
	if (pWasher)
	{
		g_Washer = pWasher;
		return 1;
	}
	return 0;
}

void Washer_Check()
{
	OLED_ShowString_Easy(2, 1, "check...");
	OLED_ShowString_Easy(3, 1, "NOW count:[");
	OLED_ShowNum_Easy(3, 12, g_Wash_Cnt_Cur, 1);
	OLED_ShowString_Easy(3, 13, "/");
	OLED_ShowNum_Easy(3, 14, g_Washer->Wash_Cnt, 1);
	OLED_ShowString_Easy(3, 15, "]");
	g_Loop_Cnt++;
	if (3 == g_Loop_Cnt) //自检结束
	{
		OLED_Printf_Easy(1, 1, "[check] done");

		g_Loop_Cnt = 0;
		g_Status_Next = WS_IDLE;
	}
}

/*空闲(等待模式设置)状态*/
void Washer_Idle()
{
	if (Menu_Enter_Event())
	{
		g_Loop_Cnt = 0;
		g_Status_Next = WS_ADD_WATER;
	}
}

/*加水状态*/
void Washer_Add_Water()
{
	OLED_ShowString_Easy(2, 1, "               ");
	OLED_ShowString_Easy(2, 1, "add water...");

	if (Menu_Enter_Event())
	{
		g_Status_Next = WS_PAUSE;
	}
	else
	{
		g_Loop_Cnt++;
		if (g_Washer->Water_Volume == (g_Loop_Cnt / 10)) //加水结束
		{
			g_Loop_Cnt = 0;
			g_Status_Next = WS_WASH;
			OLED_ShowString_Easy(1, 1, "               ");
			OLED_ShowString_Easy(1, 1, "[add_water] done");
		}
	}
}

/*清洗状态*/
void Washer_Wash()
{
	OLED_ShowString_Easy(2, 1, "               ");
	OLED_ShowString_Easy(2, 1, "wash...");

	if (Menu_Enter_Event())
	{
		g_Status_Next = WS_PAUSE;
	}
	else
	{
		g_Loop_Cnt++;
		if (g_Washer->Wash_Time == (g_Loop_Cnt / 10)) //清洗结束
		{
			g_Loop_Cnt = 0;
			g_Status_Next = WS_DRAIN_WATER;
			OLED_ShowString_Easy(1, 1, "               ");
			OLED_ShowString_Easy(1, 1, "[wash] done.");
		}
	}
}

/*排水状态*/
void Washer_Drain_Water()
{
	OLED_ShowString_Easy(2, 1, "               ");
	OLED_ShowString_Easy(2, 1, "drain water...");

	if (Menu_Enter_Event())
	{
		g_Status_Next = WS_PAUSE;
	}
	else
	{
		g_Loop_Cnt++;
		if (g_Washer->Water_Volume == (g_Loop_Cnt / 10)) //排水结束
		{
			g_Loop_Cnt = 0;
			OLED_ShowString_Easy(1, 1, "               ");
			OLED_ShowString_Easy(1, 1, "[drain] done");
			g_Status_Next = WS_SPIN_DRY;
		}
	}
}

/*甩干状态*/
void Washer_Spin_Dry()
{
	OLED_ShowString_Easy(2, 1, "               ");
	OLED_ShowString_Easy(2, 1, "spin dry...");

	if (Menu_Enter_Event())
	{
		g_Status_Next = WS_PAUSE;
	}
	else
	{
		g_Loop_Cnt++;
		if (g_Washer->Spin_Dry_Time == (g_Loop_Cnt / 10)) //甩干结束
		{
			g_Loop_Cnt = 0;
			g_Status_Next = WS_IDLE;
			OLED_ShowString_Easy(1, 1, "               ");
			OLED_ShowString_Easy(1, 1, "[spin_dry] done");

			//完成一次清洗, 记录次数
			g_Wash_Cnt_Cur++;
			OLED_ShowString_Easy(3, 1, "               ");
			OLED_ShowString_Easy(3, 1, "NOW count:[");
			OLED_ShowNum_Easy(3, 12, g_Wash_Cnt_Cur, 1);
			OLED_ShowString_Easy(3, 13, "/");
			OLED_ShowNum_Easy(3, 14, g_Washer->Wash_Cnt, 1);
			OLED_ShowString_Easy(3, 15, "]");

			if (g_Washer->Wash_Cnt == g_Wash_Cnt_Cur)
			{
				g_Status_Next = WS_IDLE;
			}
			else
			{
				g_Status_Next = WS_ADD_WATER;
			}
		}
	}
}

/*暂停状态*/
void Washer_Pause()
{
	OLED_Clear();
	OLED_Printf_Easy(1, 1, "pause...");

	if (Menu_Enter_Event())
	{
		switch (g_Status_Last)
		{
		case WS_ADD_WATER:
			g_Status_Next = WS_ADD_WATER;
			break;
		case WS_WASH:
			g_Status_Next = WS_WASH;
			break;
		case WS_DRAIN_WATER:
			g_Status_Next = WS_DRAIN_WATER;
			break;
		case WS_SPIN_DRY:
			g_Status_Next = WS_SPIN_DRY;
			break;
		default: break;
		}
	}
}

int8_t Washer_Run(void* Param)
{
	if (!Washer_Init((Washer*)Param)) return -1;

	while (1)
	{
		switch (g_Status_Next)
		{
			/*开机初始化自检*/
		case WS_CHECK:
			Washer_Check();
			break;

			/*空闲(等待模式设置)状态*/
		case WS_IDLE:
			Washer_Idle();
			break;

			/*加水状态*/
		case WS_ADD_WATER:
			Washer_Add_Water();
			break;

			/*清洗状态*/
		case WS_WASH:
			Washer_Wash();
			break;

			/*排水状态*/
		case WS_DRAIN_WATER:
			Washer_Drain_Water();
			break;

			/*甩干状态*/
		case WS_SPIN_DRY:
			Washer_Spin_Dry();
			break;

			/*暂停状态*/
		case WS_PAUSE:
			Washer_Pause();
			break;

		default: break;
		}

		if (g_Status_Cur != g_Status_Next)
		{
			g_Status_Last = g_Status_Cur;
			g_Status_Cur = g_Status_Next;
		}
		Delay_ms(100);
		if (Menu_Back_Event()) return -1;
	}
}
