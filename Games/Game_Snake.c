#include "stm32f10x.h"                  // Device header
#include <stdlib.h>
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Encoder.h"
#include "Game_Snake.h"
#include "Game_Snake_Data.h"

extern uint8_t OLED_DisplayBuf[8][128];		//把OLED显存拿过来

Tile Map[8][16];
uint8_t Game_Speed = 200;		//游戏速度(延时)
uint8_t Game_Credits = 0;	//游戏积分

void Game_Snake_Show_Tile_8x8(uint8_t Y, uint8_t X, Tile Tile)
{
	for (int8_t i = 0; i < 8; i++)
	{
		OLED_DisplayBuf[Y][X * 8 + i] = (Game_Snake_Tile_8x8[Tile][i]);			//显示区块
	}
}

void Map_Clear(void)	//清除地图
{
	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 16; j++) {
			Map[i][j] = air;
		}
	}
}

void Map_Update(void)		//上传地图
{
	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 16; j++) {
			Game_Snake_Show_Tile_8x8(i, j, Map[i][j]);
		}
	}
}

void RandFood(void)	//随机放置食物
{
	srand(rand());
	int Y = rand() % 8;
	int X = rand() % 16;
	while (Map[Y][X] != air)
	{
		Y = rand() % 8;
		X = rand() % 16;
	}
	Map[Y][X] = food;
}

Tile Game_Snake_GetFront(Game_Snake_Class* Snake)	//获取前方信息
{
	switch (Snake->Heading)
	{
	case up: { return Map[Snake->H_Y - 1][Snake->H_X]; }
	case right: { return Map[Snake->H_Y][Snake->H_X + 1]; }
	case down: { return Map[Snake->H_Y + 1][Snake->H_X]; }
	case left: { return Map[Snake->H_Y][Snake->H_X - 1]; }
	}
	return air;
}

uint8_t Game_Snake_Advance(Game_Snake_Class* Snake)//把蛇映射到地图
{
	uint8_t Front_X = Snake->H_X, Front_Y = Snake->H_Y;

	switch (Snake->Heading)
	{
	case up: { Front_Y = Snake->H_Y - 1; break; }
	case right: { Front_X = Snake->H_X + 1; break; }
	case down: { Front_Y = Snake->H_Y + 1; break; }
	case left: { Front_X = Snake->H_X - 1; break; }
	}
	Front_X %= 16;
	Front_Y %= 8;

	if (Map[Front_Y][Front_X] == air)							//如果前方为空气		
	{
		*Snake->node[Snake->Head_i] = SnakeBody;				//蛇头节点指向的地图方块变为蛇身
		Snake->Head_i = (Snake->Head_i + 1) % 128;				//蛇头节点下标前进1格
		Snake->node[Snake->Head_i] = &Map[Front_Y][Front_X];	//蛇头节点指向到前方地图方块
		*Snake->node[Snake->Head_i] = SnakeHead;				//蛇头节点指向的地图方块变为蛇头

		*Snake->node[Snake->Tail_i] = air;						//消除蛇尾地图方块
		Snake->Tail_i = (Snake->Tail_i + 1) % 128;				//蛇尾节点下标前进1格
		*Snake->node[Snake->Tail_i] = SnakeTail;				//蛇尾节点指向的地图方块变为蛇尾

		Snake->H_X = Front_X;									//蛇头坐标更新
		Snake->H_Y = Front_Y;
	}
	else if (Map[Front_Y][Front_X] == food)						//如果前方为食物
	{
		*Snake->node[Snake->Head_i] = SnakeBody;				//蛇头节点指向的地图方块变为蛇身
		Snake->Head_i = (Snake->Head_i + 1) % 128;				//蛇头节点下标前进1格
		Snake->node[Snake->Head_i] = &Map[Front_Y][Front_X];	//蛇头节点指向到前方地图方块
		*Snake->node[Snake->Head_i] = SnakeHead;				//蛇头节点指向的地图方块变为蛇头

		RandFood();											//随机放置食物
		Game_Credits += 1;									//加积分
		Game_Speed -= Game_Speed / 8;							//减延时


		Snake->H_X = Front_X;								//蛇头坐标更新
		Snake->H_Y = Front_Y;
	}
	else				//前方有障碍
	{
		return 0;		//前进失败
	}

	return 1;			//前进成功
}

void Game_Snake_Play(Game_Snake_Class* Snake)		//开始游戏
{
	while (Snake->Head_i - Snake->Tail_i < 3)		//出身点随机方向强制移动三格;
	{
		Snake->H_X++;
		*Snake->node[Snake->Head_i] = SnakeBody;					//蛇头节点指向的地图方块变为蛇身
		Snake->Head_i = (Snake->Head_i + 1) % 128;					//蛇头节点下标前进1格
		Snake->node[Snake->Head_i] = &Map[Snake->H_Y][Snake->H_X];	//蛇头节点指向到前方地图方块
		*Snake->node[Snake->Head_i] = SnakeHead;					//蛇头节点指向的地图方块变为蛇头

		Map_Update();
		OLED_Update();
		Delay_ms(Game_Speed);
	}
	WSAD Heading_Previous = Snake->Heading;
	int8_t temp = 0;

	while (1)	//主循环
	{
		if (Key_Enter_Get()) { return; }	//退出游戏
		if (Key_Back_Get()) { return; }	//退出游戏

		temp = Encoder_Get_Div4();

		if (temp > 0) { Snake->Heading++; }
		if (temp < 0) { Snake->Heading--; }
		Snake->Heading %= 4;

		if (Game_Snake_Advance(Snake)) { Heading_Previous = Snake->Heading; }	//如果前进成功则记录方向
		else
		{
			Snake->Heading = Heading_Previous; 		//如果前进失败尝试之前的方向再试一次
			if (Game_Snake_Advance(Snake) == 0)		//如果仍然失败则游戏结束
			{
				OLED_ShowImage(0, 0, 128, 64, Wallpaper);
				OLED_Printf(0, 48, OLED_8X16, "游戏结束 得分:%d ", Game_Credits);
				OLED_Update();

				while (1) {
					if (Key_Enter_Get()) { return; }		//退出游戏
					if (Key_Back_Get()) { return; }		//退出游戏
				}
			}
		}

		Map_Update();
		OLED_Update();
		Delay_ms(Game_Speed);
	}
}

void Game_Snake_Init(void)
{
	Game_Credits = 0;
	Game_Speed = 200;
	Map_Clear();		//清除蛇尸

	Game_Snake_Class Snake_1;
	Snake_1.Head_i = 0;
	Snake_1.Tail_i = 0;
	Snake_1.H_X = rand() % 16;
	Snake_1.H_Y = rand() % 8;
	Snake_1.Heading = right;
	Snake_1.node[Snake_1.Head_i] = &Map[Snake_1.H_Y][Snake_1.H_X];

	Map[Snake_1.H_Y][Snake_1.H_X] = SnakeHead;

	RandFood();

	OLED_Update();
	Game_Snake_Play(&Snake_1);

	Encoder_Get_Div4();
	return;
}
