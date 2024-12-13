#ifndef __GAME_SNAKE_H
#define __GAME_SNAKE_H

typedef enum{
	air,		//空气
	wall,		//墙
	food,		//食物
	SnakeHead,	//蛇头
	SnakeBody,	//蛇身
	SnakeTail,	//蛇尾
}Tile;

typedef enum{
	up		= 0	,	//	上
	right	= 1	,	//	右
	down	= 2	,	//	下
	left	= 3	,	//	左
}WSAD;				//方向键

typedef struct{
	Tile* node[128];	//蛇节点
	
	uint8_t Head_i;		//蛇头下标
	uint8_t Tail_i;		//蛇尾下标
	
	uint8_t	 H_X;		//蛇头X坐标
	uint8_t H_Y;		//蛇头Y坐标
	WSAD Heading;		//前进方向
}Game_Snake_Class;

void Game_Snake_Init(void);

#endif
