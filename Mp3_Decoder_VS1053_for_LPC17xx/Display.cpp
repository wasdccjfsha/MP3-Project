#include "Display.hpp"

void Display::init(char a[],char t[])
{
	set(a,t);
	uart.init_uart(9600);
	uart.transmit(0x12);
	delay_ms(5);	
	titleLength = 0;
	artistLength = 0;	
	volume = 0;
	titleCursor = 0;
	updateMode = 0;
	updateFlag = 0;
	previousVolume = 0;
	volumeLoopCount = 0;
	circle = false;
	volumeLoopLimit = 2;
	vol = false;
	initBuffer();
}

void Display::clearScreen(){
	uart.transmit(0xFE);
	uart.transmit(0x01);
	delay_ms(5);
}
void Display::setCursor(int row,int col){
	uart.transmit(0xFE);
	uart.transmit(cursorArray[row*16+col]);
	delay_ms(5);

}

void Display::update()
{
	clearScreen();
		setCursor(0,0);
		if((updateMode==1) || (updateMode==3)){
			//printf("Buffer1: %s\n",displayBuffer1);
			for(int i = 0;i<16;i++)
			{
				uart.transmit(displayBuffer1[i]);
			}
			delay_ms(5);
		}
		setCursor(1,0);
		if(updateMode==2 || updateMode==3){
			for(int j = 0;j<16;j++)
			{
				
			//printf("Buffer1: %s\n",displayBuffer2);
				uart.transmit(displayBuffer2[j]);
			}
			delay_ms(5);
		}
}

void Display::circularView()
{
	if(strlen(title)>16){
		strncpy(displayBuffer1,title+titleCursor,16);
		titleCursor++;
		if(strlen(title)-titleCursor<16) titleCursor = 0;
	}
}

void Display::volumeChange()
{
	if(volume!=previousVolume){
		strcpy(displayBuffer1, volumeText);
		displayBuffer1[15] = ' ';
		for(int i=0;i<volume;i++){
			displayBuffer2[i] = '#';
		}
		for(int i=volume;i<16;i++){
			displayBuffer2[i] = ' ';
		}
		volumeLoopCount++;
		if(volumeLoopCount > volumeLoopLimit){
			previousVolume = volume;
			volumeLoopCount = 0;
		}
	}
}

void Display::initBuffer(){
	strcpy(displayBuffer1,emptyString);
	strcpy(displayBuffer2,emptyString);
	displayBuffer1[15] = ' ';
	displayBuffer2[15] = ' ';
	strcpy(displayBuffer1,title);
	displayBuffer1[strlen(title)] = ' ';
	strcpy(displayBuffer2,artist);
	displayBuffer2[strlen(artist)] = ' ';
}

void Display::displayUpdate()
{
	if(updateFlag == 1){
		titleCursor = 0;
		updateFlag = 0;
	}
	initBuffer();	
	circularView();	
	volumeChange();
	update();
}

void Display::set(char a[],char t[])
{
	titleLength = strlen(t);
	strcpy(title,t);
	artistLength = strlen(a);
	strcpy(artist,a);
}
