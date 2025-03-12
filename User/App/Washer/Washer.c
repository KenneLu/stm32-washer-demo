#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "task.h"

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
#include "Drv_LED.h"

#include "Menu.h"
#include "Washer.h"
#include "Washer_Data.h"
#include "Task_Manager.h"


static BUZZER_Device* g_pDev_Buzzer;
static MPU6050_Device* g_pDev_MPU6050;
static AD_Device* g_pDev_TCRT5000;
static SERVOMOTOR_Device* g_pDev_ServoMotor;
static TB6612_Device* g_pDev_TB6612;
static LED_Device* g_pDev_LED_RED;
static LED_Device* g_pDev_LED_BLUE;

#define DISPLAY_DELAY_MS 500

#define WASHER_CNT_MIN (g_Loop_Cnt / 60) 	// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_S (g_Loop_Cnt / 10) 		// g_Loop_Cnt与分钟的转换
#define WASHER_CNT_100MS (g_Loop_Cnt / 1)	// g_Loop_Cnt与分钟的转换

#define LED_GPIO_RCC RCC_APB2Periph_GPIOA
#define LED_GPIO_x GPIOA
#define LED_GPIO_PIN_R GPIO_Pin_3
#define LED_GPIO_PIN_B GPIO_Pin_2



// static Washer* g_pWDat;	// 指向当前的洗衣模式
static Washer_Errors g_Washer_Error_Cur = NO_ERROR;		// 当前错误状态
static Washer_Errors g_Washer_Error_Last = NO_ERROR;	// 上次错误状态

static uint32_t g_Loop_Cnt = 0;	// 循环计数器
static uint32_t g_Wash_Cnt_Cur = 0;	// 当前洗衣次数
static uint8_t g_OLED_Need_Refresh = 0;	// 刷新OLED标志位

int8_t Washer_StartStop_Event(void);
int8_t Washer_Quit_Event(void);
int8_t Washer_Power_Event(void);
void Washer_State_Refresh(void);


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
	OLED_SHOW_STR_E(4, 1, "Wash Count[ / ]");
	OLED_SHOW_NUM_E(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_SHOW_NUM_E(4, 14, g_pWDat->Wash_Cnt, 1);
}

void Washer_Task_Init(void)
{
	vTaskSuspendAll();

	if (*Get_Task_Washer_Stop_Handle() == 0)
	{
		Do_Create_Task_Washer_Stop();
		vTaskSuspend(*Get_Task_Washer_Stop_Handle());
	}

	if (*Get_Task_Washer_Key_Handle() == 0)
		Do_Create_Task_Washer_Key();
	else
		vTaskResume(*Get_Task_Washer_Key_Handle());

	if (*Get_Task_Washer_Safety_Handle() == 0)
		Do_Create_Task_Washer_Safety();
	else
		vTaskResume(*Get_Task_Washer_Safety_Handle());

	if (*Get_Task_Washer_Pause_Handle() == 0)
	{
		Do_Create_Task_Washer_Pause();
		vTaskResume(*Get_Task_Washer_Pause_Handle());
	}

	if (*Get_Task_Washer_Error_Handle() == 0)
	{
		Do_Create_Task_Washer_Error();
		vTaskSuspend(*Get_Task_Washer_Error_Handle());	// 挂起任务
	}

	if (*Get_Task_Washer_Run_Handle() == 0)
		Do_Create_Task_Washer_Run();
	else
		vTaskResume(*Get_Task_Washer_Run_Handle());

	xTaskResumeAll();
}

