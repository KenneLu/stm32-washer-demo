#ifndef __WASHER_H
#define __WASHER_H	 

#include <stdint.h>


typedef enum
{
	NO_ERROR,
	ERROR_TILT,
	ERROR_SHAKE,
	ERROR_DOOR_OPEN,
} Washer_Errors;


void Washer_Init(uint8_t Is_Restore);
int8_t Washer_Run(void* Param);


#endif
