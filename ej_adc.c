/*
===============================================================================
 Name        : Ejercicio de clase
 Author      : $Fabio
 Version     :
 Copyright   : $(copyright)
 Description :	Implementar un sistema que lea el valor de un sensor de temperatura
 	 	 	 	analógico conectado al canal 0 del ADC, configurado para generar
 	 	 	 	una conversión periódica mediante un temporizador. El sistema debe
 	 	 	 	levantar una interrupción cuando la conversión ADC esté lista, y en
 	 	 	 	base al valor de temperatura, tomar decisiones. Si la temperatura
 	 	 	 	supera un umbral predefinido, se debe encender un LED rojo
 	 	 	 	(simulando una alarma), sino un LED verde. Tener en cuenta los
 	 	 	 	valores de referencia del ADC
===============================================================================
*/

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#define TEMP_LIMIT 30	// Temperature threshold. If the temp measured is greater
						// than TEMP_LIMIT, a red led turns on. If it's less, a
						// green led turns on.

void config_pins(void);
void config_timer0(void);
void config_adc(void);

volatile uint16_t adc_val = 0;

int main(void)
{
	SystemInit();
	config_pins();
	config_adc();
	config_timer0();

    while (1);
    return 0;
}

void TIMER0_IRQHandler(){
	ADC_StartCmd(LPC_TIM0, ADC_START_NOW);
	TIM_ClearIntPending(LPC_TIM0, 0);	// (MR0 flag)
}

void ADC_IRQHandler(){
	static uint16_t temp_val = 0;

	adc_val = ADC_ChannelGetData(LPC_ADC, 0);	// (ch0)
	// adc_val = [0 , 4096]
	// assuming a sensor for temps between 0°-100°
	temp_val = adc_val * 100 / 4095;

	if(temp_val > TEMP_LIMIT){
		GPIO_SetValue(2, 1<<1);	// Red led on
		GPIO_ClearValue(2, 1);	// Green led off
	}
	else{
		GPIO_SetValue(2, 1);	// Green led on
		GPIO_ClearValue(2, 1<<1);	// Red led off
	}
}

void config_pins(){
	// ADC Input
	PINSEL_CFG_Type pin_cfg = {0, 23, 1, 3, 0};	// (port0, pin23, ADC0.0, pull down, No opendrain)
	PINSEL_ConfigPin(&pin_cfg);

	// Leds
	GPIO_SetDir(0, 3, 1);	// (port0, pins 0-1, output)
}

void config_timer0(){
	TIM_TIMERCFG_Type timer_cfg;
	timer_cfg.PrescaleOption = TIM_PRESCALE_USVAL;	// PrescaleValue in us
	timer_cfg.PrescaleValue = 1e3;	// increases every 1 ms

	TIM_MATCHCFG_Type match_cfg;
	match_cfg.MatchChannel = 0;	// MAT0
	match_cfg.ExtMatchOutputType = 3;	// Toggle output on match
	match_cfg.MatchValue = 500;	// Interrupts every 500 ms
	match_cfg.IntOnMatch = 1;	// Enable interrupt
	match_cfg.ResetOnMatch = 1;	// Enable reset
	match_cfg.StopOnMatch = 0;	// Disable stop

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg);
	TIM_ConfigMatch(LPC_TIM0, &match_cfg);
	TIM_Cmd(LPC_TIM0, 1);	// (on)
}

void config_adc(){
	ADC_Init(LPC_ADC, 200e3);	// (200 kHz)
	ADC_ChannelCmd(LPC_ADC, 0, 1);	// (ch0, on)
	ADC_IntConfig(LPC_ADC, 0, 1);	// (ch0, on)
	ADC_BurstCmd(LPC_ADC, 0);	// (off)

	NVIC_EnableIRQ(ADC_IRQn);
}