void Washer_Back_To_Menu(void)
{
	Washer_Stop();
	Washer_Key_DeInit(); // 注销按键回调

	vTaskSuspendAll();
	// Task Suspend
	if (*Get_Task_Washer_Safety_Handle())	// Washer_Safety
		vTaskSuspend(*Get_Task_Washer_Safety_Handle());
	if (*Get_Task_Washer_Error_Handle())	// Washer_Error
		vTaskSuspend(*Get_Task_Washer_Error_Handle());
	if (*Get_Task_Washer_Run_Handle())		// Washer_Run
	{
		vTaskDelete(*Get_Task_Washer_Run_Handle());
		*Get_Task_Washer_Run_Handle() = 0;
	}
	xTaskResumeAll();

	g_pWDat->State_Next = S_INIT;
	g_pWDat->Shutdown_Type = CUSTOMER_SHUTDOWN;
	TASK_WASHER_DATA_STORE;
	vTaskDelay(10);

	vTaskSuspendAll();
	// Task Suspend
	vTaskResume(*Get_Task_MainMenu_Handle());
	if (*Get_Task_Washer_Key_Handle())		// Washer_Key
		vTaskSuspend(*Get_Task_Washer_Key_Handle());
	xTaskResumeAll();
}

void Washer_Shutdown(void)
{
	Washer_Stop();
	Washer_Key_DeInit(); // 注销按键回调

	vTaskSuspendAll();
	// Task Delete
	if (*Get_Task_Washer_Stop_Handle())		// Washer_Stop
		vTaskDelete(*Get_Task_Washer_Stop_Handle());
	if (*Get_Task_Washer_Safety_Handle())	// Washer_Safety
		vTaskDelete(*Get_Task_Washer_Safety_Handle());
	if (*Get_Task_Washer_Pause_Handle())	// Washer_Pause
		vTaskDelete(*Get_Task_Washer_Pause_Handle());
	if (*Get_Task_Washer_Error_Handle())	// Washer_Error
		vTaskDelete(*Get_Task_Washer_Error_Handle());
	if (*Get_Task_Washer_Run_Handle())		// Washer_Run
		vTaskDelete(*Get_Task_Washer_Run_Handle());
	xTaskResumeAll();

	Menu_Washer_Power_Off();

	// Task Delete
	if (*Get_Task_Washer_Key_Handle())		// Washer_Key
		vTaskDelete(*Get_Task_Washer_Key_Handle());
}

void Washer_Key()
{
	if (Washer_StartStop_Event())
	{
		printf("Washer_StartStop_Event\r\n");
		if (g_pWDat->State_Cur == S_FINISH)
		{
			//完成洗衣，返回菜单
			Washer_Back_To_Menu();
		}
		else if (g_pWDat->State_Cur == S_PAUSE)
		{
			//恢复键按下，恢复洗衣
			TASK_WASHER_PAUSE_CONTINUE;
			g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
		}
		else
		{
			//暂停键按下，暂停洗衣
			TASK_WASHER_PAUSE_PAUSE;
		}
	}

	if (Washer_Quit_Event())
	{
		//退出键按下，返回菜单洗衣机
		printf("Washer_Quit_Event\r\n");
		Washer_Back_To_Menu();
	}

	if (Washer_Power_Event())
	{
		//电源键按下，关闭洗衣机
		printf("Washer_Power_Event\r\n");
		Washer_Shutdown();
	}
}

void Washer_Stop()
{
	g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
	Washer_Door_UnLock();
	g_pDev_Buzzer->Off(g_pDev_Buzzer);
	g_pDev_LED_RED->Off(g_pDev_LED_RED);
	g_pDev_LED_BLUE->Off(g_pDev_LED_BLUE);
}

void Washer_Pause()
{
	if (g_pWDat->State_Cur == S_ERROR)
		return;
	Washer_Stop();
	Washer_OLED_Refresh();
	OLED_SHOW_STR_E(3, 1, "pause...");
	g_pWDat->State_Cur = S_PAUSE;
	TASK_WASHER_DATA_STORE;
	if (*Get_Task_Washer_Run_Handle())
		vTaskSuspend(*Get_Task_Washer_Run_Handle());
}

