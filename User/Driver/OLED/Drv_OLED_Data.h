#ifndef __OLED_DATA_H
#define __OLED_DATA_H

/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
#include <stdint.h>

/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			3		//UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct
{
	uint8_t Data[32];						//字模数据
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

/*图像数据声明*/
extern const uint8_t Diode[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
extern const uint8_t Cursor[];
extern const uint8_t goutou[];
extern const uint8_t Wallpaper[];
/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/



/*****************Kenneth****************/
// Data_Queue 相关结构体

#define OLED_UPDATE OLED_Send((OLED_DATA_DISPLAY) { .Cmd = OLED_CMD_UPDATE });

#define OLED_CLEAR OLED_Send((OLED_DATA_DISPLAY) { .Cmd = OLED_CMD_CLEAR });

#define OLED_CLEAR_UPDATE OLED_Send((OLED_DATA_DISPLAY) { .Cmd = OLED_CMD_CLEAR_UPDATE });

#define OLED_SHOW_NUM_E(Line, Column, Number, Length) 	\
{															\
	OLED_Send((OLED_DATA_DISPLAY) { 						\
		.Cmd = OLED_CMD_PUSH_UPDATE, 						\
		.Type = OLED_NUM_E,									\
		.Union.Num = { Line, Column, Number, Length } });	\
}

#define OLED_SHOW_STR_E(Line, Column, String) 	\
{													\
	OLED_Send((OLED_DATA_DISPLAY) { 				\
		.Cmd = OLED_CMD_PUSH_UPDATE, 				\
		.Type = OLED_STR_E,							\
		.Union.Str = { Line, Column, String } });	\
}

#define OLED_SHOW_CN_E(Line, Column, String) 	\
{														\
	OLED_Send((OLED_DATA_DISPLAY) { 					\
		.Cmd = OLED_CMD_PUSH_UPDATE, 					\
		.Type = OLED_CN_E,								\
		.Union.CN = { Line, Column, String } });		\
}


typedef enum {
	OLED_CMD_UPDATE,
	OLED_CMD_PUSH,
	OLED_CMD_PUSH_UPDATE,
	OLED_CMD_CLEAR,
	OLED_CMD_CLEAR_UPDATE,
} OLED_DATA_CMD;

typedef enum {
	OLED_NUM,
	OLED_STR,
	OLED_RECT,
	OLED_NUM_E,
	OLED_STR_E,
	OLED_CN_E,
} OLED_DATDA_TYPE;

typedef struct {
	OLED_DATA_CMD Cmd;
	OLED_DATDA_TYPE Type;
	union {
		struct { uint8_t Line; uint8_t Column; uint32_t Number; uint8_t Length; } Num;
		struct { uint8_t Line; uint8_t Column; char* String; } Str;
		struct { uint8_t Line; uint8_t Column; char* Chinese; } CN;
		struct { uint8_t X; uint8_t Y; uint8_t Width; uint8_t Height; uint8_t IsFilled; } Rect;
	} Union;
} OLED_DATA_DISPLAY;


void Drv_OLED_Data_Queue_Init(void);
void OLED_Send(OLED_DATA_DISPLAY Display);
void OLED_Display(void);
/*****************Kenneth****************/


#endif
