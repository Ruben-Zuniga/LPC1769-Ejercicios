/*
===============================================================================
 Name        : Ejercicio 11
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : señal PWM variable con una entrada
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>

uint32_t cnt = 10;
uint32_t DC = 1;

int main(){
    LPC_PINCON -> PINSEL1 &= ~(3 << 12);
    LPC_PINCON -> PINSEL4 |= (1 << 20);
    LPC_PINCON -> PINSEL4 &= (1 << 21);

    LPC_PINMODE -> PINMODE4 &= ~(3 << 20);

    LPC_GPIO0 -> FIODIR |= (1 << 22);
    //LPC_GPIO0 -> FIOMASK = ~(1 << 22);

    LPC_SC -> EXTMODE |= 1;
    LPC_SC -> EXTPOLAR |= 1;

    NVIC_SetPriority(Systick_IRQn, 1);
    NVIC_SetPriority(EINT0_IRQn, 2);

    Systick -> LOAD = SystemCoreClock / 1000 - 1;
    Systick -> CTRL = 7;

    NVIC_EnableIRQ(EINT0_IRQn);

    Systick -> VAL = 0;

    while(1);
    return 0;
}

void Systick_Handler(){
    if(cnt == 10){
        LPC_GPIO0 -> FIOSET |= (1 << 22);
        cnt = 1;
    }
    else{
        if(cnt >= DC)
            LPC_GPIO0 -> FIOCLR |= (1 << 22);
        cnt++;
    }
    Systick -> CTRL &= Systick -> CTRL;
}

void EINT0_Handler(){
    if(DC == 10)
        DC = 1;
    else
        DC++;

    LPC_SC -> EXTINT |= 1;
}











