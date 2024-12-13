#include "stm32f10x.h"                  // Device header
#include  "DHT11.h"
#include  "Delay.h"

void DH11_GPIO_Init_Out(void)
{
	RCC_APB2PeriphClockCmd(DHT11_GPIO_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = DHT11_GOIO_MODE_OUT;
	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN_DATA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(DHT11_GOIO_x, &GPIO_InitStructure);
}

void DH11_GPIO_Init_In(void)
{
	RCC_APB2PeriphClockCmd(DHT11_GPIO_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = DHT11_GOIO_MODE_IN;
	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN_DATA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(DHT11_GOIO_x, &GPIO_InitStructure);
}

//主机向DHT11发送开始指令
void DHT11_Start(void)
{
	DH11_GPIO_Init_Out();	// 输出模式，准备发送开始指令

	DHT11_HIGH;				// 先拉高
	Delay_us(30);

	DHT11_LOW;				// 拉低电平至少18us
	Delay_ms(20);

	DHT11_HIGH;				// 拉高电平20~40us
	Delay_us(30);

	DH11_GPIO_Init_In();	// 输入模式，准备接收HDT11返回的数据
}

uint8_t DHT11_Recive_Byte(void)
{
	uint8_t Data;

	for (uint8_t i = 0;i < 8;i++)	//1个数据就是1个字节byte，1个字节byte有8位bit
	{
		while (Read_Data == 0);		//从1bit开始，低电平变高电平，等待低电平结束
		Delay_us(30);				//延迟30us是为了区别数据0和数据1，0只有26~28us

		Data <<= 1;					//左移

		if (Read_Data == 1)			//如果过了30us还是高电平的话就是数据1
		{
			Data |= 1;				//数据+1
		}

		while (Read_Data == 1);		//高电平变低电平，等待高电平结束
	}

	return Data;
}

void DHT11_Recive_Data(DHT11_Data_t* Out)
{
	uint8_t CHECK;

	DHT11_Start();		// 主机向DHT11发送开始指令
	DHT11_HIGH; 		// 拉高电平

	if (Read_Data == 0)	// 判断DHT11是否响应
	{
		while (Read_Data == 0); // 低电平变高电平，等待低电平结束
		while (Read_Data == 1); // 高电平变低电平，等待高电平结束

		//接收数据及校验位
		Out->Humi = DHT11_Recive_Byte();
		Out->Humi_Dec = DHT11_Recive_Byte();
		Out->Temp = DHT11_Recive_Byte();
		Out->Temp_Dec = DHT11_Recive_Byte();
		CHECK = DHT11_Recive_Byte();

		DHT11_LOW;		// 当最后一bit数据传送完毕后，DHT11拉低总线 50us
		Delay_us(55);	// 这里延时55us
		DHT11_HIGH; 	// 随后总线由上拉电阻拉高进入空闲状态。

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

