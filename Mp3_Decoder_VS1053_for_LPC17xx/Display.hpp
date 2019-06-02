#ifndef DISPLAY_H
#define DISPLAY_H

#include "uart.hpp"
#include "string.h"	
#include "utilities.h"
class Display{
	private: 
		uint8_t cursorArray[32] = {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf};
		char displayBuffer1[16];
		char displayBuffer2[16];
		char emptyString[16] = "               ";
		char volumeText[16] = "Volume         ";
		int titleLength;
		int previousVolume;
		int volumeLoopLimit;
		int volumeLoopCount;
		int artistLength;
	public:
		char title[256];
		char artist[256];
		
		int volume;
		int titleCursor = 0;
		int updateMode;
		bool circle;
		bool vol;
		LabUART uart;
		int updateFlag;

		void initBuffer();
		void clearScreen();
		void setCursor(int row,int col);	
		void init(char a[], char t[]);

		void set(char a[],char t[]);		
		
		void update();//1 for first row, 2 for second row,3 for both row
		void circularView();
		void volumeChange();
		void displayUpdate();
		

};
#endif
