#include "Washer.h"
#include "Key.h"
#include "Delay.h"
#include "OLED.h"
#include "Washer.h"
#include "DHT11.h"
#include "TB6612.h"
#include "MyAD.h"


#define WASHER_CNT_MIN (g_Loop_Cnt / 60) 	// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_S (g_Loop_Cnt / 10) 		// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_100MS (g_Loop_Cnt / 1)	// g_Loop_Cnt与分钟的转换


typedef enum { WASHER_STATUS_ENUM(ENUM_ITEM) } WASHER_STATUS;
const char* Washer_Status[] = { WASHER_STATUS_ENUM(ENUM_STRING) };

typedef enum { WASH_STATUS_ENUM(ENUM_ITEM) } WASH_STATUS;
const char* Wash_Status[] = { WASH_STATUS_ENUM(ENUM_STRING) };

typedef enum { SPIN_DRY_STATUS_ENUM(ENUM_ITEM) } SPIN_DRY_STATUS;
const char* Spin_Dry_Status[] = { SPIN_DRY_STATUS_ENUM(ENUM_STRING) };

static WASHER_STATUS g_Status_Next = S_INIT;
static WASHER_STATUS g_Status_Cur = S_INIT;
static WASHER_STATUS g_Status_Last = S_INIT;

static const Washer* g_Washer;	// 指向当前的洗衣模式
static Washer_Errors g_Washer_Error;	// 错误状态

static uint32_t g_Loop_Cnt = 0;	// 循环计数器
static uint32_t g_Wash_Cnt_Cur = 0;	// 当前洗衣次数
static uint8_t g_Security_Monitor_On = 1;	// 安全监测开关
static uint8_t g_OLED_Need_Refresh = 0;	// 刷新OLED标志位

int8_t Menu_Enter_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Menu_Back_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Menu_Power_Event(void); // 临时这样解决一下 warning，后续再优化


void Washer_OLED_Refresh()
{
	OLED_Clear();
	OLED_ShowString_Easy(4, 1, "Wash Count[ / ]");
	OLED_ShowNum_Easy(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_ShowNum_Easy(4, 14, g_Washer->Wash_Cnt, 1);
}

void Washer_Init(const Washer* pWasher)
{
	if (!pWasher)
	{
		g_Status_Next = S_QUIT;
		return; // 失败，直接退出
	}

	g_Washer = pWasher;
	g_Washer_Error = NO_ERROR;
	g_Status_Last = S_INIT;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;
	g_Security_Monitor_On = 1;  // 启动安全监测
	g_OLED_Need_Refresh = 1;	// 刷新OLED

	// 显示初始化信息
	Washer_OLED_Refresh();
	OLED_ShowString_Easy(1, 1, "Init...        ");

	// 初始化DHT11
	DHT11_Data_t DHT11_Data;
	DHT11_Recive_Data(&DHT11_Data); //接收温度和湿度的数据
	Delay_s(1);

	// 初始化电机驱动
	TB6612_Init();

	// 初始化ADC, 用于检测门是否打开
	MyAD_Init();


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
		g_Status_Next = S_HEAT_DRY;
		break;
	default:
		break;
	}
}

void Washer_Stop()
{
	TB6612_Motor_SetSpeed(0);
}

void Washer_Pause()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_Printf_Easy(3, 1, "pause...");
		Washer_Stop();
	}

	if (Menu_Enter_Event())
	{
		Washer_OLED_Refresh();
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
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_Printf_Easy(1, 1, "ERROR!!!");
		switch (g_Washer_Error)
		{
		case ERROR_DOOR_OPEN:
			OLED_Printf_Easy(2, 1, "Close The Door!");
			break;
		default:
			break;
		}
		Washer_Stop();
	}

	if (g_Washer_Error == NO_ERROR) //异常解除
	{
		Washer_OLED_Refresh();
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
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "heat water...");

		OLED_ShowString_Easy(2, 1, "Temp:");	// 温度
		OLED_ShowString_Easy(2, 8, ".");
		OLED_ShowChinese_Easy(2, 11 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		OLED_ShowString_Easy(3, 1, "Target:");	// 目标温度
		OLED_ShowNum_Easy(3, 8, (uint32_t)g_Washer->Water_Temp, 2);
		OLED_ShowString_Easy(3, 10, ".00");
		OLED_ShowChinese_Easy(3, 13 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整
	}

	DHT11_Data_t DHT11_Data;
	DHT11_Recive_Data(&DHT11_Data); //接收温度和湿度的数据
	Delay_ms(500);

	OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Temp, 2);
	OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Temp_Dec, 2);

	if (DHT11_Data.Temp >= g_Washer->Water_Temp) //烧水结束
	{
		OLED_ShowString_Easy(1, 1, "heat water[DONE]");

		g_Loop_Cnt = 0;
		g_Status_Next = S_ADD_WATER;

		Delay_ms(2000);
	}
}

