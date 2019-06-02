#include "FreeRTOS.h"
#include "task.h"
#include "LPC17xx.h"
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"
#include "uart0_min.h"
#include "queue.h"
#include <string.h>
#include <stdint.h>
#include "lpc_sys.h"
#include "gpio.hpp"
#include "mp3decoder.hpp"
extern "C" {
#include "ff.h"
}
#include <time.h>
#include <sys/time.h>
#include "handlers.hpp"
#include "tasks.hpp"
#include "printf_lib.h"
#include "io.hpp"
#include "IR_sensor.hpp"
#include "uart.hpp"
#include "Display.hpp";
MP3DECODER mp3driver;
char fileName[100][100];
char SongName[100][50];
char artist[100][30];
uint8_t volume = 0x50;
QueueHandle_t Global_Queue_Handle = 0;
QueueHandle_t Play_Queue_Handle = 0;
QueueHandle_t Song_Queue_Handle = 0;

int songNum=0;
static int songIndex=0;


/*=======================================================================*/
Display display;
void updateDisplay(void * p)
{
	display.updateMode = 3;
	while(1){

		display.displayUpdate();
		vTaskDelay(1000);
	}
}
/*=======================================================================*/
void vIRCode(void * p)
{
    while(1)
    {
        if(IR_Sensor::getInstance().isIRCodeReceived()){
		uint32_t data = IR_Sensor::getInstance().getLastIRCode();
		if( xQueueSend(Global_Queue_Handle, &data, portMAX_DELAY) );
	}
	vTaskDelay(50);
    }
}

void vTaskStateMachine(void *p)	
{
	uint32_t data;
	uint8_t play=0;
	uint8_t selectMode = 0;
	while(1)
	{
		
		if(xQueueReceive(Global_Queue_Handle,&data,portMAX_DELAY ))
		{
			switch(data)
			{
			case 0x631cff00: //ok
				if(!selectMode)
				{
					play = ~play;
					xQueueSend(Play_Queue_Handle, &play, portMAX_DELAY);
				}
				else
				{
					xQueueSend(Song_Queue_Handle, &fileName[songIndex], portMAX_DELAY);
					
				}
								
				break;
			case 0x6718ff00: //up
				//printf("up volume");
				
				if(volume >= 5)
				{
					volume -= 0x5;
					mp3driver.VolSet(volume);
					display.volume++;
				}	
				else
				{	
					volume = 0;
					mp3driver.VolSet(0x0);
				}			
				break;
			case 0x2d52ff00: //down 
				//printf("down volume");
				
				if(volume <= 0xf9) 
				{
					volume += 0x5;
					mp3driver.VolSet(volume);
					display.volume--;
				}
				else 
				{	
					volume = 0xfe;
					mp3driver.VolSet(0xfe);
				}
				break;
			case 0x7708ff00: //left (previous song)
				//printf("prev song");
				if(songIndex == 0) songIndex = songNum - 2;
				else songIndex = songIndex - 1;
				if(!selectMode)
				{
					xQueueSend(Song_Queue_Handle, &fileName[songIndex], portMAX_DELAY);
				}	
				display.updateFlag = 1;	
				display.set(artist[songIndex],SongName[songIndex]);		
				break;
			case 0x255aff00: //right (next song)
				//printf("next song");
				if(songIndex == (songNum - 2)) songIndex = 0;				
				else songIndex = songIndex + 1;
				if(!selectMode)
				{
					xQueueSend(Song_Queue_Handle, &fileName[songIndex], portMAX_DELAY);
				}
				display.updateFlag = 1;
				display.set(artist[songIndex],SongName[songIndex]);		
				break;
			case 0x6916ff00: //* search mode
				selectMode = ~selectMode;
				break;
			default: 
				//printf("unchange"); 
				vTaskDelay(100);
				break;
			}	
		}
	}

}

FRESULT scan_all_files (
    char* path        
)

