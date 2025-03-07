#ifndef __DELAY_H
#define __DELAY_H

typedef uint32_t TickType_t;

void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_Until_ms(TickType_t* const pxPreviousWakeTime, uint32_t ms);

#endif
