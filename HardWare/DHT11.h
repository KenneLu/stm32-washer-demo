#ifndef __DHT11_H
#define __DHT11_H


typedef struct
{
    uint8_t Humi;
    uint8_t Humi_Dec;
    uint8_t Temp;
    uint8_t Temp_Dec;
} DHT11_HumiTemp;

typedef enum
{
    DHT11,
    DHT11_NUM,
} DHT11_ID;

typedef struct DHT11_Device {
    DHT11_HumiTemp(*DHT11_Get_HumiTemp)(struct DHT11_Device* pDev);
    void* Priv_Data;
} DHT11_Device;

DHT11_Device* GetDHT11Device(DHT11_ID ID);

void DHT11_Init(void);


#endif
