#include "stdio.h"
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "print_bytes.h"
#include "SPI.h"
#include "SDcard.h"
#include "Long_Serial_In.h"
#include "Directory_Functions_struct.h"
#include "File_System_v2.h"
#include "STA013_Config.h"
#include "SEOS.h"
#include "LCD_Routines.h"
#include "Switch.h"

uint8_t xdata buf1[512];
uint8_t xdata buf2[512];


extern Sw_global_t  play_button,scrollup_button,scrolldown_button;
main()
{
   uint32_t  idata Current_directory, Entry_clus;
   uint16_t  idata  i, num_entries, entry_num;
   uint8_t   idata error_flag;
	 uint8_t idata mp3_name[16];
   uint8_t idata file_attr[16];
   FS_values_t * Drive_p;

   AMBERLED=OFF;
   YELLOWLED=OFF;
   GREENLED=OFF;
   REDLED = ON;
   STA013_RESET=0;
   i=0;
   while(i<=60000) i++;
   REDLED = OFF;
   AUXR=0x0c;   // make all of XRAM available
   if(OSC_PER_INST==6)
   {
      CKCON0=0x01;  // set X2 clock mode
   }
   else
   {
      CKCON0=0x00;  // set standard clock mode
   } 
   uart_init();
   printf("I2C Test Program\n\r\n\n");
   error_flag=SPI_Master_Init(400000UL);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   printf("SD Card Initialization ... \n\r");
   error_flag=SD_card_init();
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }                        
   error_flag=SPI_Master_Init(50000000UL);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   for(i=0;i<512;i++)
   {
      buf1[i]=0xff;  // erase valout for debug
      buf2[i]=0xff;
   }
	 for(i=0;i<12;i++)
   {
      mp3_name[i]=0x00;  // erase valout for debug
   }
   error_flag=Mount_Drive(buf1);
   if(error_flag!=no_errors)
   {
      REDLED=ON;
      while(1);
   }
   Drive_p=Export_Drive_values();
   Current_directory=Drive_p->FirstRootDirSec;
   
   
   	 	LCD_Init();
   STA013_init(); 
   Switch_init();	 
	 num_entries=Print_Directory(Current_directory, buf1);
	 entry_num=1;
	 if(entry_num<=num_entries){
	 	Entry_clus=Read_Dir_Entry(Current_directory, entry_num, buf1,mp3_name,file_attr);
		Update_Line1(mp3_name,16);
		Update_Line2(file_attr,16);
	 }
   while(1)
   {
		 
  	Switch_Get_Input(SCROLLUP_MASK ,&scrollup_button);
		if(scrollup_button.sw_state==SW_PRESSED) // scrollup_button
		{
			if((entry_num-1)>=1){
				entry_num=entry_num-1;
				Entry_clus=Read_Dir_Entry(Current_directory, entry_num, buf1,mp3_name,file_attr);
				Update_Line1(mp3_name,16);
				Update_Line2(file_attr,16);

			}
		}
		Switch_Get_Input(SCROLLDOWN_MASK ,&scrolldown_button);
		if(scrolldown_button.sw_state==SW_PRESSED) //scrolldown_button
		{
			if((entry_num+1)<=num_entries){
				entry_num=entry_num+1;
				Entry_clus=Read_Dir_Entry(Current_directory, entry_num, buf1,mp3_name,file_attr);
				Update_Line1(mp3_name,16);
				Update_Line2(file_attr,16);
			}
		}
		Switch_Get_Input(PLAY_MASK ,&play_button);
		if(play_button.sw_state==SW_PRESSED) // play_button
		{
			 if(Entry_clus&directory_bit)  // if it is a drectory
		  {
		     Entry_clus&=0x0FFFFFFF;
         Current_directory=First_Sector(Entry_clus);
				
				num_entries=Print_Directory(Current_directory, buf1);
				entry_num=1;
				if(entry_num<=num_entries){
				Entry_clus=Read_Dir_Entry(Current_directory, entry_num, buf1,mp3_name,file_attr);
				Update_Line1(mp3_name,16);
				Update_Line2(file_attr,16);
				}
		  }
 	     else
		  {

		    Play_Song_State_Machine(Entry_clus);// if it is a file
		  }
			
		}
 
		 
		 
   }
} 




