#ifndef __SEVEO_MOTOR_H
#define __SEVEO_MOTOR_H


typedef enum
{
    SERVOMOTOR,
    SERVOMOTOR_NUM,
} SERVOMOTOR_ID;

typedef struct SERVOMOTOR_Device {
    void(*SetAngle)(struct SERVOMOTOR_Device* pDev, float Angle);
    void* Priv_Data;
} SERVOMOTOR_Device;

SERVOMOTOR_Device* Drv_ServoMotor_GetDevice(SERVOMOTOR_ID ID);

void Drv_ServoMotor_Init(void);


#endif