void Washer_Add_Water()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "add water[  /  ]");
		OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);
	}

	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_Washer->Water_Volume) //加水结束（1s加一升）
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "add water[DONE]");
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_WASH;
	}
}

void Washer_Wash()
{
	static WASH_STATUS Wash_Status_Cur;
	static uint32_t Wash_Loop_Cnt;

	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "washing..[  /  ]");
		OLED_ShowNum_Easy(1, 14, g_Washer->Wash_Time, 2);
		Wash_Status_Cur = S_WASH_TURN_LEFT;
		Wash_Loop_Cnt = 0;
	}

	OLED_ShowNum_Easy(1, 11, WASHER_CNT_MIN, 2);

	switch (Wash_Status_Cur)
	{
	case S_WASH_TURN_LEFT:
		TB6612_Motor_SetSpeed(0);
		if (Wash_Loop_Cnt >= 6) //600ms
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_UP_20;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_20:
		TB6612_Motor_SetSpeed(20);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_UP_40;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_40:
		TB6612_Motor_SetSpeed(40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_DOWN_20;
		}
		break;
	case S_WASH_LEFT_SPEED_DOWN_20:
		TB6612_Motor_SetSpeed(20);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_TURN_RIGHT;
		}
		break;
	case S_WASH_TURN_RIGHT:
		TB6612_Motor_SetSpeed(0);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_UP_20;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_20:
		TB6612_Motor_SetSpeed(-20);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_UP_40;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_40:
		TB6612_Motor_SetSpeed(-40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_DOWN_20;
		}
		break;
	case S_WASH_RIGHT_SPEED_DOWN_20:
		TB6612_Motor_SetSpeed(-20);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_TURN_LEFT;
		}
		break;

	default:
		break;
	}

	g_Loop_Cnt++;
	Wash_Loop_Cnt++;
	if (WASHER_CNT_MIN >= g_Washer->Wash_Time) //清洗结束
	{
		OLED_ShowNum_Easy(1, 11, WASHER_CNT_MIN, 2);
		OLED_ShowString_Easy(2, 1, "washing[DONE]");

		g_Loop_Cnt = 0;
		Wash_Loop_Cnt = 0;
		TB6612_Motor_SetSpeed(0);
		g_Status_Next = S_DRAIN_WATER;

		Delay_ms(2000);
	}
}

void Washer_Drain_Water()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "draing...[  /  ]");
		OLED_ShowNum_Easy(1, 14, g_Washer->Water_Volume, 2);
	}

	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_Washer->Water_Volume) //排水结束（1s排一升）
	{
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "draing[DONE]");
		Delay_ms(2000);

		g_Loop_Cnt = 0;
		g_Status_Next = S_SPIN_DRY;
	}

}

void Washer_Spin_Dry()
{
	static SPIN_DRY_STATUS Spin_Dry_Status_Cur;

	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "spin dry.[00/  ]");
		OLED_ShowNum_Easy(1, 14, g_Washer->Spin_Dry_Time, 2);
		Spin_Dry_Status_Cur = S_SPIN_STOP;
		g_Loop_Cnt = 0;
	}

	switch (Spin_Dry_Status_Cur)
	{
	case S_SPIN_STOP:
		TB6612_Motor_SetSpeed(0);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_20;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_20:
		TB6612_Motor_SetSpeed(20);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_40;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_40:
		TB6612_Motor_SetSpeed(40);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_60;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_60:
		TB6612_Motor_SetSpeed(60);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_80;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_80:
		TB6612_Motor_SetSpeed(80);
		if (WASHER_CNT_S >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_100;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_100:
		TB6612_Motor_SetSpeed(100);
		OLED_ShowNum_Easy(1, 11, WASHER_CNT_MIN, 2);
		if ((WASHER_CNT_MIN) >= g_Washer->Spin_Dry_Time)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_DOWN_60;
		}
		break;
	case S_SPIN_LEFT_SPEED_DOWN_60:
		TB6612_Motor_SetSpeed(60);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_DOWN_40;
		}
	case S_SPIN_LEFT_SPEED_DOWN_40:
		TB6612_Motor_SetSpeed(40);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_DOWN_20;
		}
	case S_SPIN_LEFT_SPEED_DOWN_20:
		TB6612_Motor_SetSpeed(20);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			TB6612_Motor_SetSpeed(0);
			g_Status_Next = S_WASH_CNT;

			OLED_ShowString_Easy(2, 1, "spin dry[DONE]"); //甩干结束
			Delay_ms(2000);
		}
		break;
	default:
		break;
	}
	g_Loop_Cnt++;
}

