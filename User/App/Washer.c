#include "stm32f10x.h"
#include "Menu.h"
#include "Washer.h"
#include "Sys_Delay.h"
#include "Drv_Key.h"
#include "Drv_OLED.h"
#include "Drv_DHT11.h"
#include "Drv_TB6612.h"
#include "Drv_AD.h"
#include "Drv_ServoMotor.h"
#include "Drv_MPU6050.h"
#include "Drv_Buzzer.h"
#include "Drv_W25Q64.h"

static BUZZER_Device* g_pDev_Buzzer;
static MPU6050_Device* g_pDev_MPU6050;
static W25Q64_Device* g_pDev_W25Q64;
static AD_Device* g_pDev_TCRT5000;
static SERVOMOTOR_Device* g_pDev_ServoMotor;
static TB6612_Device* g_pDev_TB6612;

static uint8_t g_Washer_Data[10] = { 0 };

#define DISPLAY_DELAY_MS 500

#define WASHER_CNT_MIN (g_Loop_Cnt / 60) 	// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_S (g_Loop_Cnt / 10) 		// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_100MS (g_Loop_Cnt / 1)	// g_Loop_Cnt与分钟的转换

#define LED_GPIO_RCC RCC_APB2Periph_GPIOA
#define LED_GPIO_x GPIOA
#define LED_GPIO_PIN_R GPIO_Pin_3
#define LED_GPIO_PIN_B GPIO_Pin_2

typedef enum {
	LED_RED = 0,
	LED_BLUE,
} LED_TYPE;

typedef enum { WASHER_STATUS_ENUM(ENUM_ITEM) } WASHER_STATUS;
const char* Washer_Status[] = { WASHER_STATUS_ENUM(ENUM_STRING) };

typedef enum { WASH_STATUS_ENUM(ENUM_ITEM) } WASH_STATUS;
const char* Wash_Status[] = { WASH_STATUS_ENUM(ENUM_STRING) };

typedef enum { SPIN_DRY_STATUS_ENUM(ENUM_ITEM) } SPIN_DRY_STATUS;
const char* Spin_Dry_Status[] = { SPIN_DRY_STATUS_ENUM(ENUM_STRING) };

static WASHER_STATUS g_Status_Next = S_INIT;
static WASHER_STATUS g_Status_Cur = S_INIT;
static WASHER_STATUS g_Status_Last = S_INIT;

static Washer* g_Washer;	// 指向当前的洗衣模式
static Washer_Errors g_Washer_Error_Cur;	// 当前错误状态
static Washer_Errors g_Washer_Error_Last;	// 上次错误状态

static uint32_t g_Loop_Cnt = 0;	// 循环计数器
static uint32_t g_Wash_Cnt_Cur = 0;	// 当前洗衣次数
static uint8_t g_Security_Monitor_On = 1;	// 安全监测开关
static uint8_t g_OLED_Need_Refresh = 0;	// 刷新OLED标志位

int8_t Washer_StartStop_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Washer_Quit_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Washer_Power_Event(void); // 临时这样解决一下 warning，后续再优化
void Washer_Save(void); // 记录当前状态变化，防止意外断电


//--------------------------------------------
typedef struct
{
	uint8_t StartStop; 	//启停键
	uint8_t Quit;		//结束键
	uint8_t Power;		//电源键
} Washer_Keys;
Washer_Keys g_Washer_Keys;

static uint8_t Is_Key_Active(uint8_t* Key)
{
	if (!(*Key)) return 0;
	*Key = 0;
	return 1;
}

static int8_t Washer_StartStop_Event(void)
{
	return Is_Key_Active(&g_Washer_Keys.StartStop);
}

static int8_t Washer_Quit_Event(void)
{
	return Is_Key_Active(&g_Washer_Keys.Quit);
}

static int8_t Washer_Power_Event(void)
{
	return Is_Key_Active(&g_Washer_Keys.Power);
}

static void Key_StartStop_Active(void)
{
	g_Washer_Keys.StartStop = 1;
}

