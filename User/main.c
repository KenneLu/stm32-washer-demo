#include "Task_Manager.h"
#include "Drv_USART.h"


int main(void)
{
	//串口初始化，以便Printf输出
	Drv_USART_Init();

	//创建任务
	Do_Create_Task_Start();

	while (1) {}; /* 正常不会执行到这里 */
}
