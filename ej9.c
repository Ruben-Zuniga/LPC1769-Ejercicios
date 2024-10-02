/*
===============================================================================
 Name        : Ejercicio 9
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : Tecla A con bloq mayus
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>

int main(void) {
	SystemInit();

    LPC_PINCON -> PINSEL1 &= ~0xffff;
    LPC_PINCON -> PINSEL4 |= ((1 << 20) | (1 << 22));
    LPC_PINCON -> PINSEL4 &= ~((1 << 21) | (1 << 23));

    LPC_PINCON -> PINMODE4 &= ~(0xf << 20);

    LPC_GPIO1 -> FIODIR &= ~(0xff << 16);
    LPC_GPIO1 -> FIOMASK = ~(0xff << 16);
    LPC_GPIO2 -> FIOMASK = ~(1 << 10);

    LPC_SC -> EXTMODE &= ~1;
    LPC_SC -> EXTMODE |= (1 << 1);

    LPC_SC -> EXTPOLAR |= 3;

    NVIC -> IP[18] = (2 << 3);
    NVIC -> IP[19] = (1 << 3);
    NVIC -> ISER[0] |= (3 << 18);

    while(1);
    return 0;
}

void EINT0_IRQHandler(){
    while (LPC_GPIO2 -> FIOPIN == (1 << 10));

    LPC_SC -> EXTINT |= 1;
}

void EINT1_IRQHandler(){
    if((LPC_SC -> EXTINT & 1) == 1)
        LPC_GPIO1 -> FIOPIN = (0x41 << 16);

    else
        LPC_GPIO1 -> FIOPIN = (0x61 << 16);

    LPC_SC -> EXTINT |= (1 << 1);
}


