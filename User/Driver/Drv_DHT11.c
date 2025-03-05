#include "stm32f10x.h"                  // Device header
#include "Sys_Delay.h"
#include "Drv_DHT11.h"


#define DHT11_HIGH(PORT, PIN) GPIO_SetBits(PORT, PIN)
#define DHT11_LOW(PORT, PIN) GPIO_ResetBits(PORT, PIN)
#define DHT11_READ(PORT, PIN) GPIO_ReadInputDataBit(PORT, PIN)

typedef struct
{
	DHT11_ID ID;
	GPIO_TypeDef* PORT;
	uint16_t PIN;
	GPIOMode_TypeDef MODE_IN;	// 输入模式
	GPIOMode_TypeDef MODE_OUT;	// 输出模式
	uint32_t _RCC;
} DHT11_HARDWARE;

typedef struct {
	DHT11_ID ID;
	DHT11_HARDWARE HW;
} DHT11_Data;


DHT11_HumiTemp Get_HumiTemp(DHT11_Device* pDev);
void DHT11_Recive_Data(DHT11_Device* pDev, DHT11_HumiTemp* Out);


//--------------------------------------------------


static DHT11_HARDWARE g_DHT11_HWs[DHT11_NUM] = {
	{
		.ID = DHT11,
		.PORT = GPIOB,
		.PIN = GPIO_Pin_6,
		.MODE_IN = GPIO_Mode_IN_FLOATING,
		.MODE_OUT = GPIO_Mode_Out_PP,
		._RCC = RCC_APB2Periph_GPIOB,
	},
};
static DHT11_Data g_DHT11_Datas[DHT11_NUM];
static DHT11_Device g_DHT11_Devs[DHT11_NUM];


//--------------------------------------------------


DHT11_Device* Drv_DHT11_GetDevice(DHT11_ID ID)
{
	for (int i = 0; i < sizeof(g_DHT11_Devs) / sizeof(g_DHT11_Devs[0]); i++)
	{
		DHT11_Data* pData = (DHT11_Data*)g_DHT11_Devs[i].Priv_Data;
		if (pData == 0)
			return 0;
		if (pData->ID == ID)
			return &g_DHT11_Devs[i];
	}
	return 0;
}

void Drv_DHT11_Init(void)
{
	for (uint8_t i = 0; i < DHT11_NUM; i++)
	{
		// Get Hardware
		DHT11_HARDWARE hw;
		for (uint8_t j = 0; j < sizeof(g_DHT11_HWs) / sizeof(g_DHT11_HWs[0]); j++)
		{
			if (g_DHT11_HWs[j].ID == (DHT11_ID)i)
				hw = g_DHT11_HWs[j];
		}

		// Data Init
		g_DHT11_Datas[i].ID = (DHT11_ID)i;
		g_DHT11_Datas[i].HW = hw;

		// Device Init
		g_DHT11_Devs[i].Get_HumiTemp = Get_HumiTemp;
		g_DHT11_Devs[i].Priv_Data = (void*)&g_DHT11_Datas[i];

		Get_HumiTemp(&g_DHT11_Devs[i]);	// 第一次读取数据，初始化GPIO
	}
	Delay_ms(500);	// 读取响应需要时间，延时500ms
}


//--------------------------------------------------


void DHT11_GPIO_Init_Out(DHT11_Device* pDev)
{
	DHT11_Data* pData = (DHT11_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	RCC_APB2PeriphClockCmd(pData->HW._RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = pData->HW.MODE_OUT;
	GPIO_InitStructure.GPIO_Pin = pData->HW.PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(pData->HW.PORT, &GPIO_InitStructure);
}

void DHT11_GPIO_Init_In(DHT11_Device* pDev)
{
	DHT11_Data* pData = (DHT11_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	RCC_APB2PeriphClockCmd(pData->HW._RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = pData->HW.MODE_IN;
	GPIO_InitStructure.GPIO_Pin = pData->HW.PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(pData->HW.PORT, &GPIO_InitStructure);
}

//主机向DHT11发送开始指令
void DHT11_Start(DHT11_Device* pDev)
{
	DHT11_Data* pData = (DHT11_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	GPIO_TypeDef* port = pData->HW.PORT;
	uint16_t pin = pData->HW.PIN;

	DHT11_GPIO_Init_Out(pDev);	// 输出模式，准备发送开始指令=

	DHT11_HIGH(port, pin);	// 先拉高
	Delay_us(30);

	DHT11_LOW(port, pin);	// 拉低电平至少18us
	Delay_ms(20);

	DHT11_HIGH(port, pin);	// 拉高电平20~40us
	Delay_us(30);

	DHT11_GPIO_Init_In(pDev);	// 输入模式，准备接收HDT11返回的数据
}

uint8_t DHT11_Recive_Byte(GPIO_TypeDef* port, uint16_t pin)
{
	uint8_t ReciveData;

	for (uint8_t i = 0; i < 8; i++)				//1个数据就是1个字节byte，1个字节byte有8位bit
	{
		while (DHT11_READ(port, pin) == 0);		//从1bit开始，低电平变高电平，等待低电平结束
		Delay_us(30);							//延迟30us是为了区别数据0和数据1，0只有26~28us

		ReciveData <<= 1;						//左移

		if (DHT11_READ(port, pin) == 1)			//如果过了30us还是高电平的话就是数据1
		{
			ReciveData |= 1;					//数据+1
		}

		while (DHT11_READ(port, pin) == 1);		//高电平变低电平，等待高电平结束
	}

	return ReciveData;
}

void DHT11_Recive_Data(DHT11_Device* pDev, DHT11_HumiTemp* Out)
{
	DHT11_Data* pData = (DHT11_Data*)pDev->Priv_Data;
	if (pData == 0) return;

	GPIO_TypeDef* port = pData->HW.PORT;
	uint16_t pin = pData->HW.PIN;

	uint8_t CHECK;

	DHT11_Start(pDev);			// 主机向DHT11发送开始指令
	DHT11_HIGH(port, pin); 		// 拉高电平

	if (DHT11_READ(port, pin) == 0)	// 判断DHT11是否响应
	{
		while (DHT11_READ(port, pin) == 0); // 低电平变高电平，等待低电平结束
		while (DHT11_READ(port, pin) == 1); // 高电平变低电平，等待高电平结束

		//接收数据及校验位
		Out->Humi = DHT11_Recive_Byte(port, pin);
		Out->Humi_Dec = DHT11_Recive_Byte(port, pin);
		Out->Temp = DHT11_Recive_Byte(port, pin);
		Out->Temp_Dec = DHT11_Recive_Byte(port, pin);
		CHECK = DHT11_Recive_Byte(port, pin);

		DHT11_LOW(port, pin);	// 当最后一bit数据传送完毕后，DHT11拉低总线 50us
		Delay_us(55);			// 这里延时55us
		DHT11_HIGH(port, pin); 	// 随后总线由上拉电阻拉高进入空闲状态。

		//和检验位对比，判断校验接收到的数据是否正确，如果不正确则返回0
		if (Out->Humi + Out->Humi_Dec + Out->Temp + Out->Temp_Dec != CHECK)
		{
			Out->Humi = 0;
			Out->Humi_Dec = 0;
			Out->Temp = 0;
			Out->Temp_Dec = 0;
		}
	}
}

DHT11_HumiTemp Get_HumiTemp(DHT11_Device* pDev)
{
	DHT11_HumiTemp Out;
	DHT11_Recive_Data(pDev, &Out);
	return Out;
}

