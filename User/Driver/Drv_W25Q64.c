#include "stm32f10x.h"                  // Device header
#include "Drv_W25Q64_Ins.h"
#include "Drv_W25Q64.h"
#include "Drv_SPI.h"


typedef struct
{
	W25Q64_ID ID;
	SPI_DEVICE_ID SPI_ID;
	SPI_Device* pSPI;
} W25Q64_HARDWARE;

typedef struct {
	W25Q64_ID ID;
	W25Q64_HARDWARE HW;
} W25Q64_Data;

void ReadID(W25Q64_Device* pDev, uint8_t* MID, uint16_t* DID);
void ReadData(W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint32_t Count);
void PageProgram(W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint16_t Count);
void SectorErase(W25Q64_Device* pDev, uint32_t Address);


//--------------------------------------------------


static W25Q64_HARDWARE g_W25Q64_HWs[W25Q64_NUM] = {
	{
		.ID = W25Q64,
		.SPI_ID = SPI_DEVICE_W25Q64,
	},
};
static W25Q64_Data g_W25Q64_Datas[W25Q64_NUM];
static W25Q64_Device g_W25Q64_Devs[W25Q64_NUM];


//--------------------------------------------------


W25Q64_Device* Drv_W25Q64_GetDevice(W25Q64_ID ID)
{
	for (int i = 0; i < sizeof(g_W25Q64_Devs) / sizeof(g_W25Q64_Devs[0]); i++)
	{
		W25Q64_Data* pData = (W25Q64_Data*)g_W25Q64_Devs[i].Priv_Data;
		if (pData == 0)
			return 0;
		if (pData->ID == ID)
			return &g_W25Q64_Devs[i];
	}
	return 0;
}

static uint8_t Is_SPI_Init = 0;
void Drv_W25Q64_Init(void)
{
	if (Is_SPI_Init == 0)
	{
		// Hardware Init
		Drv_SPI_Init();
		Is_SPI_Init = 1;
	}

	for (uint8_t i = 0; i < W25Q64_NUM; i++)
	{
		// Get Hardware
		W25Q64_HARDWARE hw;
		for (uint8_t j = 0; j < sizeof(g_W25Q64_HWs) / sizeof(g_W25Q64_HWs[0]); j++)
		{
			if (g_W25Q64_HWs[j].ID == (W25Q64_ID)i)
				hw = g_W25Q64_HWs[j];
			if (Is_SPI_Init == 1)
				hw.pSPI = Drv_SPI_GetDevice(hw.SPI_ID);
		}

		// Data Init
		g_W25Q64_Datas[i].ID = (W25Q64_ID)i;
		g_W25Q64_Datas[i].HW = hw;

		// Device Init
		g_W25Q64_Devs[i].ReadID = ReadID;
		g_W25Q64_Devs[i].ReadData = ReadData;
		g_W25Q64_Devs[i].PageProgram = PageProgram;
		g_W25Q64_Devs[i].SectorErase = SectorErase;
		g_W25Q64_Devs[i].Priv_Data = (void*)&g_W25Q64_Datas[i];
	}
}


//--------------------------------------------------


void ReadID(W25Q64_Device* pDev, uint8_t* MID, uint16_t* DID) // Manufacturer：厂商；Device：设备。
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_JEDEC_ID);
	*MID = pSPI->SwapData(pSPI, W25Q64_DUMMY_BYTE);
	*DID = pSPI->SwapData(pSPI, W25Q64_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= pSPI->SwapData(pSPI, W25Q64_DUMMY_BYTE);
	pSPI->Stop(pSPI);
}

void ReadData(W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint32_t Count)
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_READ_DATA);
	pSPI->SwapData(pSPI, Address >> 16);
	pSPI->SwapData(pSPI, Address >> 8);
	pSPI->SwapData(pSPI, Address);
	for (uint32_t i = 0; i < Count; i++)
	{
		DataArray[i] = pSPI->SwapData(pSPI, W25Q64_DUMMY_BYTE);
	}
	pSPI->Stop(pSPI);
}

void WriteEnable(W25Q64_Device* pDev)
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_WRITE_ENABLE);
	pSPI->Stop(pSPI);
}

void WaitBusy(W25Q64_Device* pDev)
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	uint32_t Timeout;
	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_READ_STATUS_REGISTER_1);
	Timeout = 100000;
	while ((pSPI->SwapData(pSPI, W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout--;
		if (Timeout == 0)
		{
			break;
		}
	}
	pSPI->Stop(pSPI);
}

void PageProgram(W25Q64_Device* pDev, uint32_t Address, uint8_t* DataArray, uint16_t Count)
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	WriteEnable(pDev);

	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_PAGE_PROGRAM);
	pSPI->SwapData(pSPI, Address >> 16);
	pSPI->SwapData(pSPI, Address >> 8);
	pSPI->SwapData(pSPI, Address);
	for (uint16_t i = 0; i < Count; i++)
	{
		pSPI->SwapData(pSPI, DataArray[i]);
	}
	pSPI->Stop(pSPI);

	WaitBusy(pDev);
}

void SectorErase(W25Q64_Device* pDev, uint32_t Address)
{
	W25Q64_Data* pData = (W25Q64_Data*)pDev->Priv_Data;
	if (pData == 0)
		return;
	SPI_Device* pSPI = pData->HW.pSPI;
	if (pSPI == 0)
		return;

	WriteEnable(pDev);

	pSPI->Start(pSPI);
	pSPI->SwapData(pSPI, W25Q64_SECTOR_ERASE_4KB);
	pSPI->SwapData(pSPI, Address >> 16);
	pSPI->SwapData(pSPI, Address >> 8);
	pSPI->SwapData(pSPI, Address);
	pSPI->Stop(pSPI);

	WaitBusy(pDev);
}
