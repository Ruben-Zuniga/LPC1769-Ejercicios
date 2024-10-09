/*
===============================================================================
 Name        : Ejercicio propuesto 2
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description :  Dada dos señal de 50khz periódicas de componente espectral máxima que
                ingresan por pines del ADC, se necesita convolucionarlas en el dominio del
                tiempo (las señales son de secuencias sincronizadas) y almacenarlas. El rango
                dinámico de amplitud es de 3.3v.
                Escriba el programa de la manera más conveniente. Definir si burst o software,
                tiempos de sampleo, tipo de variables.
                Encontrar el patrón de cada secuencia y hacer la convolución para un período.
===============================================================================
*/

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#define N_BUFFER 17
#define PRESCALE SystemCoreClock / 200e3 - 1

void config_pins(void);
void config_timer(void);
void config_adc(void);

int main(void)
{
	SystemInit();
	config_pins();
	config_adc();
	config_timer();

    while (1);
    return 0;
}

void TIMER0_IRQHandler(){
	ADC_StartCmd(LPC_TIM0, ADC_START_NOW);  // start conversion on both channels
	TIM_ClearIntPending(LPC_TIM0, 0);	// clear int flag
}

void ADC_IRQHandler(){
    static uint8_t n = 0;
    static uint8_t i = 0;
    static uint8_t j = 0;
    static uint8_t k = 0;
    static uint16_t x[(N_BUFFER+1)/2];  // ???
    static uint16_t h[(N_BUFFER+1)/2];
    static uint16_t y[];

    x[n] = ADC_ChannelGetData(LPC_ADC, 0);
    while(!ADC_ChannelGetStatus(LPC_ADC, 1, 1));    // ???
    h[n] = ADC_ChannelGetData(LPC_ADC, 1);

    n++;
    if(n >= N_BUFFER){
        n = 0;
        for(i = 0; i < N_BUFFER; i++){
            for(k = 0; k < i; k++){
                y[j] = x[k] * h[i-k];
                k++;
                j++;
            }
        }
    }
}

void config_pins(){
	// ADC Input
	PINSEL_CFG_Type pin_cfg0_23 = {0, 23, 1, 2, 0};	// (port0, pin23, ADC0.0, no pull resistor, No opendrain)
	PINSEL_ConfigPin(&pin_cfg0_23);

	PINSEL_CFG_Type pin_cfg0_24 = {0, 24, 1, 2, 0};	// (port0, pin24, ADC0.1, no pull resistor, No opendrain)
	PINSEL_ConfigPin(&pin_cfg0_24);
}

void config_timer(){
	TIM_TIMERCFG_Type tim0_cfg = {0, {0,0,0}, PRESCALE};   // prescale value in ticks

	TIM_MATCHCFG_Type mat0_cfg = {0, 1, 0, 1, 0, {0,0,0}, 0};  
    // (ch0, enable interrupt, disable stop, enable reset, disable ext match, reserved, match value)

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &tim0_cfg);
	TIM_ConfigMatch(LPC_TIM0, &mat0_cfg);
	TIM_Cmd(LPC_TIM0, 1);	// start timer counter
}

void config_adc(){
	ADC_Init(LPC_ADC, 200e3);	// 200 kHz
	ADC_ChannelCmd(LPC_ADC, 0, 1);	// enable ch0 convertion
	ADC_IntConfig(LPC_ADC, 0, 1);	// enable ch0 interrupt
	ADC_BurstCmd(LPC_ADC, 0);	// disable burst

	NVIC_EnableIRQ(ADC_IRQn);
}