void Washer_Resume()
{
	Washer_Door_Lock(); // 门锁上锁
	Washer_OLED_Refresh();
	g_pWDat->State_Cur = g_pWDat->State_Next;
	g_OLED_Need_Refresh = 1;
	if (*Get_Task_Washer_Run_Handle())
		vTaskResume(*Get_Task_Washer_Run_Handle());
}

void Washer_Error_Occur()
{
	if (g_OLED_Need_Refresh)
	{
		Washer_Stop();
		g_OLED_Need_Refresh = 0; // 必须先立即清零，否则后面更新的ERROR无法显示

		//刷新报错信息
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "ERROR!!!");
		switch (g_Washer_Error_Cur)
		{
		case ERROR_TILT:
			OLED_SHOW_STR_E(2, 1, "Washer Tilt!");
			break;
		case ERROR_SHAKE:
			OLED_SHOW_STR_E(2, 1, "Washer Shake!");
			break;
		case ERROR_DOOR_OPEN:
			OLED_SHOW_STR_E(2, 1, "Close The Door!");
			break;
		default:
			break;
		}
	}
}

void Washer_Error_Warning()
{
	//蜂鸣器与指示灯报警
	g_pDev_LED_RED->Revert(g_pDev_LED_RED);
	g_pDev_Buzzer->Revert(g_pDev_Buzzer);
	Delay_ms(500);
}

void Washer_Error_Fixed()
{
	g_OLED_Need_Refresh = 1;

	g_pDev_LED_RED->Off(g_pDev_LED_RED);
	g_pDev_Buzzer->Off(g_pDev_Buzzer);

	Washer_OLED_Refresh();
	OLED_SHOW_STR_E(1, 1, "ERROR Fixed!");
	Delay_ms(500);
	TASK_WASHER_PAUSE_PAUSE;
	if (*Get_Task_Washer_Error_Handle())
		vTaskSuspend(*Get_Task_Washer_Error_Handle());
}

void Washer_Safety(void)
{
	// 安全异常监测
	g_Washer_Error_Cur = NO_ERROR;

	// 检测门是否打开
	if (g_pDev_TCRT5000->GetValue(g_pDev_TCRT5000) > 400) // 大于1.5cm距离
	{
		g_Washer_Error_Cur = ERROR_DOOR_OPEN;
		printf("Washer_Error_DOOR_OPEN\r\n");
	}

	// 检测姿态是否倾斜
	static int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	static int16_t AccX_Abs, AccY_Abs;
	static uint8_t Shake_Time = 0;

	/*
	* FreeRTOS 下严禁在此处挂起调度器，
	* 否则 g_pDev_MPU6050->GetData 加锁时，
	* 挂起调度器会导致互斥锁无法进行优先级继承，导致死锁
	*/
	// STM32的硬件I2C容易死机，死机后会一直Timeout，需要整机掉电重启，或者换用软件I2C
	g_pDev_MPU6050->GetData(g_pDev_MPU6050, &AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);

	AccX_Abs = AccX > 0 ? AccX : -AccX;
	AccY_Abs = AccY > 0 ? AccY : -AccY;
	if (AccX_Abs > 150 || AccY_Abs > 50) // 瞬时加速度大于阈值
	{
		Shake_Time++; // 100ms轮询检测一次
		if (Shake_Time == 2) // 持续200ms
		{
			g_Washer_Error_Cur = ERROR_SHAKE;
			printf("Washer_Error_SHAKE\r\n");
		}
		else if (Shake_Time > 2) // 持续时间大于200ms
		{
			g_Washer_Error_Cur = ERROR_TILT;
			printf("Washer_Error_TILT\r\n");
		}
		printf("%d,   %d\r\n", AccX_Abs, AccY_Abs);
	}
	else
	{
		Shake_Time = 0;
	}

	if (g_Washer_Error_Cur == NO_ERROR) // 无异常
	{
		if (g_Washer_Error_Last != NO_ERROR) // 异常解除
		{
			g_Washer_Error_Last = NO_ERROR;
			g_pWDat->State_Cur = g_pWDat->State_Next;
			TASK_WASHER_ERROR_FIXED;
		}
		// printf("NO_ERROR\r\n");
		return;
	}

	// 异常发生，同步错误信息
	if (g_Washer_Error_Last != g_Washer_Error_Cur)
	{
		g_pWDat->State_Cur = S_ERROR;
		g_OLED_Need_Refresh = 1;
		// printf("g_OLED_Need_Refresh\r\n");
		g_Washer_Error_Last = g_Washer_Error_Cur;
		// 异常发生，报警
		if (*Get_Task_Washer_Error_Handle())
			vTaskResume(*Get_Task_Washer_Error_Handle());
		TASK_WASHER_ERROR_OCCUR;
	}

}

