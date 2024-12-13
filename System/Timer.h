#ifndef __TIMER_H
#define __TIMER_H

//T = (SYS_TIM_Period) / [SYS_CLK * 10] = (100) / [(72,000,000/1,000,000) * 10] = 0.001s
//T = 1ms
#define SYS_CLK (SystemCoreClock / 1000000) // 72 or 36
#define SYS_TIM_Period 100 - 1              // 0~65535
#define SYS_TIM_Prescaler SYS_CLK * 10 - 1  // 0~65535

#define SYS_MS 1
#define SYS_S  1000

void Timer_Init(void);
uint16_t Timer_GetCounter(void);

#endif
