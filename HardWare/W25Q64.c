#include "stm32f10x.h"                  // Device header
#include "W25Q64_Ins.h"
#include "MySPI.h"


SPI_Device* g_pDev_SPI_W25Q64;

void W25Q64_Init()
{
	Drv_SPI_Init();
	g_pDev_SPI_W25Q64 = Drv_SPI_GetDevice(SPI_DEVICE_W25Q64);
}

void W25Q64_ReadID(uint8_t* MID, uint16_t* DID) // Manufacturer：厂商；Device：设备。
{
	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_JEDEC_ID);
	*MID = g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_DUMMY_BYTE);
	*DID = g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_DUMMY_BYTE);
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);
}

void W25Q64_WriteEnable(void)
{
	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_WRITE_ENABLE);
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);
}

void W25Q64_WaitBusy(void)
{
	uint32_t Timeout;
	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_READ_STATUS_REGISTER_1);
	Timeout = 100000;
	while ((g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout--;
		if (Timeout == 0)
		{
			break;
		}
	}
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);
}

void W25Q64_PageProgram(uint32_t Address, uint8_t* DataArray, uint16_t Count)
{
	W25Q64_WriteEnable();

	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_PAGE_PROGRAM);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 16);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 8);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address);
	for (uint16_t i = 0; i < Count; i++)
	{
		g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, DataArray[i]);
	}
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);

	W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t Address)
{
	W25Q64_WriteEnable();

	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_SECTOR_ERASE_4KB);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 16);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 8);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address);
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);

	W25Q64_WaitBusy();
}

void W25Q64_ReadData(uint32_t Address, uint8_t* DataArray, uint32_t Count)
{
	g_pDev_SPI_W25Q64->Start(g_pDev_SPI_W25Q64);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_READ_DATA);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 16);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address >> 8);
	g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, Address);
	for (uint32_t i = 0; i < Count; i++)
	{
		DataArray[i] = g_pDev_SPI_W25Q64->SwapData(g_pDev_SPI_W25Q64, W25Q64_DUMMY_BYTE);
	}
	g_pDev_SPI_W25Q64->Stop(g_pDev_SPI_W25Q64);
}