void Washer_Init(void)
{
	g_Washer_Error_Cur = NO_ERROR;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;

	// 显示初始化信息
	Washer_OLED_Refresh();
	OLED_SHOW_STR_E(1, 1, "Init...        ");

	g_pDev_TB6612 = Drv_TB6612_GetDevice(TB6612); // 电机驱动
	g_pDev_TCRT5000 = Drv_AD_GetDevice(AD_TCRT5000); // ADC, 用于检测门是否打开
	g_pDev_ServoMotor = Drv_ServoMotor_GetDevice(SERVOMOTOR); // 舵机，用于锁门
	g_pDev_MPU6050 = Drv_MPU6050_GetDevice(MPU6050); // MPU6050，用于姿态检测
	g_pDev_Buzzer = Drv_Buzzer_GetDevice(BUZZER); // 化蜂鸣器，用于报警
	g_pDev_LED_RED = Drv_LED_GetDevice(LED_RED); // 红灯
	g_pDev_LED_BLUE = Drv_LED_GetDevice(LED_BLUE); // 蓝灯

	Washer_Key_Init(); // 初始化按键回调
	Washer_Door_Lock(); // 锁门

	if (g_pWDat->Mode == M_SPIN_DRY)
	{
		g_pWDat->State_Next = S_DRAIN_WATER;
	}
	else if (g_pWDat->Mode == M_HEAT_DRY)
	{
		g_pWDat->State_Next = S_HEAT_DRY;
	}
	else // M_FAST_WASH, M_STANDARD_WASH, M_HARD_WASH
	{
		g_pWDat->State_Next = S_HEAT_WATER;
	}
	Washer_State_Refresh(); // 状态切换，刷新OLED

	Washer_Task_Init(); // 初始化任务
}

void Washer_Heat_Water()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "heat water...");

		OLED_SHOW_STR_E(2, 1, "Temp:");	// 温度
		OLED_SHOW_STR_E(2, 8, ".");
		OLED_SHOW_CN_E(2, 11 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		OLED_SHOW_STR_E(3, 1, "Target:");	// 目标温度
		OLED_SHOW_NUM_E(3, 8, (uint32_t)g_pWDat->Water_Temp, 2);
		OLED_SHOW_STR_E(3, 10, ".00");
		OLED_SHOW_CN_E(3, 13 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		g_pDev_LED_RED->On(g_pDev_LED_RED);
	}

	//接收温度和湿度的数据
	DHT11_HumiTemp DHT11_Data;
	DHT11_Device* pDev_DHT11 = Drv_DHT11_GetDevice(DHT11);

	if (pDev_DHT11)
		DHT11_Data = pDev_DHT11->Get_HumiTemp(pDev_DHT11);

	if (DHT11_Data.TimeOut)
	{
		printf("DHT11 TimeOut!\r\n");
	}
	else
	{
		// 读取响应需要时间，延时500ms
		Delay_ms(500);

		// 显示新数值
		OLED_SHOW_NUM_E(2, 6, (uint32_t)DHT11_Data.Temp, 2);
		OLED_SHOW_NUM_E(2, 9, (uint32_t)DHT11_Data.Temp_Dec, 2);

		if (DHT11_Data.Temp >= g_pWDat->Water_Temp) //烧水结束
		{
			g_pDev_LED_RED->Off(g_pDev_LED_RED);
			OLED_SHOW_STR_E(1, 1, "heat water[DONE]");

			g_Loop_Cnt = 0;
			g_pWDat->State_Next = S_ADD_WATER;
			Washer_State_Refresh(); // 状态切换，刷新OLED

			Delay_ms(DISPLAY_DELAY_MS);
		}
	}
}

