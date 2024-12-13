#include <string.h>
#include <math.h>
#include "OLED.h"

/********OLED补丁*********/
/**B站UP:加油哦大灰狼****/

/**
  * 函    数：OLED显示汉字单字
  * 参    数：X 指定字符串左上角的横坐标，范围：0~127
  * 参    数：Y 指定字符串左上角的纵坐标，范围：0~63
  * 参    数：Hanzi 指定要显示的字符，范围：字库字符
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_MyShowChinese(int8_t X, int8_t Y, char* Hanzi, uint8_t FontSize)	//汉字单字打印;
{
	uint8_t pIndex;
	for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++)
	{
		/*找到匹配的汉字*/
		if (strcmp(OLED_CF16x16[pIndex].Index, Hanzi) == 0)
		{
			break;		//跳出循环，此时pIndex的值为指定汉字的索引
		}
	}
	/*将汉字字模库OLED_CF16x16的指定数据以16*16的图像格式显示*/
	OLED_ShowImage(X, Y, 16, 16, OLED_CF16x16[pIndex].Data);
}

/**
  * 函    数：OLED显示字符串
  * 参    数：X 指定字符串左上角的横坐标，范围：0~127
  * 参    数：Y 指定字符串左上角的纵坐标，范围：0~63
  * 参    数：String 指定要显示的字符串，范围：字库字符组成的字符串
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowString(uint8_t X, uint8_t Y, char* String, uint8_t FontSize)	//中英文打印;
{
	uint8_t i, len;
	for (i = 0, len = 0; String[i] != '\0'; i++, len++)		//遍历字符串的每个字符
	{
		if (String[i] == '\n') { Y += (FontSize == 8) ? 16 : 8; len = 255; continue; }			//兼容换行符
		if (X + (len + 1) * FontSize > 128) { Y += (FontSize == 8) ? 16 : 8; len = 0; }

		if (String[i] > '~')				//如果不属于英文字符
		{
			char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = { 0 };
			SingleChinese[0] = String[i];
			i++;
			SingleChinese[1] = String[i];
			if (OLED_CHN_CHAR_WIDTH == 3)
			{
				i++;
				SingleChinese[2] = String[i];
			}
			if (FontSize == 8) { OLED_MyShowChinese(X + len * FontSize, Y, SingleChinese, FontSize); }
			else { OLED_MyShowChinese(X + len * FontSize, Y, SingleChinese, FontSize); }
			len += 1;
		}
		else {
			/*调用OLED_ShowChar函数，依次显示每个字符*/
			OLED_ShowChar(X + len * FontSize, Y, String[i], FontSize);
		}
	}
}


/**
  * 函    数：旋转点
  * 参    数：CX 指定旋转原点的横坐标，范围：0~127
  * 参    数：CY 指定旋转原点的纵坐标，范围：0~63
  * 参    数：PX 指定旋转点的横坐标，范围：0~127
  * 参    数：PY 指定旋转点的纵坐标，范围：0~63
  * 参    数：Angle 指定旋转角度，范围：-360~360
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Rotation_C_P(int8_t CX, int8_t CY, float* PX, float* PY, int16_t Angle)//旋转点
{
	float Theta = (3.14 / 180) * Angle;
	float Xd = *PX - CX;
	float Yd = *PY - CY;

	*PX = (Xd)*cos(Theta) - (Yd)*sin(Theta) + CX;// + 0.5;
	*PY = (Xd)*sin(Theta) + (Yd)*cos(Theta) + CY;// + 0.5;
}
/**
  * 函    数：将OLED显存数组部分旋转
  * 参    数：X 指定旋转原点的横坐标，范围：0~127
  * 参    数：Y 指定旋转原点的纵坐标，范围：0~63
  * 参    数：Width 指定旋转区域半径，范围：0~63
  * 参    数：Angle 指定旋转角度，范围：-360~360
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Rotation_Block(int8_t X, int8_t Y, int8_t Width, int16_t Angle) //旋转区块
{
	uint8_t OLED_DpB1[8][128];
	memcpy(OLED_DpB1, OLED_DisplayBuf, 1024);

	//OLED_Clear();
	OLED_ClearArea(X - Width, Y - Width, Width * 2, Width * 2);
	float Theta = (3.14 / 180) * Angle;

	uint8_t x, y;
	for (y = Y - Width; y < Y + Width; y++)		//遍历指定页
	{
		for (x = X - Width; x < X + Width; x++)	//遍历指定列
		{
			//			if(x > 127) break; 
			//			if(y > 63) break; 
			//			if(x < 0) break; 
			//			if(y < 0) break; 
			x %= 128;
			y %= 64;

			if (OLED_DpB1[y / 8][x] & 0x01 << (y % 8))	//效果同if(OLED_GetPoint(x, y))
			{
				OLED_DrawPoint(
					(float)(x - X) * cos(Theta) - (float)(y - Y) * sin(Theta) + X,//+ 0.5,
					(float)(x - X) * sin(Theta) + (float)(y - Y) * cos(Theta) + Y // + 0.5
				);

			}
		}
	}
}


void ellipse_algorithm(int8_t x0, int8_t y0, int8_t a, int8_t b) {
	float t = 0.01;
	float d = 6.28 / ((a + b) * 2);

	int8_t x = x0;
	int8_t y = y0;

	int8_t xtemp = x;
	int8_t ytemp = y;

	x = (a * cos(t));
	y = (b * sin(t));

	while (t <= 6.28) {

		xtemp = x;
		ytemp = y;

		x = (a * cos(t));
		y = (b * sin(t));
		OLED_DrawLine(x0 + xtemp, y0 + ytemp, x0 + x, y0 + y);
		t += d;
		//OLED_Update();
	}
}



