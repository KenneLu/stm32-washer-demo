#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>
#include "OLED_Data.h"
#include "timer.h"

/*参数宏定义*********************/

/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_8X16				8
#define OLED_6X8				6

/*IsFilled参数数值*/
#define OLED_UNFILLED			0
#define OLED_FILLED				1

/*********************参数宏定义*/

extern uint8_t OLED_DisplayBuf[8][128];

/*函数声明*********************/

/*初始化函数*/
void OLED_Init(void);

/*更新函数*/
void OLED_Update(void);
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void OLED_Clear(void);
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);
void OLED_Reverse(void);
void OLED_ReverseArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/*显示函数*/
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize);
void OLED_ShowString(uint8_t X, uint8_t Y, char* String, uint8_t FontSize);
void OLED_ShowNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowSignedNum(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowHexNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowBinNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowFloatNum(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void OLED_ShowChinese(uint8_t X, uint8_t Y, char* Chinese);
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t* Image);
void OLED_Printf(uint8_t X, uint8_t Y, uint8_t FontSize, char* format, ...);

/*显示函数简化版*/
void OLED_ShowChar_Easy(uint8_t Line, uint8_t Column, char Chare);
void OLED_ShowString_Easy(uint8_t Line, uint8_t Column, char* Stringe);
void OLED_ShowNum_Easy(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Lengthe);
void OLED_ShowSignedNum_Easy(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Lengthe);
void OLED_ShowHexNum_Easy(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Lengthe);
void OLED_ShowBinNum_Easy(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowFloatNum_Easy(uint8_t Line, uint8_t Column, double Number, uint8_t IntLength, uint8_t FraLengthe);
void OLED_ShowChinese_Easy(uint8_t Line, uint8_t Column, char* Chinese);
void OLED_Printf_Easy(uint8_t Line, uint8_t Column, char* format, ...);

/*绘图函数*/
void OLED_DrawPoint(uint8_t X, uint8_t Y);
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y);
void OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1);
void OLED_DrawRectangle(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void OLED_DrawTriangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t IsFilled);
void OLED_DrawCircle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled);
void OLED_DrawEllipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void OLED_DrawArc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

/*********************函数声明*/
void OLED_Rotation_C_P(int8_t CX, int8_t CY, float* PX, float* PY, int16_t Angle);	//点旋转
void OLED_Rotation_Block(int8_t X, int8_t Y, int8_t Width, int16_t Angle);			//区块旋转
void bresenham_ellipse(int x0, int y0, int rx, int ry);
void ellipse_algorithm(int8_t x0, int8_t y0, int8_t a, int8_t b);



#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
