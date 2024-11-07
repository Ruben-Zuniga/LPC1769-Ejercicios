#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"

#define ADC_FREQ 200000  // Frecuencia de muestreo de 200kHz
#define UART_BAUDRATE 115200 // Tasa de baudios para UART
#define N_CHANNELS 2  // Cantidad de canales a usar

typedef enum{
    CH1 = 0,
    CH2,
    SUM,
    MULTIPLICATION,
} Operation_t;

// Buffers para almacenar datos de los canales
uint16_t adcDataBuffer[N_CHANNELS];
uint16_t adcDataSend[N_CHANNELS];
Operation_t operation = SUM; // Operación actual: 'SUM' para suma, 'MULTIPLICATION' para multiplicación

// Inicialización de UART para transmisión de datos a la PC
void UART_Config(void) {
    PINSEL_CFG_Type pinCfg;
    UART_CFG_Type UARTCfg;

    // Configuración de pines para UART
    pinCfg.Portnum = PINSEL_PORT_0;
    pinCfg.Pinnum = PINSEL_PIN_2;
    pinCfg.Funcnum = PINSEL_FUNC_1; // TXD0
    pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pinCfg);

    pinCfg.Pinnum = PINSEL_PIN_3; // RXD0
    PINSEL_ConfigPin(&pinCfg);

    UARTCfg.Baud_rate = UART_BAUDRATE;
    UARTCfg.Parity = UART_PARITY_NONE;
    UARTCfg.Databits = UART_DATABIT_8;
    UARTCfg.Stopbits = UART_STOPBIT_1;
    UART_Init(LPC_UART0, &UARTCfg);
    UART_TxCmd(LPC_UART0, ENABLE); // Habilita la transmisión UART
}

// Inicialización de ADC en los canales 0 y 1
void ADC_Config(void) {
    ADC_Init(LPC_ADC, ADC_FREQ);
    ADC_BurstCmd(LPC_ADC, DISABLE);
    ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
}

void Timer_Config(void) {
    TIM_TIMERCFG_Type TimerCfg;
    TIM_MATCHCFG_Type MatchCfg;

    TimerCfg.PrescaleOption = TIM_PRESCALE_USVAL;
    TimerCfg.PrescaleValue = 50; // 50 µs

    MatchCfg.MatchChannel = 0;
    MatchCfg.IntOnMatch = ENABLE;
    MatchCfg.StopOnMatch = DISABLE;
    MatchCfg.ResetOnMatch = ENABLE;
    MatchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    MatchCfg.MatchValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerCfg);
    TIM_ConfigMatch(LPC_TIM0, &MatchCfg);

    // Habilita la interrupción del Timer0
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

// Configuración del DAC con BIAS en 0
void DAC_Config(void) {
    DAC_Init(LPC_DAC);
    DAC_SetBias(LPC_DAC, 0); // BIAS en 0
}

// Inicialización del teclado matricial
void Keypad_Config(void) {
    // Configura las filas como salidas y las columnas como entradas
    GPIO_SetDir(0, (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7), 1); // Filas
    GPIO_SetDir(1, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3), 0); // Columnas
}

// Función para detectar teclas en el teclado matricial
void Keypad_Read(void) {
    int row, col;

    // Escanear filas
    for (row = 0; row < 4; row++) {
        GPIO_SetValue(0, (1 << (4 + row))); // Activar la fila actual

        for (col = 0; col < 4; col++) {
            if ((GPIO_ReadValue(1) & (1 << col))) { // Detecta si se ha presionado una tecla en la columna
                // Identifica la tecla presionada
                if (row == 0 && col == 0) {
                    operation = SUM; // Configura para suma si se presiona "+"
                } else if (row == 0 && col == 1) {
                    operation = MULTIPLICATION; // Configura para multiplicación si se presiona "x"
                }
            }
        }

        GPIO_ClearValue(0, (1 << (4 + row))); // Desactivar la fila actual
    }
/*
    // Escanear filas
    for (row = 0; row < 4; row++) {
        GPIO_ClearValue(0, (1 << (4 + row))); // Activar la fila actual

        for (col = 0; col < 4; col++) {
            if (!(GPIO_ReadValue(1) & (1 << col))) { // Detecta si se ha presionado una tecla en la columna
                // Identifica la tecla presionada
                if (row == 2 && col == 1) {
                    operation = SUM; // Configura para suma si se presiona "+"
                } else if (row == 2 && col == 3) {
                    operation = MULTIPLICATION; // Configura para multiplicación si se presiona "x"
                }
            }
        }

        GPIO_SetValue(0, (1 << (4 + row))); // Desactivar la fila actual
    }
*/
}

// Configuración de Timer0 para interrupciones cada 500us
void TIMER0_IRQHandler(void) {
    uint16_t result;

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
        // Inicia la conversión en los canales del ADC
        ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, DISABLE);
        ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);

        // Realiza la operación según la tecla presionada
        switch (operation){
        case CH1:
            result = adcDataBuffer[0];
            break;

        case CH2:
            result = adcDataBuffer[1];
            break;

        case SUM:
            result = adcDataBuffer[0] + adcDataBuffer[1];
            break;
        
        case MULTIPLICATION:
            result = (adcDataBuffer[0] * adcDataBuffer[1]) >> 12; // Ajuste para evitar overflow
            break;

        default:
            result = adcDataBuffer[0];
            break;
        }

        // Enviar el resultado al DAC
        DAC_UpdateValue(LPC_DAC, result);

        // Limpiar la interrupción del Timer
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    }
}

void ADC_IRQHandler(){
    if(ADC_GlobalGetStatus(LPC_ADC, 0)){
        // OVERRUN //
    }

    if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, 1)){
        ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, DISABLE);
        ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);
    }
    else if (ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_1, 1)){
        // Lee el valor convertido de los canales
        adcDataBuffer[0] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        adcDataBuffer[1] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);

        // Escalar datos para que sean de 8 bits
        adcDataSend[0] = adcDataBuffer[0] * 256 / 4096;
        adcDataSend[1] = adcDataBuffer[1] * 256 / 4096;

        // Enviar datos por UART
        UART_Send(LPC_UART0, (uint8_t *)adcDataSend, sizeof(adcDataSend), BLOCKING);
    }
    

}

int main(void) {
    SystemInit();

    // Configurar periféricos
    UART_Config();
    ADC_Config();
    Timer_Config();
    DAC_Config();
    Keypad_Config();

    while (1) { // Leer el teclado para detectar operaciones
        Keypad_Read();
    }
}