void Washer_Add_Water()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "add water[  /  ]");
		OLED_SHOW_NUM_E(1, 14, g_pWDat->Water_Volume, 2);
		g_pDev_LED_BLUE->On(g_pDev_LED_BLUE);
	}

	OLED_SHOW_NUM_E(1, 11, (g_Loop_Cnt / 10), 2);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_pWDat->Water_Volume) //加水结束（1s加一升）
	{
		g_pDev_LED_BLUE->Off(g_pDev_LED_BLUE);
		OLED_SHOW_NUM_E(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_SHOW_STR_E(2, 1, "add water[DONE]");
		g_pDev_LED_RED->Off(g_pDev_LED_RED);
		Delay_ms(DISPLAY_DELAY_MS);

		g_Loop_Cnt = 0;
		g_pWDat->State_Next = S_WASH;
		Washer_State_Refresh(); // 状态切换，刷新OLED
	}
}

void Washer_Wash()
{
	static WASH_STATE Wash_State_Cur;
	static uint32_t Wash_Loop_Cnt;

	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "washing..[  /  ]");
		OLED_SHOW_NUM_E(1, 14, g_pWDat->Wash_Time, 2);
		Wash_State_Cur = S_WASH_TURN_LEFT;
		Wash_Loop_Cnt = 0;
	}

	OLED_SHOW_NUM_E(1, 11, WASHER_CNT_MIN, 2);

	switch (Wash_State_Cur)
	{
	case S_WASH_TURN_LEFT:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (Wash_Loop_Cnt >= 6) //600ms
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_LEFT_SPEED_UP_15;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 15);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_LEFT_SPEED_UP_40;
		}
		break;
	case S_WASH_LEFT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_LEFT_SPEED_DOWN_15;
		}
		break;
	case S_WASH_LEFT_SPEED_DOWN_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 15);

		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_TURN_RIGHT;
		}
		break;
	case S_WASH_TURN_RIGHT:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_RIGHT_SPEED_UP_15;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -15);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_RIGHT_SPEED_UP_40;
		}
		break;
	case S_WASH_RIGHT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -40);
		if (Wash_Loop_Cnt >= 8)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_RIGHT_SPEED_DOWN_15;
		}
		break;
	case S_WASH_RIGHT_SPEED_DOWN_15:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, -15);
		if (Wash_Loop_Cnt >= 6)
		{
			Wash_Loop_Cnt = 0;
			Wash_State_Cur = S_WASH_TURN_LEFT;
		}
		break;

	default:
		break;
	}

	g_Loop_Cnt++;
	Wash_Loop_Cnt++;
	if (WASHER_CNT_MIN >= g_pWDat->Wash_Time) //清洗结束
	{
		OLED_SHOW_NUM_E(1, 11, WASHER_CNT_MIN, 2);
		OLED_SHOW_STR_E(2, 1, "washing[DONE]");

		g_Loop_Cnt = 0;
		Wash_Loop_Cnt = 0;
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		g_pWDat->State_Next = S_DRAIN_WATER;
		Washer_State_Refresh(); // 状态切换，刷新OLED

		Delay_ms(DISPLAY_DELAY_MS);
	}
}

