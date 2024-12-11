#ifndef __SPI_H
#define __SPI_H

//GPIO
#define SPI_GPIO_RCC        RCC_APB2Periph_GPIOB
#define SPI_GOIO_x          GPIOB
#define SPI_GPIO_PIN_SS     GPIO_Pin_12
#define SPI_GPIO_PIN_SCK    GPIO_Pin_13
#define SPI_GPIO_PIN_MISO   GPIO_Pin_14
#define SPI_GPIO_PIN_MOSI   GPIO_Pin_15
#define SPI_GOIO_MODE_SS    GPIO_Mode_Out_PP
#define SPI_GOIO_MODE_SCK   GPIO_Mode_AF_PP
#define SPI_GOIO_MODE_MISO  GPIO_Mode_IPU
#define SPI_GOIO_MODE_MOSI  GPIO_Mode_AF_PP

//SPI
#define SPI_SPI_RCC         RCC_APB1Periph_SPI2
#define SPI_SPI_x           SPI2

void MySPI_Init(void);
void MySPI_Start(void);
void MySPI_Stop(void);
uint8_t MySPI_SwapByte(uint8_t BytSend);

#endif
