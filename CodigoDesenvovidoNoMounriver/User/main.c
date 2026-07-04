#include "debug.h"


// ======== Variaveis Globais ======== //

// -------- ADC --------
#define V_pin GPIO_Pin_2
#define I_pin GPIO_Pin_4
#define V_ADC ADC_Channel_3
#define I_ADC ADC_Channel_7

#define V_sample ADC_SampleTime_30Cycles
#define I_sample ADC_SampleTime_30Cycles
volatile u16 adc_buf[2];
volatile u16 ADC_COMPLETE;
volatile u16 V_lido;
volatile u16 I_lido;

// -------- USART --------
volatile uint32_t V_recebido = 0;
volatile uint32_t I_recebido = 0;
#define TxSize (size (exemplo_TX))
#define RxSize (size (exemplo_RX))
#define size(a) (sizeof (a) / sizeof (*(a)))
#define exemplo_TX "V:1023,I:1023\r\n"
#define exemplo_RX "V:1023,I:1023\r\n"

volatile char TxBuffer[TxSize] = {0};
volatile char RxBuffer[RxSize] = {0};
volatile u32 Tx_pronto = 0;
volatile u32 Rx_pronto = 0;

//--------- Variaveis Globais -------
u16 PWM = 0;
#define TIM1_PSC 1 - 1  // Timer do PWM
#define TIM1_ARR 480 - 1

//-------- variaveis e defines do controle ------
#define PWM_MAX     480 - 60
#define PWM_MIN     8

#define KP          6   // 2
#define KI          4   // 4
#define SHIFT       4   // 4

#define V_FPB_STEP    10
#define I_FPB_STEP    40

#define INT_MAX     3000   // 824//927 //800 //340
#define INT_MIN     -3000  // 824//927 //800 //340

u32 V_ref = 0;
u32 I_ref = 0;


// ======== Prototipos ======== //

// -------- Inicializar sistema --------
void Custom_Option_Byte_CFG (void);
void Custom_TIM1_CFG (void);
void Custom_OPA1_CFG (void);
void Custom_ADC_CFG (u16 CCR);
void Custom_PWM_CFG (void);
void Custom_USARTx_CFG (void);
void EXTI0_INT_CFG (void);

// -------- Interrupt --------
void USART1_IRQHandler (void) __attribute__ ((interrupt));
void DMA1_Channel1_IRQHandler (void) __attribute__ ((interrupt (("WCH-Interrupt-fast"))));

// -------- Funcoes do usuario --------
void ControleBuck (void);
void Verifica_RX (void);

// ======== Main ======== //

