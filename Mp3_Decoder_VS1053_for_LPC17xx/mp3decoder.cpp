#include "FreeRTOS.h"
#include "LPC17xx.h"
#include <stdio.h>
#include "mp3decoder.hpp"
#include <string.h>
#include <iostream>    
#include "lpc_sys.h"
#include "SPI.hpp"

//Serial Protocol for Serial Command Interface (SCI)

LabSPI SPI;

MP3DECODER::MP3DECODER()
{}

MP3DECODER::~MP3DECODER()
{}

bool MP3DECODER::init()
{
SPI.init(SPI.Peripheral::SSP0);

  //cs=P1.23, dcs=P1.22, dreq=P2.6, RESET=P1.28,
  LPC_PINCON->PINSEL3 &= ~((0x3 << 12) | (0x3 << 14) | (0x3 << 24) );
  LPC_PINCON->PINSEL4 &= ~(03 << 12);

  //set cs, dcs, reset to output
  LPC_GPIO1->FIODIR |= ((1 << 23) | (1 << 22) | (1 << 28));
  LPC_GPIO2->FIODIR &= ~(1 << 6); //set dreq to input


  LPC_GPIO1->FIOCLR = (1 << 28);  //ssp
  LPC_GPIO1->FIOSET = (1 << 28); 
  LPC_GPIO1->FIOSET = (1 << 22); 
  LPC_GPIO1->FIOSET = (1 << 23); 

  SW_RST();

  //set the sci MODE
  VolSet(0xfe);
  sci_writeReg(0x0800, MODE);
  sci_writeReg(0x0286, BASS);

sci_writeReg(0xAC45, AUDATA); //44100 sample rate
  
  sci_writeReg(0x6000 , CLOCKF); 
  VolSet(0xfe); //fefe slience

  return true;
}

bool MP3DECODER::Unselect_C(void)
{
  return (LPC_GPIO1->FIOSET = (1 << 23)); //DESELECT CS
}

bool MP3DECODER::Select_C(void)	//SELECT CS
{
  return (LPC_GPIO1->FIOCLR = (1 << 23));
}

bool MP3DECODER::Select_DC(void) //SELECT DCS
{
  return (LPC_GPIO1->FIOCLR = (1 << 22));
}

bool MP3DECODER::Unselect_DC(void) //DESELCT DCS
{
  return (LPC_GPIO1->FIOSET = (1 << 22));
}

bool MP3DECODER::Data_Req_Stat()
{
  bool request = LPC_GPIO2->FIOPIN & (1 << 6);
  if(request)
  {
    return true; 
  }
  else
  {
    return false; 
  }
}

void MP3DECODER::SCI_en(void)
{
  Unselect_DC();
  Select_C();
}

void MP3DECODER::SCI_disable(void)
{
  Unselect_C();
}

void MP3DECODER::SDI_en(void)
{
  Unselect_C();
  Select_DC();
}

void MP3DECODER::SDI_disable(void)
{
  Unselect_DC();
}

void MP3DECODER::SW_RST()
{

	Unselect_DC();
	Select_C();
  	sci_writeReg((1 << 2), MODE);
	Unselect_C();
}


void MP3DECODER::VolSet(uint8_t vol)
{
	Unselect_DC();
	Select_C();
  	uint16_t audioVolume = (vol << 8) | vol;
  	sci_writeReg(audioVolume, VOL);
	Unselect_C();
}


void MP3DECODER::sci_writeReg(uint16_t data, uint8_t addr)
{
	Select_C();
	Unselect_DC();

  	SPI.transfer(WRITE);
  	SPI.transfer(addr);
  	SPI.transfer(data >> 8);
  	SPI.transfer(data & 0xFF);
	
  	Unselect_C(); 
  	while(!Data_Req_Stat()); 
}

uint16_t MP3DECODER::sci_readReg(uint8_t addr)
{
	Unselect_DC();
  	Select_C();

  	SPI.transfer(READ);  
  	SPI.transfer(addr);

  	uint8_t upperbyte = SPI.transfer(0xFF);
  	uint8_t lowerbyte = SPI.transfer(0xFF);
  	uint16_t receive = (upperbyte << 8) | lowerbyte;

	Unselect_C();
  	while(!Data_Req_Stat());
  	return receive;
}

void MP3DECODER::sdi_write(uint8_t data)
{
	SDI_en();
	while(!Data_Req_Stat()); 
	SPI.transfer(data);
	SCI_disable();
}