static void Key_Quit_Active(void)
{
	g_Washer_Keys.Quit = 1;
}

static void Key_Power_Active(void)
{
	g_Washer_Keys.Power = 1;
}

static void Washer_Key_Init(void)
{
	KEY_Device* pDev_KeyEncoder = 0;
	KEY_Device* pDev_KeyPower = 0;
	pDev_KeyEncoder = Drv_Key_GetDevice(KEY_ENCODER);
	if (pDev_KeyEncoder)
	{
		pDev_KeyEncoder->CBRegister_R(pDev_KeyEncoder, Key_StartStop_Active);
		pDev_KeyEncoder->CBRegister_LP(pDev_KeyEncoder, Key_Quit_Active);
	}
	pDev_KeyPower = Drv_Key_GetDevice(KEY_POWER);
	if (pDev_KeyPower)
	{
		pDev_KeyPower->CBRegister_R(pDev_KeyPower, Key_Power_Active);
	}
}

static void Washer_Key_DeInit(void)
{
	KEY_Device* pDev_KeyEncoder = 0;
	KEY_Device* pDev_KeyPower = 0;
	pDev_KeyEncoder = Drv_Key_GetDevice(KEY_ENCODER);
	if (pDev_KeyEncoder)
	{
		pDev_KeyEncoder->CBUnregister_R(pDev_KeyEncoder, Key_StartStop_Active);
		pDev_KeyEncoder->CBUnregister_LP(pDev_KeyEncoder, Key_Quit_Active);
	}
	pDev_KeyPower = Drv_Key_GetDevice(KEY_POWER);
	if (pDev_KeyPower)
	{
		pDev_KeyPower->CBUnregister_R(pDev_KeyPower, Key_Power_Active);
	}
}
//--------------------------------------------


void Washer_LED_On(int8_t on, LED_TYPE type)
{
	if (on) GPIO_SetBits(GPIOA, type == LED_RED ? LED_GPIO_PIN_R : LED_GPIO_PIN_B);
	else GPIO_ResetBits(GPIOA, type == LED_RED ? LED_GPIO_PIN_R : LED_GPIO_PIN_B);
}

void Washer_LED_Revert(LED_TYPE type)
{
	if (GPIO_ReadInputDataBit(LED_GPIO_x, type == LED_RED ? LED_GPIO_PIN_R : LED_GPIO_PIN_B))
		GPIO_ResetBits(LED_GPIO_x, type == LED_RED ? LED_GPIO_PIN_R : LED_GPIO_PIN_B);
	else
		GPIO_SetBits(LED_GPIO_x, type == LED_RED ? LED_GPIO_PIN_R : LED_GPIO_PIN_B);
}

void Washer_Door_UnLock()
{
	g_pDev_ServoMotor->SetAngle(g_pDev_ServoMotor, 0);
}

void Washer_Door_Lock()
{
	g_pDev_ServoMotor->SetAngle(g_pDev_ServoMotor, 50);
}

