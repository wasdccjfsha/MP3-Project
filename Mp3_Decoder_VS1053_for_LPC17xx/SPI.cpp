#include "SPI.hpp"
#include <stdio.h>
#include "LPC17xx.h"
#include "io.hpp"

bool LabSPI::init(Peripheral peripheral)
{

    LPC_SC->PCONP |= (1 << 21); 

    LPC_SC->PCLKSEL1 &= ~(3 << 10);
    LPC_SC->PCLKSEL1 |= (0 << 10);

    LPC_PINCON->PINSEL1 &= ~((3 << 2) | (3 << 4));
    LPC_PINCON->PINSEL1 |= ((2 << 2) | (2 << 4));

    LPC_PINCON->PINSEL0 &= ~(3 << 30);
    LPC_PINCON->PINSEL0 |= (2 << 30);


    LPC_GPIO0->FIOPIN |= (1<<16); 
    LPC_PINCON->PINSEL1 &= ~(3 << 0);
    LPC_GPIO0->FIODIR |= (1<<16);
    LPC_GPIO0->FIOSET = (1<<16);

    LPC_SSP0->CR0 = 7; 			
    LPC_SSP0->CR1 = (1<<1);
    LPC_SSP0->CPSR = 4; 		

    return true;
}

uint8_t LabSPI::transfer(uint8_t send)
{
LPC_SSP0->DR = send;

while(LPC_SSP0 ->SR & (1<<4));
return LPC_SSP0->DR;
}

LabSPI::LabSPI()
{}

LabSPI::~LabSPI()
{}
