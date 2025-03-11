#include "stm32f10x.h"                  // Device header
#include "Sys_Delay.h"
#include "Drv_I2C_SW.h"


typedef struct
{
    I2C_SW_ID ID;
    GPIO_TypeDef* PORT;
    uint16_t PIN_SCL;
    uint16_t PIN_SDA;
    uint32_t _RCC;
} I2C_SW_HARDWARE;

typedef struct {
    I2C_SW_ID ID;
    I2C_SW_HARDWARE SW;
} I2C_SW_Data;

void W_SCL(I2C_SW_Data* pData, uint8_t BitValue);
void W_SDA(I2C_SW_Data* pData, uint8_t BitValue);
uint8_t R_SDA(I2C_SW_Data* pData);
void I2C_SW_Start(I2C_SW_Device* Dev);
void I2C_SW_Stop(I2C_SW_Device* Dev);
void I2C_SW_SendAck(I2C_SW_Device* Dev, uint8_t bNo);
void I2C_SW_SendByte(I2C_SW_Device* Dev, uint8_t Byte);
uint8_t I2C_SW_RecieveAck(I2C_SW_Device* Dev);
uint8_t I2C_SW_RecieveByte(I2C_SW_Device* Dev);
void I2C_SW_WriteI2C(I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t Data);
uint8_t I2C_SW_ReadI2C(I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t Addr);



//--------------------------------------------------


static I2C_SW_HARDWARE g_I2C_SW_SWs[I2C_SW_NUM] = {
    {
        .ID = I2C_SW,
        .PORT = GPIOB,
        .PIN_SCL = GPIO_Pin_8,
        .PIN_SDA = GPIO_Pin_9,
        ._RCC = RCC_APB2Periph_GPIOB,
    },
};
static I2C_SW_Data g_I2C_SW_Datas[I2C_SW_NUM];
static I2C_SW_Device g_I2C_SW_Devs[I2C_SW_NUM];


//--------------------------------------------------


I2C_SW_Device* Drv_I2C_SW_GetDevice(I2C_SW_ID ID)
{
    for (int i = 0; i < sizeof(g_I2C_SW_Devs) / sizeof(g_I2C_SW_Devs[0]); i++)
    {
        I2C_SW_Data* pData = (I2C_SW_Data*)g_I2C_SW_Devs[i].Priv_Data;
        if (pData == 0)
            return 0;
        if (pData->ID == ID)
            return &g_I2C_SW_Devs[i];
    }
    return 0;
}

void Drv_I2C_SW_Init(void)
{
    for (uint8_t i = 0; i < I2C_SW_NUM; i++)
    {
        // Get Hardware
        I2C_SW_HARDWARE hw;
        for (uint8_t j = 0; j < sizeof(g_I2C_SW_SWs) / sizeof(g_I2C_SW_SWs[0]); j++)
        {
            if (g_I2C_SW_SWs[j].ID == (I2C_SW_ID)i)
                hw = g_I2C_SW_SWs[j];
        }

        // Data Init
        g_I2C_SW_Datas[i].ID = (I2C_SW_ID)i;
        g_I2C_SW_Datas[i].SW = hw;

        // Device Init
        g_I2C_SW_Devs[i].Start = I2C_SW_Start;
        g_I2C_SW_Devs[i].Stop = I2C_SW_Stop;
        g_I2C_SW_Devs[i].SendAck = I2C_SW_SendAck;
        g_I2C_SW_Devs[i].SendByte = I2C_SW_SendByte;
        g_I2C_SW_Devs[i].RecieveAck = I2C_SW_RecieveAck;
        g_I2C_SW_Devs[i].RecieveByte = I2C_SW_RecieveByte;
        g_I2C_SW_Devs[i].WriteI2C = I2C_SW_WriteI2C;
        g_I2C_SW_Devs[i].ReadI2C = I2C_SW_ReadI2C;

        g_I2C_SW_Devs[i].Priv_Data = (void*)&g_I2C_SW_Datas[i];

        // Hardware Init
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;    // I2C均为开漏输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = hw.PIN_SCL;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = hw.PIN_SDA;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /*释放SCL和SDA*/
        W_SCL(&g_I2C_SW_Datas[i], 1);
        W_SDA(&g_I2C_SW_Datas[i], 1);
    }
}


//--------------------------------------------------


