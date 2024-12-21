#ifndef __WASHER_H
#define __WASHER_H	 

#include <stdint.h>

#define ENUM_ITEM(ITEM) ITEM,
#define ENUM_STRING(ITEM) #ITEM,

#define WASHER_STATUS_ENUM(STATUS)                 \
	STATUS(S_INIT)         /*初始化状态*/             \
	STATUS(S_PAUSE)        /*暂停状态*/               \
	STATUS(S_ERROR)        /*异常状态*/               \
	STATUS(S_HEAT_WATER)   /*烧水状态*/               \
	STATUS(S_ADD_WATER)    /*加水状态*/               \
	STATUS(S_WASH)         /*清洗状态*/               \
	STATUS(S_DRAIN_WATER)  /*排水状态*/               \
	STATUS(S_SPIN_DRY)     /*甩干状态*/               \
	STATUS(S_HEAT_DRY)     /*烘干状态*/               \
	STATUS(S_WASH_CNT)     /*完成一轮洗衣*/   		  \
	STATUS(S_FINISH)       /*完成状态*/               \
	STATUS(S_QUIT)         /*退出运行*/   		      \
	STATUS(S_NUM)          /*状态总数(无效状态)*/ 

#define WASH_STATUS_ENUM(STATUS)                    	 \
	STATUS(S_WASH_TURN_LEFT)        	/*即将左转状态*/    \
	STATUS(S_WASH_LEFT_SPEED_UP_15)     /*左转低速状态*/    \
	STATUS(S_WASH_LEFT_SPEED_UP_40)     /*左转中速状态*/    \
	STATUS(S_WASH_LEFT_SPEED_DOWN_15)   /*左转降速状态*/    \
	STATUS(S_WASH_TURN_RIGHT)        	/*即将右转状态*/    \
	STATUS(S_WASH_RIGHT_SPEED_UP_15)    /*右转低速状态*/    \
	STATUS(S_WASH_RIGHT_SPEED_UP_40)    /*右转中速状态*/    \
	STATUS(S_WASH_RIGHT_SPEED_DOWN_15)  /*右转降速状态*/    \

#define SPIN_DRY_STATUS_ENUM(STATUS)                       \
	STATUS(S_SPIN_STOP)        			/*停机状态*/          \
	STATUS(S_SPIN_LEFT_SPEED_UP_20)     /*左传低速状态*/      \
	STATUS(S_SPIN_LEFT_SPEED_UP_40)     /*左传低中速状态*/    \
	STATUS(S_SPIN_LEFT_SPEED_UP_60)     /*左传中速状态*/      \
	STATUS(S_SPIN_LEFT_SPEED_UP_80)     /*左传中高速状态*/    \
	STATUS(S_SPIN_LEFT_SPEED_UP_100)    /*左传高速状态*/      \
	STATUS(S_SPIN_LEFT_SPEED_DOWN_60)   /*左传中速状态*/      \
	STATUS(S_SPIN_LEFT_SPEED_DOWN_40)   /*左传低中速状态*/    \
	STATUS(S_SPIN_LEFT_SPEED_DOWN_20)   /*左传低速状态*/      \


typedef enum
{
	M_FAST_WASH,
	M_STANDERD_WASH,
	M_HARD_WASH,
	M_SPIN_DRY,
	M_HEAT_DRY
} Wash_Mode;

typedef struct
{
	Wash_Mode Mode;			//洗衣模式
	uint8_t Wash_Cnt;		//漂洗次数（次）
	uint8_t Wash_Time;		//洗涤时间（分）
	uint8_t Spin_Dry_Time;	//脱水时间（分）
	uint8_t Water_Volume;	//水量(升)
	uint8_t Water_Temp;		//水温（℃）
	uint8_t Heat_Temp;		//烘干温度（℃）
	uint8_t Total_Time;		//洗衣时间总和（小时-分）
} Washer;

typedef enum
{
	NO_ERROR,
	ERROR_TILT,
	ERROR_SHAKE,
	ERROR_DOOR_OPEN,
} Washer_Errors;

int8_t Washer_Run(void* Param);

#endif
