/*
===============================================================================
 Name        : Ejercicio propuesto por el profe
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : dos salidas con comportamiento segun una entrada
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>

// TODO: insert other definitions and declarations here
uint32_t cnt_5 = 0;
uint32_t cnt_25 = 0;

void A_sequence(void);
void B_sequence(void);

int main(void) {
	SystemInit();
	LPC_PINCON 	-> PINSEL1 	&= (~(0b11 << 6) & ~(0b11 << 14))	;	// P0.19 P0.23 as GPIO
	LPC_PINCON 	-> PINSEL2 	&= ~(0b11 << 30)					;	// P1.15 as GPIO

	LPC_GPIO0 	-> FIODIR 	|= (1 << 19) & ~(1 << 23)			;	// P0.19 out; P0.23 in
	LPC_GPIO1 	-> FIODIR 	|= (1 << 15)						;	// P1.15 out
	//LPC_GPIO0 	-> FIOMASK 	= ~((1 << 19) | (1 << 23))		;

	SysTick 	-> LOAD 	= (SystemCoreClock / 200) - 1		;
	SysTick 	-> CTRL 	= 7									;
	SysTick 	-> VAL 		= 0									;
	NVIC_SetPriority(SysTick_IRQn, 5)							;

    while(1);
    return 0 ;
}

void A_sequence(){
	if(cnt_25 == 7){
		LPC_GPIO0 -> FIOPIN = ~(LPC_GPIO0 -> FIOPIN);
		LPC_GPIO1 -> FIOPIN = (LPC_GPIO1 -> FIOPIN);
	}
	else{
		if(cnt_25 == 14)
			cnt_25 = 0;
		LPC_GPIO0 -> FIOPIN = (LPC_GPIO0 -> FIOPIN);
		LPC_GPIO1 -> FIOPIN = ~(LPC_GPIO1 -> FIOPIN);
	}
}

void B_sequence(){
	if(cnt_25 == 7){
		LPC_GPIO0 -> FIOPIN = (LPC_GPIO0 -> FIOPIN);
		LPC_GPIO1 -> FIOPIN = ~(LPC_GPIO1 -> FIOPIN);
	}
	else{
		if(cnt_25 == 14)
			cnt_25 = 0;
		LPC_GPIO0 -> FIOPIN = ~(LPC_GPIO0 -> FIOPIN);
		LPC_GPIO1 -> FIOPIN = (LPC_GPIO1 -> FIOPIN);
	}
}

void SysTick_Handler(){
	LPC_GPIO0 -> FIOMASK = ~(1 << 23);
	static uint32_t previous_input = 0;
	static uint32_t current_input = 0;

	cnt_5++;
	if(cnt_5 == 5){
		cnt_5 = 0;
		cnt_25++;

		previous_input = current_input;
		current_input = LPC_GPIO0 -> FIOPIN;

		LPC_GPIO0 -> FIOMASK = ~(1 << 19);
		LPC_GPIO1 -> FIOMASK = ~(1 << 15);

		if(current_input != previous_input){
			cnt_5 = 0;
			cnt_25 = 0;
		}
		if(current_input == (1 << 23))
			B_sequence();
		else
			A_sequence();

		SysTick -> CTRL &= SysTick -> CTRL;
	}
	else
		SysTick -> CTRL &= SysTick -> CTRL;
}






