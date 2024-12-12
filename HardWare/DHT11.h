#ifndef __DHT11_H
#define __DHT11_H

typedef struct
{
    uint8_t Humi;
    uint8_t Humi_Dec;
    uint8_t Temp;
    uint8_t Temp_Dec;
} DHT11_Data_t;

//GPIO
#define DHT11_GPIO_RCC          RCC_APB2Periph_GPIOB
#define DHT11_GOIO_x            GPIOB
#define DHT11_GPIO_PIN_DATA     GPIO_Pin_7
#define DHT11_GOIO_MODE_OUT     GPIO_Mode_Out_PP        //推挽输出
#define DHT11_GOIO_MODE_IN      GPIO_Mode_IN_FLOATING   //浮空输入

#define DHT11_HIGH GPIO_SetBits(DHT11_GOIO_x, DHT11_GPIO_PIN_DATA)
#define DHT11_LOW GPIO_ResetBits(DHT11_GOIO_x, DHT11_GPIO_PIN_DATA)
#define Read_Data GPIO_ReadInputDataBit(DHT11_GOIO_x, DHT11_GPIO_PIN_DATA)

void DHT11_GPIO_Init_Out(void);
void DHT11_GPIO_Init_IN(void);
void DHT11_Start(void);
uint8_t DHT11_Recive_Byte(void);
void DHT11_Recive_Data(DHT11_Data_t* Out);



#endif