void Washer_Drain_Water()
{
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "draing...[  /  ]");
		OLED_SHOW_NUM_E(1, 14, g_pWDat->Water_Volume, 2);
	}

	OLED_SHOW_NUM_E(1, 11, (g_Loop_Cnt / 10), 2);
	g_pDev_LED_BLUE->Revert(g_pDev_LED_BLUE);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_pWDat->Water_Volume) //排水结束（1s排一升）
	{
		g_pDev_LED_BLUE->Off(g_pDev_LED_BLUE);
		OLED_SHOW_NUM_E(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_SHOW_STR_E(2, 1, "draing[DONE]");
		Delay_ms(DISPLAY_DELAY_MS);

		g_Loop_Cnt = 0;
		g_pWDat->State_Next = S_SPIN_DRY;
		Washer_State_Refresh(); // 状态切换，刷新OLED
	}

}

void Washer_Spin_Dry()
{
	static SPIN_DRY_STATE Spin_Dry_State_Cur;

	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "spin dry.[00/  ]");
		OLED_SHOW_NUM_E(1, 14, g_pWDat->Spin_Dry_Time, 2);
		Spin_Dry_State_Cur = S_SPIN_STOP;
		g_Loop_Cnt = 0;
	}

	switch (Spin_Dry_State_Cur)
	{
	case S_SPIN_STOP:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_State_Cur = S_SPIN_LEFT_SPEED_UP_20;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_20:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 20);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_State_Cur = S_SPIN_LEFT_SPEED_UP_40;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (WASHER_CNT_S >= 3)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_State_Cur = S_SPIN_LEFT_SPEED_UP_60;
		}
		break;
	case S_SPIN_LEFT_SPEED_UP_60:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 60);
		OLED_SHOW_NUM_E(1, 11, WASHER_CNT_MIN, 2);
		if ((WASHER_CNT_MIN) >= g_pWDat->Spin_Dry_Time)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_State_Cur = S_SPIN_LEFT_SPEED_DOWN_40;
		}
		break;
	case S_SPIN_LEFT_SPEED_DOWN_40:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 40);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			Spin_Dry_State_Cur = S_SPIN_LEFT_SPEED_DOWN_20;
		}
	case S_SPIN_LEFT_SPEED_DOWN_20:
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 20);
		if (WASHER_CNT_100MS >= 5)
		{
			g_Loop_Cnt = 0;
			g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
			g_pWDat->State_Next = S_WASH_CNT;
			Washer_State_Refresh(); // 状态切换，刷新OLED

			OLED_SHOW_STR_E(2, 1, "spin dry[DONE]"); //甩干结束
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
	static uint8_t Dry_Relative_Humidity = 30;		//目标湿度
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_SHOW_STR_E(1, 1, "heat dry...");

		OLED_SHOW_STR_E(2, 1, "Humi:");	//湿度
		OLED_SHOW_STR_E(2, 8, ".");
		OLED_SHOW_CN_E(2, 11 / 2 + 1, "％");	   //中文是16x16的，所以要减半，并向上取整

		OLED_SHOW_STR_E(3, 1, "Target:");	// 目标湿度
		OLED_SHOW_NUM_E(3, 8, Dry_Relative_Humidity, 2);
		OLED_SHOW_STR_E(3, 10, ".00");
		OLED_SHOW_CN_E(3, 13 / 2 + 1, "％");	   //中文是16x16的，所以要减半，并向上取整

		g_pDev_LED_RED->On(g_pDev_LED_RED);
	}

	//接收温度和湿度的数据
	DHT11_HumiTemp DHT11_Data;
	DHT11_Device* pDev_DHT11 = Drv_DHT11_GetDevice(DHT11);
	if (pDev_DHT11)
		DHT11_Data = pDev_DHT11->Get_HumiTemp(pDev_DHT11);
	if (DHT11_Data.TimeOut)
	{
		printf("DHT11 TimeOut!\r\n");
	}
	else
	{
		// 读取响应需要时间，延时500ms
		Delay_ms(1000);

		// 显示新数值
		OLED_SHOW_NUM_E(2, 6, (uint32_t)DHT11_Data.Humi, 2);
		OLED_SHOW_NUM_E(2, 9, (uint32_t)DHT11_Data.Humi_Dec, 2);

		if (DHT11_Data.Humi < Dry_Relative_Humidity) //烘干结束
		{
			g_Loop_Cnt = 0;
			g_pWDat->State_Next = S_FINISH;
			Washer_State_Refresh(); // 状态切换，刷新OLED

			g_pDev_LED_RED->Off(g_pDev_LED_RED);
			OLED_SHOW_STR_E(1, 1, "heat dry[DONE]");
			Delay_ms(DISPLAY_DELAY_MS);
		}
	}
}