void Washer_Heat_Dry()
{
	static uint8_t Dry_Relative_Humidity = 30;		// 目标湿度
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "heat dry...");

		OLED_ShowString_Easy(2, 1, "Humi:");	// 湿度
		OLED_ShowString_Easy(2, 8, ".");
		OLED_ShowChinese_Easy(2, 11 / 2 + 1, "％");	   // 中文是16x16的，所以要减半，并向上取整

		OLED_ShowString_Easy(3, 1, "Target:");	// 目标湿度
		OLED_ShowNum_Easy(3, 8, Dry_Relative_Humidity, 2);
		OLED_ShowString_Easy(3, 10, ".00");
		OLED_ShowChinese_Easy(3, 13 / 2 + 1, "％");	   // 中文是16x16的，所以要减半，并向上取整
	}

	DHT11_Data_t DHT11_Data;
	DHT11_Recive_Data(&DHT11_Data); //接收温度和湿度的数据
	Delay_ms(500);

	OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Humi, 2);
	OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Humi_Dec, 2);

	if (DHT11_Data.Humi < Dry_Relative_Humidity) //烘干结束
	{
		g_Loop_Cnt = 0;
		g_Status_Next = S_FINISH;

		OLED_ShowString_Easy(1, 1, "heat dry[DONE]");
		Delay_ms(2000);
	}

}

void Washer_Wash_Cnt()
{
	g_Loop_Cnt = 0;

	Washer_OLED_Refresh();

	//完成一次清洗, 记录次数
	g_Wash_Cnt_Cur++;
	OLED_ShowString_Easy(1, 1, "This round over.");
	OLED_ShowString_Easy(4, 1, "Wash Count[ / ]");
	OLED_ShowNum_Easy(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_ShowNum_Easy(4, 14, g_Washer->Wash_Cnt, 1);

	if (g_Wash_Cnt_Cur >= g_Washer->Wash_Cnt)
	{
		g_Status_Next = S_FINISH;
	}
	else
	{
		OLED_ShowString_Easy(2, 1, "Start next wash.");
		g_Status_Next = S_HEAT_WATER;
	}
	Delay_ms(2000);
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

void Washer_Security_Monitor()
{
	g_Washer_Error = NO_ERROR;


	Delay_ms(500);
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

		case S_WASH_CNT:
			Washer_Wash_Cnt();
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
			g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
		}

		// 安全异常监测
		if (g_Security_Monitor_On)
		{
			g_Washer_Error = NO_ERROR;

			// 检测门是否打开
			static uint16_t* pADValue;
			pADValue = MyAD_GetValue();
			if (pADValue[AD_Comp_TCRT5000] > 300) // 大于1.5cm距离
			{
				g_Washer_Error = ERROR_DOOR_OPEN;
			}

			// 异常处理
			if (g_Washer_Error != NO_ERROR && g_Status_Cur != S_ERROR && g_Status_Cur != S_WASH_CNT)
			{
				g_Status_Next = S_ERROR;
				g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
				if (g_Status_Cur != S_PAUSE) // 异常解除后，直接继续原状态机，取消暂停
				{
					g_Status_Last = g_Status_Cur; // 保存异常前的状态机
				}
				g_Status_Cur = S_ERROR;
			}
		}

		// 暂停捕获，ERROR 状态下不暂停
		if (Menu_Enter_Event() && g_Status_Cur != S_ERROR && g_Status_Cur != S_WASH_CNT)
		{
			g_Status_Next = S_PAUSE;
			g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
		}

		Delay_ms(100);
		if (Menu_Back_Event()) {
			Washer_Stop();
			return -1;
		}
	}
}
