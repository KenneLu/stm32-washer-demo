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

int8_t Washer_Run(void);

#endif
