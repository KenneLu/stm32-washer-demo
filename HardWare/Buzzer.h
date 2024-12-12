#ifndef __BUZZER_H
#define __BUZZER_H

//GPIO
#define SPI_GPIO_RCC     RCC_APB2Periph_GPIOA
#define SPI_GOIO_x       GPIOA
#define SPI_GPIO_PIN_IO  GPIO_Pin_12
#define SPI_GOIO_MODE    GPIO_Mode_Out_PP

void Buzzer_Init(void);
void Buzzer_On(uint8_t On);
void Buzzer_Revert(void);

#endif