void Washer_Wash_Cnt()
{
	g_Loop_Cnt = 0;

	Washer_OLED_Refresh();

	//完成一次清洗, 记录次数
	g_Wash_Cnt_Cur++;
	OLED_SHOW_STR_E(1, 1, "This round over.");
	OLED_SHOW_STR_E(4, 1, "Wash Count[ / ]");
	OLED_SHOW_NUM_E(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_SHOW_NUM_E(4, 14, g_pWDat->Wash_Cnt, 1);

	if (g_Wash_Cnt_Cur >= g_pWDat->Wash_Cnt)
	{
		g_pWDat->State_Next = S_FINISH;
		Washer_State_Refresh(); // 状态切换，刷新OLED
	}
	else
	{
		OLED_SHOW_STR_E(2, 1, "Start next wash.");
		g_pWDat->State_Next = S_HEAT_WATER;
		Washer_State_Refresh(); // 状态切换，刷新OLED
	}
	Delay_ms(DISPLAY_DELAY_MS);
}

void Washer_Finish()
{
	if (g_OLED_Need_Refresh)
	{
		OLED_Clear();
		OLED_SHOW_STR_E(1, 1, "WASH FINISH!");

		// 关闭安全监测
		if (*Get_Task_Washer_Safety_Handle())
			vTaskSuspend(*Get_Task_Washer_Safety_Handle());

		if (*Get_Task_Washer_Pause_Handle())
			vTaskSuspend(*Get_Task_Washer_Pause_Handle());

		if (*Get_Task_Washer_Error_Handle())
			vTaskSuspend(*Get_Task_Washer_Error_Handle());
	}
}

void Washer_State_Refresh(void)
{
	// 状态机更新
	printf("Washer State: %d -> %d\r\n", g_pWDat->State_Cur, g_pWDat->State_Next);
	g_pWDat->State_Last = g_pWDat->State_Cur;
	g_pWDat->State_Cur = g_pWDat->State_Next;
	g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
	TASK_WASHER_DATA_STORE;
}

// 状态机运行函数
void Washer_State_Machine(void)
{
	printf("Washer_State_Machine is Running\r\n");
	switch (g_pWDat->State_Next)
	{
	case S_INIT:
		printf("S_INIT\r\n");
		Washer_Init();
		break;

	case S_HEAT_WATER:
		printf("S_HEAT_WATER\r\n");
		Washer_Heat_Water();
		break;

	case S_ADD_WATER:
		printf("S_ADD_WATER\r\n");
		Washer_Add_Water();
		break;

	case S_WASH:
		printf("S_WASH\r\n");
		Washer_Wash();
		break;

	case S_DRAIN_WATER:
		printf("S_DRAIN_WATER\r\n");
		Washer_Drain_Water();
		break;

	case S_SPIN_DRY:
		printf("S_SPIN_DRY\r\n");
		Washer_Spin_Dry();
		break;

	case S_HEAT_DRY:
		printf("S_HEAT_DRY\r\n");
		Washer_Heat_Dry();
		break;

	case S_WASH_CNT:
		printf("S_WASH_CNT\r\n");
		Washer_Wash_Cnt();
		break;

	case S_FINISH:
		printf("S_FINISH\r\n");
		Washer_Finish();
		break;

	default:
		printf("Unknown State\r\n");
		Washer_Init();
		break;
	}

	// 轮询周期为延时100ms
	Delay_ms(100);
}