void W_SCL(I2C_SW_Data* pData, uint8_t BitValue)
{
    /*根据BitValue的值，将SCL置高电平或者低电平*/
    GPIO_WriteBit(pData->SW.PORT, pData->SW.PIN_SCL, (BitAction)BitValue);

    /*如果单片机速度过快，可在此添加适量延时，以避免超出I2C通信的最大速度*/
    //...Delay_us(10);
}

void W_SDA(I2C_SW_Data* pData, uint8_t BitValue)
{
    /*根据BitValue的值，将SDA置高电平或者低电平*/
    GPIO_WriteBit(pData->SW.PORT, pData->SW.PIN_SDA, (BitAction)BitValue);

    /*如果单片机速度过快，可在此添加适量延时，以避免超出I2C通信的最大速度*/
    //...Delay_us(10);
}

uint8_t R_SDA(I2C_SW_Data* pData)
{
    uint8_t BitValue = 0;
    BitValue = GPIO_ReadInputDataBit(pData->SW.PORT, pData->SW.PIN_SDA);
    /*如果单片机速度过快，可在此添加适量延时，以避免超出I2C通信的最大速度*/
    //...Delay_us(10);
    return BitValue;
}

void I2C_SW_Start(I2C_SW_Device* Dev)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return;

    W_SDA(pData, 1);		//释放SDA，确保SDA为高电平
    W_SCL(pData, 1);		//释放SCL，确保SCL为高电平
    W_SDA(pData, 0);		//在SCL高电平期间，拉低SDA，产生起始信号
    W_SCL(pData, 0);		//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

void I2C_SW_Stop(I2C_SW_Device* Dev)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return;

    W_SDA(pData, 0);		//拉低SDA，确保SDA为低电平
    W_SCL(pData, 1);		//释放SCL，使SCL呈现高电平
    W_SDA(pData, 1);		//在SCL高电平期间，释放SDA，产生终止信号
}

void I2C_SW_SendAck(I2C_SW_Device* Dev, uint8_t bNo)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return;

    W_SDA(pData, bNo);
    W_SCL(pData, 1);
    W_SCL(pData, 0);
}

uint8_t I2C_SW_RecieveAck(I2C_SW_Device* Dev)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return 0;

    W_SDA(pData, 1);
    W_SCL(pData, 1);
    uint8_t bNo = R_SDA(pData);
    W_SCL(pData, 0);
    return bNo;
}

void I2C_SW_SendByte(I2C_SW_Device* Dev, uint8_t Byte)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return;

    /*循环8次，主机依次发送数据的每一位*/
    for (uint8_t i = 0; i < 8; i++)
    {
        /*使用掩码的方式取出Byte的指定一位数据并写入到SDA线*/
        /*两个!的作用是，让所有非零的值变为1*/
        W_SDA(pData, !!(Byte & (0x80 >> i)));
        W_SCL(pData, 1);	//释放SCL，从机在SCL高电平期间读取SDA
        W_SCL(pData, 0);	//拉低SCL，主机开始发送下一位数据
    }

    I2C_SW_RecieveAck(Dev);
}

uint8_t I2C_SW_RecieveByte(I2C_SW_Device* Dev)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return 0;

    uint8_t Res = 0;
    W_SDA(pData, 1);
    for (uint8_t i = 0; i < 8; i++)
    {
        Res <<= 1; // 首次无需移位
        W_SCL(pData, 1);
        Res |= R_SDA(pData);
        W_SCL(pData, 0);
    }
    I2C_SW_SendAck(Dev, 1);
    return Res;
}

void I2C_SW_WriteI2C(I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t Data)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return;

    I2C_SW_Start(Dev);
    I2C_SW_SendByte(Dev, SlaveAddr | 0x00);  // 写为0
    I2C_SW_SendByte(Dev, RegAddr);
    I2C_SW_SendByte(Dev, Data);
    I2C_SW_Stop(Dev);
}

uint8_t I2C_SW_ReadI2C(I2C_SW_Device* Dev, uint8_t SlaveAddr, uint8_t Addr)
{
    I2C_SW_Data* pData = (I2C_SW_Data*)Dev->Priv_Data;
    if (pData == 0)
        return 0;

    uint8_t Data;
    I2C_SW_Start(Dev);
    I2C_SW_SendByte(Dev, SlaveAddr);
    I2C_SW_SendByte(Dev, Addr);
    I2C_SW_Start(Dev);
    I2C_SW_SendByte(Dev, SlaveAddr | 0x01); // 读为1
    Data = I2C_SW_RecieveByte(Dev);
    I2C_SW_Stop(Dev);
    return Data;
}
