/*
===============================================================================
 Name        : Ejercicio 1
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : 10 salidas con dos secuencias alternadas
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>

#define PERIOD 100 // [ms]

void secA(void);
void secB(void);
int curr_sec = 0;

int main(void) {
	SystemInit();
	LPC_PINCON 	-> PINSEL0 	&= ~(0x3ff);	// P0.0 to P0.9 as GPIO

	LPC_GPIO0 	-> FIODIR 	|= 0x3ff;	// P0.0 to P0.9 out
	LPC_GPIO0 	-> FIOMASK 	|= 0x3ff;

	SysTick 	-> LOAD 	= (SystemCoreClock / 100) - 1;
	SysTick 	-> CTRL 	= 7;
	SysTick 	-> VAL 		= 0;
	NVIC_SetPriority(SysTick_IRQn, 3);

    while(1);
    return 0;
}

void SysTick_Handler(){
	static uint32_t cnt = 0;
	cnt++;

	if(cnt >= PERIOD/10){
		if(curr_sec)
			secA();
		else
			secB();
	}

	SysTick -> CTRL &= SysTick -> CTRL;
}