void Washer_OLED_Refresh()
{
	OLED_Clear();
	OLED_ShowString_Easy(4, 1, "Wash Count[ / ]");
	OLED_ShowNum_Easy(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_ShowNum_Easy(4, 14, g_Washer->Wash_Cnt, 1);
}

void Washer_Init(Washer* pWasher)
{
	if (!pWasher)
	{
		g_Status_Next = S_QUIT;
		return; // 失败，直接退出
	}

	g_Washer = pWasher;
	g_Washer_Error_Cur = NO_ERROR;
	g_Status_Last = S_INIT;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;
	g_Security_Monitor_On = 1;  // 启动安全监测
	g_OLED_Need_Refresh = 1;	// 刷新OLED

	// 显示初始化信息
	Washer_OLED_Refresh();
	OLED_ShowString_Easy(1, 1, "Init...        ");

	// 初始化按键回调
	Washer_Key_Init();

	// 初始化DHT11
	Drv_DHT11_Init();

	// 初始化电机驱动
	Drv_TB6612_Init();
	g_pDev_TB6612 = Drv_TB6612_GetDevice(TB6612);

	// 初始化ADC, 用于检测门是否打开
	Drv_AD_Init();
	g_pDev_TCRT5000 = Drv_AD_GetDevice(AD_TCRT5000);

	// 初始化舵机，用于锁门
	Drv_ServoMotor_Init();
	g_pDev_ServoMotor = Drv_ServoMotor_GetDevice(SERVOMOTOR);
	Washer_Door_Lock(); // 锁门

	// 初始化MPU6050，用于姿态检测
	Drv_MPU6050_Init();
	g_pDev_MPU6050 = Drv_MPU6050_GetDevice(MPU6050);

	// 初始化 LED
	RCC_APB2PeriphClockCmd(LED_GPIO_RCC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN_R | LED_GPIO_PIN_B;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_GPIO_x, &GPIO_InitStructure);

	// 初始化蜂鸣器
	Drv_Buzzer_Init();
	g_pDev_Buzzer = Drv_Buzzer_GetDevice(BUZZER);

	// 初始化W25Q64
	Drv_W25Q64_Init();
	g_pDev_W25Q64 = Drv_W25Q64_GetDevice(W25Q64);

	g_pDev_W25Q64->ReadData(g_pDev_W25Q64, 0x000000, g_Washer_Data, 10);
	if (g_Washer_Data[9] == ACCIDENT_SHUTDOWN)
	{
		g_Washer->Wash_Cnt = g_Washer_Data[0];
		g_Washer->Wash_Time = g_Washer_Data[1];
		g_Washer->Spin_Dry_Time = g_Washer_Data[2];
		g_Washer->Water_Volume = g_Washer_Data[3];
		g_Washer->Water_Temp = g_Washer_Data[4];
		g_Washer->Heat_Temp = g_Washer_Data[5];
		g_Status_Next = (WASHER_STATUS)g_Washer_Data[6];
		g_Status_Cur = (WASHER_STATUS)g_Washer_Data[7];
		g_Status_Last = (WASHER_STATUS)g_Washer_Data[8];

		return;
	}

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

void Washer_Stop(uint8_t Custom_Shout_Down)
{
	g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
	Washer_Door_UnLock();
	g_pDev_Buzzer->Off(g_pDev_Buzzer);
	Washer_LED_On(0, LED_RED);
	Washer_LED_On(0, LED_BLUE);
	if (Custom_Shout_Down)
	{
		g_Washer_Data[9] = CUSTOMER_SHUTDOWN;
		g_pDev_W25Q64->SectorErase(g_pDev_W25Q64, 0x000000);
		g_pDev_W25Q64->PageProgram(g_pDev_W25Q64, 0x000000, g_Washer_Data, 10);
	}

	Delay_ms(100);
}

void Washer_Pause()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_Printf_Easy(3, 1, "pause...");
		Washer_Stop(0);
	}

	if (Washer_StartStop_Event())	// 继续
	{
		Washer_Door_Lock(); // 门锁上锁
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
		switch (g_Washer_Error_Cur)
		{
		case ERROR_TILT:
			OLED_Printf_Easy(2, 1, "Washer Tilt!");
			break;
		case ERROR_SHAKE:
			OLED_Printf_Easy(2, 1, "Washer Shake!");
			break;
		case ERROR_DOOR_OPEN:
			OLED_Printf_Easy(2, 1, "Close The Door!");
			break;
		default:
			break;
		}
		Washer_Stop(0);
	}

	Washer_LED_Revert(LED_RED);
	g_pDev_Buzzer->Revert(g_pDev_Buzzer);
	if (g_Washer_Error_Cur == NO_ERROR) //异常解除
	{
		Washer_LED_On(0, LED_RED);
		g_pDev_Buzzer->Off(g_pDev_Buzzer);

		Delay_ms(500);
		Washer_OLED_Refresh();
		OLED_Printf_Easy(1, 1, "ERROR Fixed!");
		Delay_ms(500);
		Washer_Door_Lock(); // 门锁上锁
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

		Washer_LED_On(1, LED_RED);
	}

	//接收温度和湿度的数据
	DHT11_HumiTemp DHT11_Data;
	DHT11_Device* pDev_DHT11 = Drv_DHT11_GetDevice(DHT11);
	if (pDev_DHT11)
		DHT11_Data = pDev_DHT11->Get_HumiTemp(pDev_DHT11);

	// 读取响应需要时间，延时500ms
	Delay_ms(500);

	// 显示新数值
	OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Temp, 2);
	OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Temp_Dec, 2);

	if (DHT11_Data.Temp >= g_Washer->Water_Temp) //烧水结束
	{
		Washer_LED_On(0, LED_RED);
		OLED_ShowString_Easy(1, 1, "heat water[DONE]");

		g_Loop_Cnt = 0;
		g_Status_Next = S_ADD_WATER;

		Delay_ms(DISPLAY_DELAY_MS);
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
		Washer_LED_On(1, LED_BLUE);
	}

	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_Washer->Water_Volume) //加水结束（1s加一升）
	{
		Washer_LED_On(0, LED_BLUE);
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "add water[DONE]");
		Washer_LED_On(0, LED_RED);
		Delay_ms(DISPLAY_DELAY_MS);

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
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (Wash_Loop_Cnt >= 6) //600ms
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_UP_15;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 15);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_UP_40;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_LEFT_SPEED_DOWN_15;
		}
		break;
	case S_WASH_LEFT_SPEED_DOWN_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 15);

		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_TURN_RIGHT;
		}
		break;
	case S_WASH_TURN_RIGHT:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_UP_15;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -15);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_UP_40;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_Status_Cur = S_WASH_RIGHT_SPEED_DOWN_15;
		}
		break;
	case S_WASH_RIGHT_SPEED_DOWN_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -15);
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
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		g_Status_Next = S_DRAIN_WATER;

		Delay_ms(DISPLAY_DELAY_MS);
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
	Washer_LED_Revert(LED_BLUE);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_Washer->Water_Volume) //排水结束（1s排一升）
	{
		Washer_LED_On(0, LED_BLUE);
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "draing[DONE]");
		Delay_ms(DISPLAY_DELAY_MS);

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
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_20;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_20:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 20);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_40;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_UP_60;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_60:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 60);
		OLED_ShowNum_Easy(1, 11, WASHER_CNT_MIN, 2);
		if ((WASHER_CNT_MIN) >= g_Washer->Spin_Dry_Time)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_DOWN_40;
		}
		break;
	case S_SPIN_LEFT_SPEED_DOWN_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_Status_Cur = S_SPIN_LEFT_SPEED_DOWN_20;
		}
	case S_SPIN_LEFT_SPEED_DOWN_20:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 20);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
			g_Status_Next = S_WASH_CNT;

			OLED_ShowString_Easy(2, 1, "spin dry[DONE]"); //甩干结束
			Delay_ms(DISPLAY_DELAY_MS);
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

		Washer_LED_On(1, LED_RED);
	}

	//接收温度和湿度的数据
	DHT11_HumiTemp DHT11_Data;
	DHT11_Device* pDev_DHT11 = Drv_DHT11_GetDevice(DHT11);
	if (pDev_DHT11)
		DHT11_Data = pDev_DHT11->Get_HumiTemp(pDev_DHT11);

	// 读取响应需要时间，延时500ms
	Delay_ms(500);

	// 显示新数值
	OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Humi, 2);
	OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Humi_Dec, 2);

	if (DHT11_Data.Humi < Dry_Relative_Humidity) //烘干结束
	{
		g_Loop_Cnt = 0;
		g_Status_Next = S_FINISH;

		Washer_LED_On(0, LED_RED);
		OLED_ShowString_Easy(1, 1, "heat dry[DONE]");
		Delay_ms(DISPLAY_DELAY_MS);
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
	Delay_ms(DISPLAY_DELAY_MS);
}

