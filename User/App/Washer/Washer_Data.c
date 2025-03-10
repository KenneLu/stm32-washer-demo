#include "stm32f10x.h"
#include <stdio.h>
#include "Washer_Data.h"
#include "Drv_W25Q64.h"


#define STORE_ADDRESS 0x000000
#define DATA_LEN 12

static Washer_Data g_Washer_Data;
pWasher_Data g_pWDat = &g_Washer_Data;

uint8_t Data_Init(Washer_Data** pData);
uint8_t DataStore(Washer_Data* pData);
uint8_t DataRestore(Washer_Data** pData);


void Drv_Washer_Data_Init(void)
{
    Data_Init(&g_pWDat);
    g_Washer_Data.DataInit = Data_Init;
    g_Washer_Data.DataStore = DataStore;
    g_Washer_Data.DataRestore = DataRestore;
}

uint8_t Data_Init(Washer_Data** pData)
{
    if (pData == 0 || (*pData) == 0)
        return 0;

    (*pData)->Mode = M_FAST_WASH;
    (*pData)->Wash_Cnt = 0;
    (*pData)->Wash_Time = 0;
    (*pData)->Spin_Dry_Time = 0;
    (*pData)->Water_Volume = 0;
    (*pData)->Water_Temp = 0;
    (*pData)->Heat_Temp = 0;
    (*pData)->Total_Time = 0;
    (*pData)->Shutdown_Type = ACCIDENT_SHUTDOWN;
    (*pData)->State_Next = S_INIT;
    (*pData)->State_Cur = S_INIT;
    (*pData)->State_Last = S_INIT;
    printf("Data_Init success\r\n");

    return 1;
}

uint8_t DataStore(Washer_Data* pData)
{
    if (pData == 0)
        return 0;

    uint8_t data[DATA_LEN] = { 0 };
    data[0] = pData->Mode;
    data[1] = pData->Wash_Cnt;
    data[2] = pData->Wash_Time;
    data[3] = pData->Spin_Dry_Time;
    data[4] = pData->Water_Volume;
    data[5] = pData->Water_Temp;
    data[6] = pData->Heat_Temp;
    data[7] = pData->Total_Time;
    data[8] = pData->Shutdown_Type;
    data[9] = pData->State_Next;
    data[10] = pData->State_Cur;
    data[11] = pData->State_Last;

    W25Q64_Device* pDev_W25Q64 = Drv_W25Q64_GetDevice(W25Q64);
    if (pDev_W25Q64 == 0)
        return 0;

    pDev_W25Q64->SectorErase(pDev_W25Q64, STORE_ADDRESS);
    pDev_W25Q64->PageProgram(pDev_W25Q64, STORE_ADDRESS, data, DATA_LEN);
    printf("DataStore success\r\n");

    return 1;
}

uint8_t DataRestore(Washer_Data** pData)
{
    if (pData == 0 || (*pData) == 0)
        return 0;

    W25Q64_Device* pDev_W25Q64 = Drv_W25Q64_GetDevice(W25Q64);
    if (pDev_W25Q64 == 0)
        return 0;

    uint8_t data[DATA_LEN] = { 0 };
    pDev_W25Q64->ReadData(pDev_W25Q64, STORE_ADDRESS, data, DATA_LEN);

    (*pData)->Mode = (WASHER_MODE)data[0];
    (*pData)->Wash_Cnt = data[1];
    (*pData)->Wash_Time = data[2];
    (*pData)->Spin_Dry_Time = data[3];
    (*pData)->Water_Volume = data[4];
    (*pData)->Water_Temp = data[5];
    (*pData)->Heat_Temp = data[6];
    (*pData)->Total_Time = data[7];
    (*pData)->Shutdown_Type = (SHUTDOWN_TYPE)data[8];
    (*pData)->State_Next = (WASHER_STATE)data[9];
    (*pData)->State_Cur = (WASHER_STATE)data[10];
    (*pData)->State_Last = (WASHER_STATE)data[11];
    printf("DataRestore success\r\n");

    return 1;
}
