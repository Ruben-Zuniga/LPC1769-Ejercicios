#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"

#define ADC_FREQ 200000  // Frecuencia de muestreo de 200kHz
#define UART_BAUDRATE 115200 // Tasa de baudios para UART

// Buffers para almacenar datos de los canales
uint16_t adcDataBuffer[2];
uint8_t operation = '+'; // Operación actual: '+' para suma, 'x' para multiplicación

// Inicialización de UART para transmisión de datos a la PC
void UART_Config(void) {
    PINSEL_CFG_Type PinCfg;
    UART_CFG_Type UARTConfigStruct;

    // Configuración de pines para UART
    PinCfg.Funcnum = 1;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 2; // TXD0
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 3; // RXD0
    PINSEL_ConfigPin(&PinCfg);

    UARTConfigStruct.Baud_rate = UART_BAUDRATE;
    UARTConfigStruct.Databits = UART_DATABIT_8;
    UARTConfigStruct.Parity = UART_PARITY_NONE;
    UARTConfigStruct.Stopbits = UART_STOPBIT_1;
    UART_Init(LPC_UART0, &UARTConfigStruct);
    UART_TxCmd(LPC_UART0, ENABLE); // Habilita la transmisión UART
}

// Inicialización de ADC en los canales 0 y 1
void ADC_Config(void) {
    ADC_Init(LPC_ADC, ADC_FREQ);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
}

// Configuración de Timer0 para interrupciones cada 500us
void TIMER0_IRQHandler(void) {
    uint16_t result;

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
        // Inicia la conversión en los canales del ADC
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);

        // Lee el valor convertido de los canales
        adcDataBuffer[0] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        adcDataBuffer[1] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);

        // Enviar datos por UART
        UART_Send(LPC_UART0, (uint8_t *)adcDataBuffer, sizeof(adcDataBuffer), BLOCKING);

        // Realiza la operación según la tecla presionada
        if (operation == '+') {
            result = adcDataBuffer[0] + adcDataBuffer[1];
        } else if (operation == 'x') {
            result = (adcDataBuffer[0] * adcDataBuffer[1]) >> 12; // Ajuste para evitar overflow
        }

        // Enviar el resultado al DAC
        DAC_UpdateValue(LPC_DAC, result);

        // Limpiar la interrupción del Timer
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    }
}

void Timer_Config(void) {
    TIM_TIMERCFG_Type TimerConfig;
    TIM_MATCHCFG_Type MatchConfig;

    TimerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
    TimerConfig.PrescaleValue = 500; // 500 µs

    MatchConfig.MatchChannel = 0;
    MatchConfig.IntOnMatch = ENABLE;
    MatchConfig.ResetOnMatch = ENABLE;
    MatchConfig.StopOnMatch = DISABLE;
    MatchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    MatchConfig.MatchValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TimerConfig);
    TIM_ConfigMatch(LPC_TIM0, &MatchConfig);

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
        GPIO_ClearValue(0, (1 << (4 + row))); // Activar la fila actual

        for (col = 0; col < 4; col++) {
            if (!(GPIO_ReadValue(1) & (1 << col))) { // Detecta si se ha presionado una tecla en la columna
                // Identifica la tecla presionada
                if (row == 2 && col == 1) {
                    operation = '+'; // Configura para suma si se presiona "+"
                } else if (row == 2 && col == 3) {
                    operation = 'x'; // Configura para multiplicación si se presiona "x"
                }
            }
        }

        GPIO_SetValue(0, (1 << (4 + row))); // Desactivar la fila actual
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