{

    FRESULT result;
    DIR dir;
    UINT i;
    FILINFO fno;
    char Lfname[_MAX_LFN];
    result = f_opendir(&dir, path); 
    if (result == FR_OK) {
	
        for (;; ) { 
        fno.lfname = Lfname; 
        //fno.lfsize = _MAX_LFN - 1; 
        fno.lfsize = sizeof(Lfname);
        result = f_readdir(&dir, &fno); 
        if (result != FR_OK|| !fno.fname[0]) { 
               break;                          
        } 
	
	if(fno.fattrib & AM_DIR) { 
	          if(Lfname[0]!='.')
		  {
			  i = strlen(path);
		          sprintf(&path[i], "/%s", fno.lfname[0] ? Lfname : &fno.fname[0]); 
			  result = scan_all_files(path);
			  if (result != FR_OK) break;
			  path[i] = 0;
		  }
		  
        } 
        else { 
		  if(Lfname[strlen(Lfname)-1] == '3')
		  {
			uint8_t j = 0;
			int k = 0;
			
			while((Lfname[j] != '-') && (j <(strlen(Lfname)-1))) j++;
			if(Lfname[j] == '-') 
			{
				k=j;
				while(Lfname[k-1] == ' ') k--;
				strncpy(artist[songIndex],Lfname,k);	
				k=j;
				while(Lfname[k+1] == ' ')k++;
				//int l;
				strcpy(SongName[songIndex],&Lfname[k+1]);		
			}		
			else			
			{
				strcpy(SongName[songIndex],Lfname);
				strcpy(artist[songIndex],"unknown");	
			}
			SongName[songIndex][strlen(SongName[songIndex])-4] = 0;
			sprintf(fileName[songIndex],"%s/%s",path,Lfname);
			//printf("%i,%s\n",songIndex,fileName[songIndex]);
			//printf("%s\n",SongName[songIndex]);
			//strcpy(fileName[songIndex],
			songIndex += 1;
		  }
		  
        } 
        

	}
        f_closedir(&dir);
        
    }
    return result;
}

void vPlaySong(void *p)
{
	char songname[100];
	strcpy(songname,fileName[songIndex]);
	unsigned char array[128];  
        uint8_t playS = 0;
	//bool play=false;
	bool cancel = false;
	mp3driver.init();
	mp3driver.VolSet(0x40);
	while(1)
  	{
		FILE *fil;
		cancel = false;
		if((fil = fopen(songname,"r")) == NULL)
		{
			exit(1);
		}
		else
		{
			//puts("file open successfully");
		}

		while(!feof(fil))
		{
			xQueueReceive(Play_Queue_Handle,&playS,1);		      	
			if(playS)
		      	{
				fread(&array, 1, 128, fil);
				//printf("1");
				for(int i=0; i<128; i++)
				{
					
			    		mp3driver.sdi_write(array[i]);
				}
		      	}
			if(xQueueReceive(Song_Queue_Handle,&songname,1))
			{
				playS = 0;
				//printf("2");
				if(!cancel){
					mp3driver.sci_writeReg(0x808,MODE);				
					cancel=true;				
				}				
				
			}
			else if(cancel)
			{
				//printf("3");
				while(!(mp3driver.sci_readReg(MODE) & 0x8))
				{
					mp3driver.sci_writeReg(0x808,MODE);
					fread(&array, 1, 32, fil);
					for(int i=0; i<32; i++)
					{
			    			mp3driver.sdi_write(array[i]);
					}
				}
				playS = true;
				cancel=false;
				break;
			}
		
			
  		}
  		fclose(fil);
  	}

  	return;
}


int main (void)
{

    FATFS filesystem;

	FRESULT result;
	  const uint32_t STACK_SIZE = 1024;
	char buff[256];

	result = f_mount(&filesystem, "", 1);

    if (result == FR_OK) {
        strcpy(buff, "1:");
        result = scan_all_files(buff);
    }
songNum = songIndex + 1;
songIndex = 0;

Global_Queue_Handle = xQueueCreate(2, sizeof(uint32_t));
Play_Queue_Handle = xQueueCreate(1, sizeof(uint8_t));
Song_Queue_Handle = xQueueCreate(1,100*sizeof(char));
//printf("This is what suppose o be %s\n",SongName[songIndex]);
display.init(artist[songIndex],SongName[songIndex]);
xTaskCreate(vIRCode, "vIRCode", STACK_SIZE, NULL, 3, NULL );
xTaskCreate(vPlaySong, "vPlaySong", 2048, NULL, 3, NULL );
xTaskCreate(vTaskStateMachine, "vStateMachine", 512, NULL, 3, NULL );
xTaskCreate(updateDisplay, "display", 512, NULL, 1, NULL );
  scheduler_add_task(new terminalTask(2));
  scheduler_start();
vTaskStartScheduler();

	return 0;
} 
