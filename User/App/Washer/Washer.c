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

int8_t Washer_StartStop_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Washer_Quit_Event(void); // 临时这样解决一下 warning，后续再优化
int8_t Washer_Power_Event(void); // 临时这样解决一下 warning，后续再优化


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
	OLED_ShowString_Easy(4, 1, "Wash Count[ / ]");
	OLED_ShowNum_Easy(4, 12, g_Wash_Cnt_Cur, 1);
	OLED_ShowNum_Easy(4, 14, g_pWDat->Wash_Cnt, 1);
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

void Washer_Task_DeInit(void)
{
	vTaskSuspendAll();

	// if (*Get_Task_Washer_Stop_Handle())
	// 	vTaskSuspend(*Get_Task_Washer_Stop_Handle());

	if (*Get_Task_Washer_Key_Handle())
		vTaskSuspend(*Get_Task_Washer_Key_Handle());

	if (*Get_Task_Washer_Safety_Handle())
		vTaskSuspend(*Get_Task_Washer_Safety_Handle());

	// if (*Get_Task_Washer_Pause_Handle())
	// 	vTaskSuspend(*Get_Task_Washer_Pause_Handle());

	if (*Get_Task_Washer_Error_Handle())
		vTaskSuspend(*Get_Task_Washer_Error_Handle());

	if (*Get_Task_Washer_Run_Handle())
		vTaskSuspend(*Get_Task_Washer_Run_Handle());

	xTaskResumeAll();
}

void Washer_Init(void)
{
	g_Washer_Error_Cur = NO_ERROR;
	g_Loop_Cnt = 0;
	g_Wash_Cnt_Cur = 0;
	//g_Security_Monitor_On = 1;  // 启动安全监测
	g_OLED_Need_Refresh = 1;	// 刷新OLED

	// 显示初始化信息
	Washer_OLED_Refresh();
	OLED_ShowString_Easy(1, 1, "Init...        ");

	g_pDev_TB6612 = Drv_TB6612_GetDevice(TB6612); // 电机驱动
	g_pDev_TCRT5000 = Drv_AD_GetDevice(AD_TCRT5000); // ADC, 用于检测门是否打开
	g_pDev_ServoMotor = Drv_ServoMotor_GetDevice(SERVOMOTOR); // 舵机，用于锁门
	g_pDev_MPU6050 = Drv_MPU6050_GetDevice(MPU6050); // MPU6050，用于姿态检测
	g_pDev_Buzzer = Drv_Buzzer_GetDevice(BUZZER); // 化蜂鸣器，用于报警
	g_pDev_LED_RED = Drv_LED_GetDevice(LED_RED); // 红灯
	g_pDev_LED_BLUE = Drv_LED_GetDevice(LED_BLUE); // 蓝灯

	Washer_Key_Init(); // 初始化按键回调
	Washer_Door_Lock(); // 锁门

	Washer_Task_Init(); // 初始化任务

	g_pWDat->Status_Next = S_HEAT_WATER;
	g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
}

void Washer_DeInit(void)
{
	Washer_Key_DeInit(); // 注销按键回调
	Washer_Task_DeInit(); // 挂起任务
}

void Washer_Custom_Shutdown()
{
	TASK_WASHER_DATA_INIT;
	g_pWDat->Shutdown_Type = CUSTOMER_SHUTDOWN;
	TASK_WASHER_DATA_STORE;
}

void Washer_Key()
{
	if (Washer_StartStop_Event())
	{
		printf("Washer_StartStop_Event\r\n");
		if (g_pWDat->Status_Cur == S_FINISH)
		{
			//TODO: 完成洗衣，返回菜单
			Washer_Stop();
			Washer_Custom_Shutdown();
			Menu_Washer_Init();
			Washer_DeInit();
		}
		else if (g_pWDat->Status_Cur == S_PAUSE)
		{
			//恢复洗衣
			TASK_WASHER_PAUSE_CONTINUE;
			g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
		}
		else
		{
			//暂停洗衣
			TASK_WASHER_PAUSE_PAUSE;
		}
	}

	if (Washer_Power_Event())
	{
		printf("Washer_Power_Event\r\n");
		Washer_Stop();
		Washer_Custom_Shutdown();
		Menu_Washer_Power_Off();
		Washer_DeInit();
	}

	if (Washer_Quit_Event())
	{
		printf("Washer_Quit_Event\r\n");
		Washer_Stop();
		Menu_Washer_Init();
		Washer_DeInit();
	}
}

void Washer_Stop()
{
	g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
	Washer_Door_UnLock();
	g_pDev_Buzzer->Off_Buzzer(g_pDev_Buzzer);
	g_pDev_LED_RED->Off_LED(g_pDev_LED_RED);
	g_pDev_LED_BLUE->Off_LED(g_pDev_LED_BLUE);
}

void Washer_Pause()
{
	if (g_pWDat->Status_Cur == S_ERROR)
		return;
	Washer_Stop();
	Washer_OLED_Refresh();
	OLED_Printf_Easy(3, 1, "pause...");
	g_pWDat->Status_Cur = S_PAUSE;
	TASK_WASHER_DATA_STORE;
	if (*Get_Task_Washer_Run_Handle())
		vTaskSuspend(*Get_Task_Washer_Run_Handle());
}