int main (void) {

    RCC_PLLCmd (ENABLE);
    RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    // SystemCoreClockUpdate();
    // SystemInit();
    Delay_Init();

    Custom_Option_Byte_CFG();
    Custom_TIM1_CFG();
    Custom_OPA1_CFG();
    Custom_ADC_CFG (0);
    Custom_PWM_CFG();
    Custom_USARTx_CFG();

    USART_DMACmd (USART1, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd (USART1, USART_DMAReq_Rx, ENABLE);

    while (1) {
        if (ADC_COMPLETE)
            ControleBuck();
        if (Rx_pronto)
            Verifica_RX();
        sprintf ((char *)TxBuffer, "V:%04d,I:%04d\r\n", V_lido, I_lido);
    }
}

// ======== Funcoes ======== //

void ControleBuck (void) {
    volatile static int32_t integral = 0;
    volatile static int32_t erro = 0;
    volatile static u32     PROTECAO_ATIVA = 0;
    volatile static u32     V_PID = 0;

    volatile int32_t saida = 0;

    /* ---------- Filtro passa baixa e protecao de corrente ---------- */
    PROTECAO_ATIVA = (I_lido > I_ref);
    
    if (PROTECAO_ATIVA) {
        integral -= integral >> 3; // Subtrai integral/8

        if(V_PID > I_FPB_STEP) V_PID -= I_FPB_STEP;
        else V_PID = 0;
    }
    else {
        if (V_PID < V_ref) {
            V_PID += V_FPB_STEP;
            if (V_PID > V_ref) V_PID = V_ref;
        }
        else if (V_PID > V_ref) {
            V_PID -= V_FPB_STEP;
            if (V_PID < V_ref) V_PID = V_ref;
        }
    }

    /* ---------- Controle de tensao ---------- */
    erro = V_PID - V_lido;

    saida = ((KP * erro) + (KI * integral)) >> SHIFT;

    /* ---------- Saturacao e anti windup ---------- */
    if(saida < PWM_MIN) saida = 0;
    else if (saida > PWM_MAX) saida = TIM1_ARR+1;
    else if (!PROTECAO_ATIVA) {
        integral += erro;

        if (integral > INT_MAX)         integral = INT_MAX;
        else if (integral < INT_MIN)    integral = INT_MIN;
    }

    TIM_SetCompare3 (TIM1, (u16)saida);
    ADC_COMPLETE = RESET;
}

void Verifica_RX (void) {
    uint8_t len = RxSize - DMA_GetCurrDataCounter (DMA1_Channel5);  // Quantos bytes chegaram

    /* Coloca terminador */
    if (len >= RxSize) {
        len = RxSize - 1;
        RxBuffer[len] = '\0';
    }

    /* Faz o parse da mensagem */
    if (sscanf ((char *)RxBuffer, "V:%04u,I:%04u", &V_recebido, &I_recebido) == 2) {
        V_ref = V_recebido;
        I_ref = I_recebido;
    }

    /* Rearma DMA */
    DMA_SetCurrDataCounter (DMA1_Channel5, RxSize);
    DMA_Cmd (DMA1_Channel5, ENABLE);
    Rx_pronto = RESET;
}

// ======== Interrupt ======== //


void DMA1_Channel1_IRQHandler (void) {
    // ADC
    DMA_ClearITPendingBit (DMA1_IT_TC1);
    V_lido = adc_buf[0];
    I_lido = adc_buf[1];
    ADC_COMPLETE = SET;
}

void USART1_IRQHandler (void) {
    if (USART_GetITStatus (USART1, USART_IT_IDLE)) {
        DMA_Cmd (DMA1_Channel5, DISABLE);  // Para DMA
        Rx_pronto = SET;

        /* Limpa a flag IDLE */
        USART1->STATR;
        USART1->DATAR;
    }
}

// ======== Inicializar sistema ======== //

void Custom_Option_Byte_CFG (void) {
    // Desabilita pino do reset
    FLASH_Unlock();
    FLASH_EraseOptionBytes();
    FLASH_UserOptionByteConfig (OB_IWDG_SW, OB_STDBY_NoRST, OB_RST_NoEN, OB_PowerON_Start_Mode_BOOT);
    FLASH_Lock();
}

void Custom_TIM1_CFG (void) {
    TIM_TimeBaseInitTypeDef TIM1_TimeBaseInitStructure = {0};
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_TIM1, ENABLE);

    TIM1_TimeBaseInitStructure.TIM_Prescaler = TIM1_PSC;
    TIM1_TimeBaseInitStructure.TIM_Period = TIM1_ARR;
    TIM1_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM1_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit (TIM1, &TIM1_TimeBaseInitStructure);

    TIM_Cmd (TIM1, ENABLE);
}

void Custom_OPA1_CFG (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    OPA_InitTypeDef OPA_InitStructure = {0};
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOD, &GPIO_InitStructure);
    GPIO_PinLockConfig (GPIOD, GPIO_Pin_7);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    OPA_InitStructure.PSEL = CHP1;
    OPA_InitStructure.NSEL = CHN1;
    OPA_Init (&OPA_InitStructure);

    OPA_Cmd (ENABLE);
}

