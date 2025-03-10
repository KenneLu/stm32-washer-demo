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


void Washer_Init(void);
void Washer_State_Machine(void);
void Washer_Stop(void);
void Washer_Key(void);
void Washer_Pause(void);
void Washer_Resume(void);
void Washer_Safety(void);

void Washer_Error_Occur(void);
void Washer_Error_Warning(void);
void Washer_Error_Fixed(void);


#endif
