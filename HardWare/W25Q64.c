#include "stm32f10x.h"                  // Device header
#include "W25Q64_Ins.h"
#include "MySPI.h"


SPI_HW_Device* g_pDev_SPI_HW;

void W25Q64_Init()
{
	Drv_SPI_HW_Init();
	g_pDev_SPI_HW = Drv_SPI_HW_GetDevice(SPI_HW);
}

void W25Q64_ReadID(uint8_t* MID, uint16_t* DID) // Manufacturer：厂商；Device：设备。
{
	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_JEDEC_ID);
	*MID = g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_DUMMY_BYTE);
	*DID = g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_DUMMY_BYTE);
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);
}

void W25Q64_WriteEnable(void)
{
	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_WRITE_ENABLE);
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);
}

void W25Q64_WaitBusy(void)
{
	uint32_t Timeout;
	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_READ_STATUS_REGISTER_1);
	Timeout = 100000;
	while ((g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout--;
		if (Timeout == 0)
		{
			break;
		}
	}
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);
}

void W25Q64_PageProgram(uint32_t Address, uint8_t* DataArray, uint16_t Count)
{
	W25Q64_WriteEnable();

	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_PAGE_PROGRAM);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 16);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 8);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address);
	for (uint16_t i = 0; i < Count; i++)
	{
		g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, DataArray[i]);
	}
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);

	W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t Address)
{
	W25Q64_WriteEnable();

	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_SECTOR_ERASE_4KB);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 16);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 8);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address);
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);

	W25Q64_WaitBusy();
}

void W25Q64_ReadData(uint32_t Address, uint8_t* DataArray, uint32_t Count)
{
	g_pDev_SPI_HW->Start(g_pDev_SPI_HW);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_READ_DATA);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 16);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address >> 8);
	g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, Address);
	for (uint32_t i = 0; i < Count; i++)
	{
		DataArray[i] = g_pDev_SPI_HW->SwapData(g_pDev_SPI_HW, W25Q64_DUMMY_BYTE);
	}
	g_pDev_SPI_HW->Stop(g_pDev_SPI_HW);
}
