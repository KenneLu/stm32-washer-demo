#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Sys_Delay.h"

#define FREERTOS 1

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{

#if FREERTOS == 1	//FreeRTOS的延时函数

	uint32_t ticks = xus * (SystemCoreClock / 1000000);//需要的节拍数 
	uint32_t reload = SysTick->LOAD;	//系统定时器的重载值	
	uint32_t told = SysTick->VAL;       //刚进入时的计数器值
	uint32_t tnow, tcnt = 0;

	/* 挂起调度器[可选,会导致高优先级任务无法抢占当前任务，但能够提高当前任务时间的精确性] */
	// vTaskSuspendAll();

	while (1)
	{
		tnow = SysTick->VAL;

		if (tnow != told)
		{
			/* SYSTICK是一个递减的计数器 */
			if (tnow < told)
				tcnt += told - tnow;
			else
				tcnt += reload - tnow + told;

			told = tnow;

			/* 时间超过/等于要延迟的时间,则退出。*/
			if (tcnt >= ticks)
				break;
		}
	}

	/* 恢复调度器[可选] */
	// xTaskResumeAll();

#else	//SysTick的延时函数

	SysTick->LOAD = 72 * xus;				//设置定时器重装值
	SysTick->VAL = 0x00;					//清空当前计数值
	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
	while (!(SysTick->CTRL & 0x00010000));	//等待计数到0
	SysTick->CTRL = 0x00000004;				//关闭定时器

#endif




}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{

#if FREERTOS == 1

	vTaskDelay(xms / portTICK_RATE_MS);

#else

	while (xms--)
	{
		Delay_us(1000);
	}

#endif

}


#if FREERTOS == 1
/**
  * @brief  毫秒级延时（FreeRTOS绝对延时函数）
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_Until_ms(TickType_t* const pxPreviousWakeTime, const uint32_t xms)
{
	vTaskDelayUntil(pxPreviousWakeTime, xms / portTICK_RATE_MS);
}
#endif
