/*
===============================================================================
 Name        : Ejercicio de clase
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description :  1. Generar con timer0 una señal de freq. variable.
                2. Usando el capture “medir”el periodo usando otro timer.
                3. Prender leds tipo vúmetro según la frecuencia.
                4. Con un pulsador cambiar la frecuencia de pasos de 100khz. Actualizar el
                vúmetro.
===============================================================================
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"

#define PRESCALE SystemCoreClock / 10e6 - 1
#define MATCH_N 4

void config_pins(void);
void config_EINT0(void);
void config_TIMER0(void);
void config_TIMER1(void);

int main(void)
{
	SystemInit();
    config_pins();
    config_EINT0();
    config_TIMER0();
    config_TIMER1();

    while (1);
    return 0;
}

void config_pins(){
	PINSEL_CFG_Type p0_0_cfg 	= {0, 0,  0, 2, 0};		// P0.0 to P0.3 as GPIO
	PINSEL_ConfigPin(&p0_0_cfg);
	PINSEL_CFG_Type p0_1_cfg 	= {0, 1,  0, 2, 0};
	PINSEL_ConfigPin(&p0_1_cfg);
	PINSEL_CFG_Type p0_2_cfg 	= {0, 2,  0, 2, 0};
	PINSEL_ConfigPin(&p0_2_cfg);
	PINSEL_CFG_Type p0_3_cfg 	= {0, 3,  0, 2, 0};
	PINSEL_ConfigPin(&p0_3_cfg);
	GPIO_SetDir(0, 0xf, 1);								// P0.0 to P0.3 as output
	//FIO_SetMask(0, ~0xf, 1);							// Only P0.0 to P0.3 are affected by FIOPIN

	PINSEL_CFG_Type p1_28_cfg 	= {1, 28, 3, 2, 0};		// P1.28 as MAT0.0
	PINSEL_ConfigPin(&p1_28_cfg);

	PINSEL_CFG_Type p1_18_cfg 	= {1, 18, 3, 3, 0};		// P1.18 as CAP1.0 with pull down
	PINSEL_ConfigPin(&p1_18_cfg);

	PINSEL_CFG_Type p2_10_cfg 	= {2, 10, 1, 3, 0};		// P2.10 as EINT0 with pull down
	PINSEL_ConfigPin(&p2_10_cfg);
}

void config_EINT0(){
	EXTI_InitTypeDef EINT0_cfg = {0, 1, 1};		// EINT0 is rising edge sensitive
	EXTI_Config(&EINT0_cfg);
}

void config_TIMER0(){
	LPC_SC->PCONP 		|= (1 << 1);			// Timer0 on
	LPC_SC->PCLKSEL0 	|= (1 << 2);		// pclk = cclk
	LPC_SC->PCLKSEL0 	&= ~(1 << 3);
	LPC_TIM0->CTCR 		= 0;					// Timer mode
	LPC_TIM0->PR 		= PRESCALE;			// TC increases every 1 us
	LPC_TIM0->MR0 		= 50;					// Initial frequency: 100 kHz
	LPC_TIM0->EMR 		|= (3 << 4);			// Toggle MAT0.0 pin
	LPC_TIM0->MCR 		|= (1 << 1);					// Reset on MAT0.0
	LPC_TIM0->MCR 		&= ~(1 | (1 << 2));
	LPC_TIM0->TCR 		= 3;					// Enable and Reset TC
	LPC_TIM0->TCR 		= 1;
}

void config_TIMER1(){
	LPC_SC->PCONP 		|= (1 << 2);			// Timer1 on
	LPC_SC->PCLKSEL0 	|= (1 << 4);		// pclk = cclk
	LPC_SC->PCLKSEL0 	&= ~(1 << 5);
	LPC_TIM1->CTCR 		= 0;					// Timer mode
	LPC_TIM1->PR 		= PRESCALE;			// TC increases every 1 us
	LPC_TIM1->CCR 		|= 1 | (1 << 2);		// Interrupt on rising edge of CAP1.0
	LPC_TIM1->CCR 		&= ~(1 << 1);
	LPC_TIM1->TCR 		= 3;					// Enable and Reset TC
	LPC_TIM1->TCR 		= 1;
}

void EINT0_IRQHandler(){
	static uint8_t match_div = 1;

	LPC_TIM0->TCR |= (1 << 1);			// Reset TC
	if(match_div >= MATCH_N)			// For every rising edge on EINT0, MAT0.0 multiplies its frequency by match_div+1
										// until match_div = MATCH_N
		match_div = 1;
	else
		match_div++;

	LPC_TIM0->MR0 = 50 / match_div;
	LPC_TIM0->TCR &= ~(1 << 1);			// Resume TC
	EXTI_ClearEXTIFlag(0);
}

void TIMER1_IRQHandler(){
	static uint32_t val = 0;
	static uint32_t val_prev = 0;
	static uint32_t period = 0;
	uint32_t period_350k = SystemCoreClock / (350e3 * (PRESCALE + 1));
	uint32_t period_250k = SystemCoreClock / (250e3 * (PRESCALE + 1));
	uint32_t period_150k = SystemCoreClock / (150e3 * (PRESCALE + 1));
	uint32_t period_50k = SystemCoreClock / (50e3 * (PRESCALE + 1));

	val = LPC_TIM1->CR0;
	period = val - val_prev;
	val_prev = val;

	if(period < period_350k){
		if(period < period_250k){
			if(period < period_150k){
				if(period < period_50k){
					GPIO_ClearValue(0, 0xf);
				}
				else{
					GPIO_SetValue(0, 1);
					GPIO_ClearValue(0, (7 << 1));
				}
			}
			else{
				GPIO_SetValue(0, 3);
				GPIO_ClearValue(0, (3 << 2));
			}
		}
		else{
			GPIO_SetValue(0, 7);
			GPIO_ClearValue(0, (1 << 3));
		}
	}
	else{
		GPIO_SetValue(0, 0xf);
	}
	LPC_TIM1->IR |= (1 << 4);
}








