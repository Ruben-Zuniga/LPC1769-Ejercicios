/*
===============================================================================
 Name        : Ejercicio 6
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : 2 salidas con secuencia segun interrupcion por flanco de 2 entradas
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>

void P0_0_sequence(void);
void P0_1_sequence(void);

int main(void) {
	SystemInit();
    LPC_PINCON 	-> PINSEL0 		&= ~0xf;
    LPC_PINCON 	-> PINSEL4 		&= ~0xf;
    LPC_PINCON 	-> PINMODE4 	&= ~0xf;

    LPC_GPIO0 	-> FIODIR 		|= 3;
    LPC_GPIO2 	-> FIODIR 		&= ~3;

    NVIC 		-> ISER[0] 		|= (1<<21);
    NVIC 		-> IP[21] 		|= (5<<3);

    LPC_GPIOINT -> IO2IntEnR	|= 1;
    LPC_GPIOINT -> IO2IntEnF 	|= (1<<1);

    while(1);
    return 0;
}

void P0_0_sequence(){
    LPC_GPIO0 -> FIOCLR |= 1;
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOSET |= 1;
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOCLR |= 1;
    for(int i = 0; i < 2000000; i++);

    LPC_GPIO0 -> FIOSET |= 1;
    for(int i = 0; i < 2000000; i++);

    LPC_GPIO0 -> FIOCLR |= 1;
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOSET |= 1;
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOCLR |= 1;
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOSET |= 1;
}

void P0_1_sequence(){
    LPC_GPIO0 -> FIOCLR |= (1<<1);
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOSET |= (1<<1);
    for(int i = 0; i < 3000000; i++);

    LPC_GPIO0 -> FIOCLR |= (1<<1);
    for(int i = 0; i < 2000000; i++);

    LPC_GPIO0 -> FIOSET |= (1<<1);
    for(int i = 0; i < 2000000; i++);

    LPC_GPIO0 -> FIOCLR |= (1<<1);
    for(int i = 0; i < 1000000; i++);

    LPC_GPIO0 -> FIOSET |= (1<<1);
}

void EINT3_IRQHandler(){
    if((LPC_GPIOINT -> IO2IntStatR & 1) == 1){
        P0_0_sequence();
        LPC_GPIOINT -> IO2IntClr |= 1;
    }
    else if ((LPC_GPIOINT -> IO2IntStatF & (1<<1)) == (1<<1)){
        P0_1_sequence();
        LPC_GPIOINT -> IO2IntClr |= (1<<1);
    }
}





