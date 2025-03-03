#ifndef _KEY_H_
#define _KEY_H_


typedef void (*KeyCallBack) (void);

typedef enum
{
    KEY_ENCODER,
    KEY_POWER,
    KEYNUM,
} KEY_ID;

typedef struct KEY_Device {
    uint8_t(*Key_CBRegister_P)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Key_CBRegister_R)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Key_CBRegister_LP)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Key_CBRegister_LP_Cont)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Key_CBRegister_LP_R)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Is_Key_Pressed)(struct KEY_Device* pDev);
    void* Priv_Data;
} KEY_Device;

void Key_Init(void);
void Key_Scan(void);
KEY_Device* GetKeyDevice(KEY_ID ID);


#endif
