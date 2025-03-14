#ifndef __DHT11_H
#define __DHT11_H


typedef struct
{
    uint8_t Humi;
    uint8_t Humi_Dec;
    uint8_t Temp;
    uint8_t Temp_Dec;
    uint8_t TimeOut;
} DHT11_HumiTemp;

typedef enum
{
    DHT11,
    DHT11_NUM,
} DHT11_ID;

typedef struct DHT11_Device {
    DHT11_HumiTemp(*Get_HumiTemp)(struct DHT11_Device* pDev);
    void* Priv_Data;
} DHT11_Device;

DHT11_Device* Drv_DHT11_GetDevice(DHT11_ID ID);

void Drv_DHT11_Init(void);


#endif
