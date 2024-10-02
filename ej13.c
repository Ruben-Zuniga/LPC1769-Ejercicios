/*
===============================================================================
 Name        : Ejercicio 13
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : 	Calcular cuál es el tiempo máximo que se puede temporizar utilizando un timer en
				modo match con máximo valor de prescaler y máximo divisor de frecuencia de
				periférico. Especificar el valor a cargar en los correspondientes registros del timer.
				Suponer una frecuencia de core cclk de 50 Mhz.
===============================================================================
*/

/*
 *  Times calculation
 *  f(cclk) = 50 MHz; f(pclk) = f(cclk)/8; PR = 2^32-1
 *  --> TC increments on every PR+1/PCLK = 11hs 27min 11,69sec
 */

#include "LPC17xx.h"

#define PRESCALER 2^(32)-1	// Prescaler value for Timer0
#define MATCH 2^(32)-1      // Match value for Timer0

void configTimer0(void);

int main(void)
{
	SystemInit();
    configTimer0();

    while (1);
    return 0;
}

void configTimer0(void)
{
    LPC_PINCON->PINSEL3 |= (3 << 24); // P1.28 as MAT0.0

    LPC_SC->PCONP |= (1 << 1);	// Enable Timer0
    LPC_SC->PCLKSEL0 |= (3 << 2); // PCLK = cclk/8

    LPC_TIM0->PR = PRESCALER;
    LPC_TIM0->MR0 = MATCH;
    LPC_TIM0->MCR = 1;         // Timer0 interrupt on Match0
    LPC_TIM0->EMR |= (3 << 4); // MAT0.0 toggle mode
    LPC_TIM0->IR |= 0x3F;      // Clear all interrupt flags
    LPC_TIM0->TCR = 3;         // Enable and Reset=1
    LPC_TIM0->TCR &= ~2;	   // Reset=0
}
