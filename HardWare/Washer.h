#ifndef __WASHER_H
#define __WASHER_H	 

#include <stdint.h>

#define ENUM_ITEM(ITEM) ITEM,
#define ENUM_STRING(ITEM) #ITEM,

#define WASHER_STATUS_ENUM(STATUS)                     \
	STATUS(WS_CHECK)        /*安全自检*/               \
	STATUS(WS_IDLE)         /*空闲(等待模式设置)状态*/ \
	STATUS(WS_ADD_WATER)    /*加水状态*/               \
	STATUS(WS_WASH)         /*清洗状态*/               \
	STATUS(WS_DRAIN_WATER)  /*排水状态*/               \
	STATUS(WS_SPIN_DRY)     /*甩干状态*/               \
	STATUS(WS_PAUSE)        /*暂停状态*/               \
	STATUS(WS_NUM)          /*状态总数(无效状态)*/ 

typedef enum
{
	FAST,
	STANDERD,
	HARD,
	SWING,
	HEAT
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

int8_t Washer_Run(void* Param);

#endif
