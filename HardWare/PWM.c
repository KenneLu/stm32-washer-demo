#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "OLED.h"

TIM_TimeBaseInitTypeDef TIM_Structure_Default = {
    .TIM_ClockDivision = TIM_CKD_DIV1, // 滤波电路采样信号的分频系数
    .TIM_CounterMode = TIM_CounterMode_Up, // 向上计数
    .TIM_Period = 20000 - 1,         // ARR 分辨率 1%
    .TIM_Prescaler = 72 - 1,       // PSC
    .TIM_RepetitionCounter = 0,    // 重复计数器，置0。
};

PWM_TIM PWM_TIM1 = {
    .TIM_x = TIM1,
    .Inited = FALSE,
    .Level = Advanced,
    .TIM_RCC = RCC_APB2Periph_TIM1,
    .InitStructure = &TIM_Structure_Default,
    .GPIO_RCC = RCC_APB2Periph_GPIOA,
    .GPIO_x = GPIOA,
    .OC_PIN_List = (OC_PIN[]){
        {GPIO_Pin_8, FALSE},    // TIM1_CH1
        {GPIO_Pin_9, FALSE},    // TIM1_CH2
        {GPIO_Pin_10, FALSE},   // TIM1_CH3
        {GPIO_Pin_11, FALSE}},  // TIM1_CH4
    .OC_PIN_Count = 4,
    .GPIO_MODE = GPIO_Mode_AF_PP // 复用推挽输出，引脚PINx的控制权交给外设CHx，参考手册 8.1.11
};

PWM_TIM PWM_TIM2 = {
    .TIM_x = TIM2,
    .Inited = FALSE,
    .Level = General,
    .TIM_RCC = RCC_APB1Periph_TIM2,
    .InitStructure = &TIM_Structure_Default,
    .GPIO_RCC = RCC_APB2Periph_GPIOA,
    .GPIO_x = GPIOA,
    .OC_PIN_List = (OC_PIN[]){
        {GPIO_Pin_0, FALSE},    // TIM1_CH1
        {GPIO_Pin_1, FALSE},    // TIM1_CH2
        {GPIO_Pin_2, FALSE},   // TIM1_CH3
        {GPIO_Pin_3, FALSE}},  // TIM1_CH4
    .OC_PIN_Count = 4,
    .GPIO_MODE = GPIO_Mode_AF_PP // 复用推挽输出，引脚PINx的控制权交给外设CHx，参考手册 8.1.11
};

void PWM_TIM_Init(PWM_TIM* T, TIM_TimeBaseInitTypeDef* CustomStruct)
{
    // if (T->Inited) return; // 已经初始化过了

    //开启TIM时钟，默认内部时钟源
    if (T->TIM_RCC == RCC_APB2Periph_TIM1) RCC_APB2PeriphClockCmd(T->TIM_RCC, ENABLE);
    else if (T->TIM_RCC == RCC_APB1Periph_TIM2) RCC_APB1PeriphClockCmd(T->TIM_RCC, ENABLE);
    else if (T->TIM_RCC == RCC_APB1Periph_TIM3) RCC_APB1PeriphClockCmd(T->TIM_RCC, ENABLE);
    else if (T->TIM_RCC == RCC_APB1Periph_TIM4) RCC_APB1PeriphClockCmd(T->TIM_RCC, ENABLE);
    TIM_InternalClockConfig(T->TIM_x);

    //配置时基单元
    if (CustomStruct) TIM_TimeBaseInit(T->TIM_x, CustomStruct);
    else TIM_TimeBaseInit(T->TIM_x, T->InitStructure);

    //启动定时器
    if (T->Level == Advanced) TIM_CtrlPWMOutputs(T->TIM_x, ENABLE);    // 高级定时器
    TIM_Cmd(T->TIM_x, ENABLE);

    T->Inited = TRUE; // 标记初始化完成
}

void PWM_GPIO_Init(PWM_TIM* T, OC_ID OC_x)
{
    if (OC_x >= T->OC_PIN_Count) return; // 参数错误
    if (T->OC_PIN_List[OC_x].Inited)  return; // 已经初始化过了

    // 配置GPIO
    RCC_APB2PeriphClockCmd(T->GPIO_RCC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = T->GPIO_MODE;
    GPIO_InitStructure.GPIO_Pin = T->OC_PIN_List[OC_x].PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(T->GPIO_x, &GPIO_InitStructure);

    //配置OC单元
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);                         // 默认初始化
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               // 设置OC模式
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 设置极性
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   // 设置输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;                              // 设置CCR
    if (OC_x == OC1) TIM_OC1Init(T->TIM_x, &TIM_OCInitStructure);
    else if (OC_x == OC2) TIM_OC2Init(T->TIM_x, &TIM_OCInitStructure);
    else if (OC_x == OC3) TIM_OC3Init(T->TIM_x, &TIM_OCInitStructure);
    else if (OC_x == OC4) TIM_OC4Init(T->TIM_x, &TIM_OCInitStructure);

    T->OC_PIN_List[OC_x].Inited = TRUE; // 标记初始化完成
}

void PWM_SetCompare_x(PWM_TIM* T, OC_ID OC_x, uint16_t Compare) // 通过CCR调整频率
{
    if (OC_x == OC1) TIM_SetCompare1(T->TIM_x, Compare);
    else if (OC_x == OC2) TIM_SetCompare2(T->TIM_x, Compare);
    else if (OC_x == OC3) TIM_SetCompare3(T->TIM_x, Compare);
    else if (OC_x == OC4) TIM_SetCompare4(T->TIM_x, Compare);
}

void PWM_SetPrescaler(PWM_TIM* T, uint16_t Prescaler) // 通过PSC调整频率
{
    TIM_PrescalerConfig(T->TIM_x, Prescaler, TIM_PSCReloadMode_Update);
}
