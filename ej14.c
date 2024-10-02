/*
===============================================================================
 Name        : Ejercicio 14
 Author      : $Ruben
 Version     :
 Copyright   : $(copyright)
 Description : 	Utilizando el Timer 1, escribir un código en C para que por cada presión de un
                pulsador, la frecuencia de parpadeo de un led disminuya a la mitad debido a la
                modificación del reloj que llega al periférico. El pulsador debe producir una
                interrupción por GPIO0 con flanco descendente.
===============================================================================
*/

#include "LPC17xx.h"

#define TC_PERIOD   1e-3 // Timer Counter increases every 1 ms
#define TLED        1000 // In (TC_PERIOD * TLED) seconds the led toggles

void config_pins (void);
void config_timer1 (void);

int main(void)
{
	SystemInit();
    config_pins();
    config_timer1();

    NVIC_SetPriority(EINT3_IRQn, 0);
    NVIC_EnableIRQ(EINT3_IRQn);

    while (1);
    return 0;
}

void config_pins(){
    LPC_PINCON->PINSEL0     &= ~3;                              // P0.0 as GPIO
    LPC_PINCON->PINMODE0    &= ~3;                              // Pull up resistor for P0.0
    LPC_GPIO0->FIODIR       &= ~1;                              // P0.0 as input
    LPC_GPIOINT->IO0IntEnF  |= 1;                               // Enable falling edge interrupt for P0.0

    LPC_PINCON->PINSEL3     |= (3 << 12);                       // P1.22 as external Match0 pin for Timer1
}

void config_timer1(){
    LPC_SC->PCONP           |= (1 << 2);	                    // Enable Timer1
    LPC_SC->PCLKSEL0        |= (1 << 4);                        // pclk = cclk
    LPC_SC->PCLKSEL0        &= ~(1 << 5);

    LPC_TIM1->CTCR          &= ~3;                              // Timer mode
    LPC_TIM1->PR            = TC_PERIOD * SystemCoreClock - 1;  // TC increases every 1 ms
    LPC_TIM1->MR0           = TLED;                             // In (TC_PERIOD * TLED) seconds the led toggles
    LPC_TIM1->MCR           &= ~5;                              // Timer1 reset on Match0;
    LPC_TIM1->EMR           |= (3 << 4);                        // MAT1.0 toggle mode
    LPC_TIM1->IR            |= 0x3F;                            // Clear all interrupt flags
    LPC_TIM1->TCR           = 3;                                // Enable=1 and Reset=1
    LPC_TIM1->TCR           &= ~(1 << 1);	                    // Reset=0
}

void EINT3_IRQHandler(){
    if(LPC_TIM1->MR0 == 0)                                      // Divide led frequency by 2
        LPC_TIM1->MR0       = TLED;
    else
        LPC_TIM1->MR0       /= 2;

    LPC_TIM1->TCR           |= (1 << 1);                        // Reset Timer Counter
    LPC_TIM1->TCR           &= ~(1 << 1);

    LPC_GPIOINT->IO0IntClr  |= 1;                               // Clear EINT3 flag
}