void Washer_Resume()
{
	Washer_Door_Lock(); // 门锁上锁
	Washer_OLED_Refresh();
	g_pWDat->Status_Cur = g_pWDat->Status_Next;
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
	}
}

void Washer_Error_Warning()
{
	//蜂鸣器与指示灯报警
	g_pDev_LED_RED->Revert_LED(g_pDev_LED_RED);
	g_pDev_Buzzer->Revert_Buzzer(g_pDev_Buzzer);
	Delay_ms(500);
}

void Washer_Error_Fixed()
{
	g_pDev_LED_RED->Off_LED(g_pDev_LED_RED);
	g_pDev_Buzzer->Off_Buzzer(g_pDev_Buzzer);

	Washer_OLED_Refresh();
	OLED_Printf_Easy(1, 1, "ERROR Fixed!");
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
	vTaskSuspendAll();
	if (g_pDev_TCRT5000->GetValue(g_pDev_TCRT5000) > 400) // 大于1.5cm距离
	{
		g_Washer_Error_Cur = ERROR_DOOR_OPEN;
		printf("Washer_Error_DOOR_OPEN\r\n");
	}
	xTaskResumeAll();

	// 检测姿态是否倾斜
	static int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	static int16_t AccX_Abs, AccY_Abs;
	static uint8_t Shake_Time = 0;
	vTaskSuspendAll();
	// STM32的硬件I2C容易死机，死机后会一直Timeout，此时断一次电即可，不断电复位是不管用的
	g_pDev_MPU6050->GetData(g_pDev_MPU6050, &AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
	xTaskResumeAll();
	AccX_Abs = AccX > 0 ? AccX : -AccX;
	AccY_Abs = AccY > 0 ? AccY : -AccY;
	if (AccX_Abs > 100 || AccY_Abs > 50) // 瞬时加速度大于阈值
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
			g_pWDat->Status_Cur = g_pWDat->Status_Next;
			TASK_WASHER_ERROR_FIXED;
		}
		printf("NO_ERROR\r\n");
		return;
	}

	// 异常发生，同步错误信息
	if (g_Washer_Error_Last != g_Washer_Error_Cur)
	{
		g_pWDat->Status_Cur = S_ERROR;
		g_OLED_Need_Refresh = 1;
		// printf("g_OLED_Need_Refresh\r\n");
		g_Washer_Error_Last = g_Washer_Error_Cur;
		// 异常发生，报警
		TASK_WASHER_ERROR_OCCUR;
		if (*Get_Task_Washer_Error_Handle())
			vTaskResume(*Get_Task_Washer_Error_Handle());
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
		OLED_ShowNum_Easy(3, 8, (uint32_t)g_pWDat->Water_Temp, 2);
		OLED_ShowString_Easy(3, 10, ".00");
		OLED_ShowChinese_Easy(3, 13 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		g_pDev_LED_RED->On_LED(g_pDev_LED_RED);
	}

	//接收温度和湿度的数据
	DHT11_HumiTemp DHT11_Data;
	DHT11_Device* pDev_DHT11 = Drv_DHT11_GetDevice(DHT11);

	vTaskSuspendAll();
	if (pDev_DHT11)
		DHT11_Data = pDev_DHT11->Get_HumiTemp(pDev_DHT11);
	xTaskResumeAll();

	// 读取响应需要时间，延时500ms
	Delay_ms(500);

	// Delay期间可能触发暂停，暂停后恢复需要刷新OLED
	if (g_OLED_Need_Refresh)
	{
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "heat water...");

		OLED_ShowString_Easy(2, 1, "Temp:");	// 温度
		OLED_ShowString_Easy(2, 8, ".");
		OLED_ShowChinese_Easy(2, 11 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		OLED_ShowString_Easy(3, 1, "Target:");	// 目标温度
		OLED_ShowNum_Easy(3, 8, (uint32_t)g_pWDat->Water_Temp, 2);
		OLED_ShowString_Easy(3, 10, ".00");
		OLED_ShowChinese_Easy(3, 13 / 2 + 1, "℃");	   // 中文是16x16的，所以要减半，并向上取整

		g_pDev_LED_RED->On_LED(g_pDev_LED_RED);
	}

	// 显示新数值
	OLED_ShowNum_Easy(2, 6, (uint32_t)DHT11_Data.Temp, 2);
	OLED_ShowNum_Easy(2, 9, (uint32_t)DHT11_Data.Temp_Dec, 2);

	if (DHT11_Data.Temp >= g_pWDat->Water_Temp) //烧水结束
	{
		g_pDev_LED_RED->Off_LED(g_pDev_LED_RED);
		OLED_ShowString_Easy(1, 1, "heat water[DONE]");

		g_Loop_Cnt = 0;
		g_pWDat->Status_Next = S_ADD_WATER;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED

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
		OLED_ShowNum_Easy(1, 14, g_pWDat->Water_Volume, 2);
		g_pDev_LED_BLUE->On_LED(g_pDev_LED_BLUE);
	}

	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_pWDat->Water_Volume) //加水结束（1s加一升）
	{
		g_pDev_LED_BLUE->Off_LED(g_pDev_LED_BLUE);
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "add water[DONE]");
		g_pDev_LED_RED->Off_LED(g_pDev_LED_RED);
		Delay_ms(DISPLAY_DELAY_MS);

		g_Loop_Cnt = 0;
		g_pWDat->Status_Next = S_WASH;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
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
		OLED_ShowNum_Easy(1, 14, g_pWDat->Wash_Time, 2);
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
	if (WASHER_CNT_MIN >= g_pWDat->Wash_Time) //清洗结束
	{
		OLED_ShowNum_Easy(1, 11, WASHER_CNT_MIN, 2);
		OLED_ShowString_Easy(2, 1, "washing[DONE]");

		g_Loop_Cnt = 0;
		Wash_Loop_Cnt = 0;
		g_pDev_TB6612->SetSpeed(g_pDev_TB6612, 0);
		g_pWDat->Status_Next = S_DRAIN_WATER;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED

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
		OLED_ShowNum_Easy(1, 14, g_pWDat->Water_Volume, 2);
	}

	OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
	g_pDev_LED_BLUE->Revert_LED(g_pDev_LED_BLUE);

	g_Loop_Cnt++;
	if (WASHER_CNT_S >= g_pWDat->Water_Volume) //排水结束（1s排一升）
	{
		g_pDev_LED_BLUE->Off_LED(g_pDev_LED_BLUE);
		OLED_ShowNum_Easy(1, 11, (g_Loop_Cnt / 10), 2);
		OLED_ShowString_Easy(2, 1, "draing[DONE]");
		Delay_ms(DISPLAY_DELAY_MS);

		g_Loop_Cnt = 0;
		g_pWDat->Status_Next = S_SPIN_DRY;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
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
		OLED_ShowNum_Easy(1, 14, g_pWDat->Spin_Dry_Time, 2);
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
		if ((WASHER_CNT_MIN) >= g_pWDat->Spin_Dry_Time)
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
			g_pWDat->Status_Next = S_WASH_CNT;
			g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED

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
	static uint8_t Dry_Relative_Humidity = 30;		//目标湿度
	if (g_OLED_Need_Refresh)
	{
		g_OLED_Need_Refresh = 0;
		Washer_OLED_Refresh();
		OLED_ShowString_Easy(1, 1, "heat dry...");

		OLED_ShowString_Easy(2, 1, "Humi:");	//湿度
		OLED_ShowString_Easy(2, 8, ".");
		OLED_ShowChinese_Easy(2, 11 / 2 + 1, "％");	   //中文是16x16的，所以要减半，并向上取整

		OLED_ShowString_Easy(3, 1, "Target:");	// 目标湿度
		OLED_ShowNum_Easy(3, 8, Dry_Relative_Humidity, 2);
		OLED_ShowString_Easy(3, 10, ".00");
		OLED_ShowChinese_Easy(3, 13 / 2 + 1, "％");	   //中文是16x16的，所以要减半，并向上取整

		g_pDev_LED_RED->On_LED(g_pDev_LED_RED);
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
		g_pWDat->Status_Next = S_FINISH;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED

		g_pDev_LED_RED->Off_LED(g_pDev_LED_RED);
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
	OLED_ShowNum_Easy(4, 14, g_pWDat->Wash_Cnt, 1);

	if (g_Wash_Cnt_Cur >= g_pWDat->Wash_Cnt)
	{
		g_pWDat->Status_Next = S_FINISH;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
	}
	else
	{
		OLED_ShowString_Easy(2, 1, "Start next wash.");
		g_pWDat->Status_Next = S_HEAT_WATER;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED
	}
	Delay_ms(DISPLAY_DELAY_MS);
}

void Washer_Finish()
{
	if (g_OLED_Need_Refresh)
	{
		OLED_Clear();
		OLED_ShowString_Easy(1, 1, "WASH FINISH!");

		// 关闭安全监测
		if (*Get_Task_Washer_Safety_Handle())
			vTaskSuspend(*Get_Task_Washer_Safety_Handle());

		if (*Get_Task_Washer_Pause_Handle())
			vTaskSuspend(*Get_Task_Washer_Pause_Handle());

		if (*Get_Task_Washer_Error_Handle())
			vTaskSuspend(*Get_Task_Washer_Error_Handle());
	}
}

// 状态机运行函数
void Washer_Run(void)
{
	switch (g_pWDat->Status_Next)
	{
	case S_INIT:
		Washer_Init();
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

	default:
		Washer_Init();
		break;
	}

	// 状态机更新
	if (g_pWDat->Status_Cur != g_pWDat->Status_Next &&
		g_pWDat->Status_Cur != S_PAUSE)
	{
		g_pWDat->Status_Last = g_pWDat->Status_Cur;
		g_pWDat->Status_Cur = g_pWDat->Status_Next;
		g_OLED_Need_Refresh = 1; // 状态切换，刷新OLED

		TASK_WASHER_DATA_STORE;
	}

	// 轮询周期为延时100ms
	Delay_ms(100);
}