void Washer_Finish()
{
	if (g_OLED_Need_Refresh)
	{
		OLED_Clear();
		OLED_ShowString_Easy(1, 1, "WASH FINISH!");
		g_Security_Monitor_On = 0; // 关闭安全监测
	}

	if (Washer_StartStop_Event())
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

		case S_WASH_CNT:
			Washer_Wash_Cnt();
			break;

		case S_FINISH:
			Washer_Finish();
			break;

		case S_QUIT:
			Washer_Stop(1);
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

			Washer_Save();
		}

		// 安全异常监测
		if (g_Security_Monitor_On)
		{
			g_Washer_Error_Cur = NO_ERROR;

			// 检测门是否打开
			if (g_pDev_TCRT5000->GetValue(g_pDev_TCRT5000) > 400) // 大于1.5cm距离
			{
				g_Washer_Error_Cur = ERROR_DOOR_OPEN;
			}

			// 检测姿态是否倾斜
			static int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
			static int16_t AccX_Abs, AccY_Abs;
			static uint8_t Shake_Time = 0;
			g_pDev_MPU6050->GetData(g_pDev_MPU6050, &AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
			AccX_Abs = AccX > 0 ? AccX : -AccX;
			AccY_Abs = AccY > 0 ? AccY : -AccY;
			if (AccX_Abs > 100 || AccY_Abs > 50) // 瞬时加速度大于50
			{
				Shake_Time++;
				if (Shake_Time == 2)	g_Washer_Error_Cur = ERROR_SHAKE; // 持续200ms
				else if (Shake_Time > 2) g_Washer_Error_Cur = ERROR_TILT; // 持续时间大于200ms
			}
			else
			{
				Shake_Time = 0;
			}

			// 异常处理
			if (g_Washer_Error_Cur != NO_ERROR && g_Status_Cur != S_ERROR && g_Status_Cur != S_WASH_CNT)
			{
				g_Status_Next = S_ERROR;
				g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
				if (g_Status_Cur != S_PAUSE) // 异常解除后，直接继续原状态机，取消暂停
				{
					g_Status_Last = g_Status_Cur; // 保存异常前的状态机
				}
				g_Status_Cur = S_ERROR;

				Washer_Save();
			}

			if (g_Washer_Error_Cur != NO_ERROR && g_Washer_Error_Cur != g_Washer_Error_Last)
			{
				g_OLED_Need_Refresh = 1; // ERROR 状态机里发生状态切换，刷新OLED
				g_Washer_Error_Last = g_Washer_Error_Cur;
			}
		}

		// 暂停捕获，ERROR 状态下不暂停
		if (Washer_StartStop_Event() && g_Status_Cur != S_ERROR && g_Status_Cur != S_WASH_CNT)
		{
			g_Status_Next = S_PAUSE;
		}

		if (Washer_Power_Event())
		{
			Washer_Stop(1);
			Washer_Key_DeInit();
			Menu_Washer_Power_Off();
			return -1;
		}

		if (Washer_Quit_Event())
		{
			Washer_Stop(1);
			Washer_Key_DeInit();
			return -1;
		}

		// 轮询周期为延时100ms
		Delay_ms(100);
	}
}

void Washer_Save(void)
{
	// 记录当前状态变化，防止意外断电
	g_Washer_Data[0] = g_Washer->Wash_Cnt;
	g_Washer_Data[1] = g_Washer->Wash_Time;
	g_Washer_Data[2] = g_Washer->Spin_Dry_Time;
	g_Washer_Data[3] = g_Washer->Water_Volume;
	g_Washer_Data[4] = g_Washer->Water_Temp;
	g_Washer_Data[5] = g_Washer->Heat_Temp;
	g_Washer_Data[6] = g_Status_Next;
	g_Washer_Data[7] = g_Status_Cur;
	g_Washer_Data[8] = g_Status_Last;
	g_Washer_Data[9] = ACCIDENT_SHUTDOWN;
	g_pDev_W25Q64->SectorErase(g_pDev_W25Q64, 0x000000);
	g_pDev_W25Q64->PageProgram(g_pDev_W25Q64, 0x000000, g_Washer_Data, 10);
}