void Custom_ADC_CFG (uint16_t CCR) {
    // -------- Trigger CFG --------

    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse = CCR;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init (TIM1, &TIM_OCInitStructure);  // CC2 confgiurado como referencia
    TIM_OC2PreloadConfig (TIM1, TIM_OCPreload_Enable);


    // -------- DMA CFG --------

    DMA_InitTypeDef DMA_InitStructure = {0};
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit (DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->RDATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)adc_buf;  // 2 valores (scan length = 2)
    DMA_InitStructure.DMA_BufferSize = size (adc_buf);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;      // Cont??nuo
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  // A USART RX tem prioridade maior
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init (DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd (DMA1_Channel1, ENABLE);

    // -------- Interrupt --------

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);
    DMA_ITConfig (DMA1_Channel1, DMA_IT_TC, ENABLE);

    // -------- ADC CFG --------

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig (RCC_PCLK2_Div2);

    GPIO_InitStructure.GPIO_Pin = V_pin | I_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    ADC_DeInit (ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                           // Le os dois canais em sequencia
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                     // DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC2;  // Trigger
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_Init (ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig (ADC1, V_ADC, 1, V_sample);
    ADC_RegularChannelConfig (ADC1, I_ADC, 2, I_sample);

    ADC_Calibration_Vol (ADC1, ADC_CALVOL_50PERCENT);
    ADC_DMACmd (ADC1, ENABLE);
    ADC_Cmd (ADC1, ENABLE);

    ADC_ResetCalibration (ADC1);
    while (ADC_GetResetCalibrationStatus (ADC1));
    ADC_StartCalibration (ADC1);
    while (ADC_GetCalibrationStatus (ADC1));

    ADC_Cmd (ADC1, ENABLE);
}

void Custom_PWM_CFG (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    GPIO_PinRemapConfig (GPIO_PartialRemap1_TIM1, ENABLE);

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC3Init (TIM1, &TIM_OCInitStructure);

    TIM_OC3PreloadConfig (TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM1, ENABLE);

    TIM_CtrlPWMOutputs (TIM1, ENABLE);
}

void Custom_USARTx_CFG (void) {
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    // -------- DMA TX --------

    DMA_InitTypeDef DMA_TX_InitStructure = {0};
    DMA_DeInit (DMA1_Channel4);
    DMA_TX_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);
    DMA_TX_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;
    DMA_TX_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TX_InitStructure.DMA_BufferSize = TxSize;
    DMA_TX_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_TX_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_TX_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_TX_InitStructure.DMA_Mode = DMA_Mode_Circular;     // Envia pra sempre e foda-se
    DMA_TX_InitStructure.DMA_Priority = DMA_Priority_Low;  // Prioridade mais baixa do projeto
    DMA_TX_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init (DMA1_Channel4, &DMA_TX_InitStructure);


    // -------- DMA RX --------

    DMA_InitTypeDef DMA_RX_InitStructure = {0};
    DMA_DeInit (DMA1_Channel5);
    DMA_RX_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);
    DMA_RX_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;
    DMA_RX_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_RX_InitStructure.DMA_BufferSize = RxSize;
    DMA_RX_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_RX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_RX_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_RX_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_RX_InitStructure.DMA_Mode = DMA_Mode_Normal;            // Recebe uma vez
    DMA_RX_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  // Prioridade mais alta do projeto
    DMA_RX_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init (DMA1_Channel5, &DMA_RX_InitStructure);

    NVIC_InitTypeDef NVIC_RX_InitStructure = {0};
    NVIC_RX_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_RX_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_RX_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_RX_InitStructure.NVIC_IRQChannelSubPriority = ENABLE;
    NVIC_Init (&NVIC_RX_InitStructure);

    // -------- USART --------
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // Pull UP para RX
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init (USART1, &USART_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    USART_ITConfig (USART1, USART_IT_IDLE, ENABLE);

    DMA_Cmd (DMA1_Channel4, ENABLE); /* USART1 Tx */
    DMA_Cmd (DMA1_Channel5, ENABLE); /* USART1 Rx */

    USART_Cmd (USART1, ENABLE);
}
