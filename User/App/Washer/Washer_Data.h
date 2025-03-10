#ifndef __WASHER_DATA_H
#define __WASHER_DATA_H


typedef enum {
    S_WASH_TURN_LEFT,            /*即将左转状态*/
    S_WASH_LEFT_SPEED_UP_15,     /*左转低速状态*/
    S_WASH_LEFT_SPEED_UP_40,     /*左转中速状态*/
    S_WASH_LEFT_SPEED_DOWN_15,   /*左转降速状态*/
    S_WASH_TURN_RIGHT,           /*即将右转状态*/
    S_WASH_RIGHT_SPEED_UP_15,    /*右转低速状态*/
    S_WASH_RIGHT_SPEED_UP_40,    /*右转中速状态*/
    S_WASH_RIGHT_SPEED_DOWN_15,  /*右转降速状态*/
} WASH_STATUS;

typedef enum {
    S_SPIN_STOP,        	     /*停机状态*/
    S_SPIN_LEFT_SPEED_UP_20,     /*左传低速状态*/
    S_SPIN_LEFT_SPEED_UP_40,     /*左传低中速状态*/
    S_SPIN_LEFT_SPEED_UP_60,     /*左传中速状态*/
    S_SPIN_LEFT_SPEED_DOWN_40,   /*左传低中速状态*/
    S_SPIN_LEFT_SPEED_DOWN_20,   /*左传低速状态*/
} SPIN_DRY_STATUS;

typedef enum {
    S_INIT,         /*初始化状态*/
    S_ERROR,        /*错误状态*/
    S_PAUSE,        /*暂停状态*/
    S_HEAT_WATER,   /*烧水状态*/
    S_ADD_WATER,    /*加水状态*/
    S_WASH,         /*清洗状态*/
    S_DRAIN_WATER,  /*排水状态*/
    S_SPIN_DRY,     /*甩干状态*/
    S_HEAT_DRY,     /*烘干状态*/
    S_WASH_CNT,     /*完成一轮洗衣*/
    S_FINISH,       /*完成状态*/
    S_QUIT,         /*退出运行*/
    S_NUM,          /*状态总数(无效状态)*/
} WASHER_STATUS;

typedef enum {
    ACCIDENT_SHUTDOWN = 1,
    CUSTOMER_SHUTDOWN = 2,
} SHUTDOWN_TYPE;

typedef enum WASHER_MODE
{
    M_FAST_WASH,
    M_STANDARD_WASH,
    M_HARD_WASH,
    M_SPIN_DRY,
    M_HEAT_DRY,
    M_NUM
} WASHER_MODE;

typedef struct Washer_Data
{
    WASHER_MODE Mode;			    //洗衣模式
    uint8_t Wash_Cnt;		        //漂洗次数（次）
    uint8_t Wash_Time;		        //洗涤时间（分）
    uint8_t Spin_Dry_Time;	        //脱水时间（分）
    uint8_t Water_Volume;	        //水量(升)
    uint8_t Water_Temp;		        //水温（℃）
    uint8_t Heat_Temp;		        //烘干温度（℃）
    uint8_t Total_Time;		        //洗衣时间总和（小时-分）
    SHUTDOWN_TYPE Shutdown_Type;	//关机类型
    WASHER_STATUS Status_Next;      //下一个状态
    WASHER_STATUS Status_Cur;       //当前状态
    WASHER_STATUS Status_Last;      //上一个状态
    uint8_t(*DataInit)(struct Washer_Data** pData);
    uint8_t(*DataStore)(struct Washer_Data* pData);
    uint8_t(*DataRestore)(struct Washer_Data** pData);
} Washer_Data, (*pWasher_Data);

extern pWasher_Data g_pWDat;

void Drv_Washer_Data_Init(void);


#endif
