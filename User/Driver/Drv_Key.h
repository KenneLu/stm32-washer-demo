#ifndef _KEY_H_
#define _KEY_H_


typedef void (*KeyCallBack) (void);

typedef enum
{
    KEY_ENCODER,
    KEY_POWER,
    KEY_NUM,
} KEY_ID;

typedef struct KEY_Device {
    uint8_t(*CBRegister_P)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*CBRegister_R)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*CBRegister_LP)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*CBRegister_LP_Cont)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*CBRegister_LP_R)(struct KEY_Device* pDev, KeyCallBack CB);
    uint8_t(*Is_Press)(struct KEY_Device* pDev);
    void* Priv_Data;
} KEY_Device;

KEY_Device* Drv_Key_GetDevice(KEY_ID ID);


void Drv_Key_Init(void);
void Drv_Key_Scan(void);


#endif
