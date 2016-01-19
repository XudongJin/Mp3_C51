#include "main.h"
#include "PORT.H"
#include "SEOS.h"
#include "stdio.h"
#include "hardware_delay.h"
#include "LCD_Routines.h"
#include "SPI.h"
#include "File_System_v2.h"
#include "Read_Sector.h"
#include "I2C.h"
#include "STA013_Config.h"
#include "Switch.h"

extern Sw_global_t  stop_button,play_button,scrollup_button,scrolldown_button;

extern uint8_t xdata buf1[512];
extern uint8_t xdata buf2[512]; 

Mp3_State_t   state_g;
Play_State_t  play_status_g;
uint8_t  send_val[2];
uint32_t  sector_base_g,sector_offset_g,cluster_g;
uint16_t  index1, index2;
uint8_t   temp8;


void Timer2_Interrupt_Init(){
	

	TH2 = (uint8_t)Timer2_Reload_H;
	TL2 = (uint8_t)Timer2_Reload_L;
	RCAP2H = (uint8_t)Timer2_Reload_H;
	RCAP2L = (uint8_t)Timer2_Reload_L;

	
	
	ET2=1; // Enable Timer2 Interrupt
	EA=1;  // Enable Interrupt
	TR2=1;  //Start Timer2
	
}




void Timer2_ISR(void) interrupt Timer_2_Overflow {
	
	  TF2=0;
	

  
	
		Switch_Get_Input(STOP_MASK ,&stop_button);
		if(stop_button.sw_state==SW_HELD) // stop button
		{
				play_status_g = STOP;
		}
	
		Switch_Get_Input(PLAY_MASK ,&play_button);
		if(play_button.sw_state==SW_HELD)// PAUSE button
		{
			  if(play_status_g ==PLAY) { // song playing
					play_status_g = PAUSE; // pause song
					send_val[0]=0x13;  // play register
          send_val[1]=0x00;
					I2C_Write(STA013,2,send_val);
					send_val[0]=0x14;   // mute register
          send_val[1]=0x00;
					I2C_Write(STA013,2,send_val);
				}
				else {
					play_status_g = PLAY;
					send_val[0]=0x13;  // play register
          send_val[1]=0x01;
					I2C_Write(STA013,2,send_val);
					send_val[0]=0x14;  // mute register
          send_val[1]=0x00;
					I2C_Write(STA013,2,send_val);
					//START PLAY SONG
				}
		}
		
//			if(play_status_g==PLAY){
//		   time_g += (uint16_t)ISR_TICK;
//			}

		
		if(state_g==LOAD_BUFFER_1)
	{
		if(sector_offset_g>=64)
		{
			state_g =FIND_CLUSTER_1;
		}
	}

	if(state_g==LOAD_BUFFER_2)
	{
		if(sector_offset_g>=64)
		{
			state_g =FIND_CLUSTER_2;
		}
	}


	
	switch(state_g)
	{
	
	 case DATA_IDLE_1:
	 {

     GREENLED=OFF;REDLED=OFF;YELLOWLED=OFF;AMBERLED=OFF;
			TH0=(uint8_t)Timer_Reload_10ms_THx;
			TL0=(uint8_t)Timer_Reload_10ms_TLx;

			TF0=0;   // Clear overflow
			TR0=1;   // Start Timer Running
		  while((DATA_REQ == INACTIVE ) && (TF0==0));
		  TR0 = 0;  
		  if(DATA_REQ == ACTIVE)
						state_g = DATA_SEND_1;
			break;
	 }
	 
	 case DATA_IDLE_2:
	 {
		// 	TMOD &=0xF0;
		//	TMOD |=0x01;

	//		ET0=0;  //interrupts not used
		  //updataPlayTime();

      GREENLED=OFF;REDLED=OFF;YELLOWLED=OFF;AMBERLED=OFF;
			TH0=(uint8_t)Timer_Reload_10ms_THx;
			TL0=(uint8_t)Timer_Reload_10ms_TLx;

			TF0=0;   // Clear overflow
			TR0=1;   // Start Timer Running

			while((DATA_REQ == INACTIVE ) && (TF0==0));
		  TR0 = 0;  
		  if(DATA_REQ == ACTIVE)
						state_g = DATA_SEND_2;
			else
				    state_g = DATA_IDLE_2;
			break;
		 
	 }
	
	 case DATA_SEND_1:
	 {		
		 
	       if(play_status_g == PLAY){

				 TH0=(uint8_t)Timer_Reload_10ms_THx;
				 TL0=(uint8_t)Timer_Reload_10ms_TLx;

				 TF0=0;   // Clear overflow
				 TR0=1;   // Start Timer Running
		   
				 BIT_EN =1;
				 GREENLED=ON;REDLED=OFF;YELLOWLED=OFF;AMBERLED=OFF;
				 while((DATA_REQ == ACTIVE ) && (TF0==0))
				 {
			
			//		 GREENLED=ON;
					 SPI_Transfer(buf1[index1], &temp8);
			//		 GREENLED=OFF;
					 index1++;
					 if(index1>511) // Buffer1 Empty
					 {
						 if(index2>511) // Buffer2 Empty
						 {
							  if(play_status_g ==FINISH)
							  {
									play_status_g=STOP;
								}
								else
								{
									
									state_g = LOAD_BUFFER_2;
									
								}
						 }
						 else state_g = DATA_SEND_2;
						 TF0=1;	 
					 }	 
				 }
				 if(DATA_REQ == INACTIVE )
				 {
					 if(index2>511)// Buffer2 Empty
					 {	 	state_g = LOAD_BUFFER_2;
					 }
					else state_g = DATA_IDLE_1;
				 }
				 TR0 = 0;  
				 BIT_EN =0;
			 }
				 break;
	}
			
	case LOAD_BUFFER_1:
	{	
		GREENLED=OFF;REDLED=OFF;YELLOWLED=ON;AMBERLED=OFF;
		Read_Sector(sector_base_g+sector_offset_g,512,buf1);
		sector_offset_g++;
		index1=0;
		state_g = DATA_IDLE_2;
		
		break;
	}
	
	case FIND_CLUSTER_1:
	{

		cluster_g=Find_Next_Clus(cluster_g,buf1);
		GREENLED=OFF;REDLED=ON;YELLOWLED=OFF;AMBERLED=OFF;
		if(cluster_g == 0x0FFFFFFF){ // EOF Found
			play_status_g = FINISH;
			state_g = DATA_IDLE_2;
		}
		else{

			sector_base_g = First_Sector(cluster_g);
			sector_offset_g =0 ;
			state_g = DATA_IDLE_2;
		}	
		break;
	}
	
	case LOAD_BUFFER_2:
	{	
		GREENLED=OFF;REDLED=OFF;YELLOWLED=OFF;AMBERLED=ON;

		Read_Sector(sector_base_g+sector_offset_g, 512, buf2);
		sector_offset_g++;
		index2=0;
		state_g = DATA_IDLE_1;

		break;
	}
	
	case FIND_CLUSTER_2:
	{
		GREENLED=OFF;REDLED=ON;YELLOWLED=OFF;AMBERLED=OFF;
		cluster_g=Find_Next_Clus(cluster_g,buf2);
		if(cluster_g == 0x0FFFFFFF){ // EOF Found
			play_status_g = FINISH;
			state_g = DATA_IDLE_1;
		}
		else{
			sector_base_g = First_Sector(cluster_g);
			sector_offset_g =0 ;
			state_g = DATA_IDLE_1;	
		}	
		break;
	}
	
	 case DATA_SEND_2:
	 {	
         if(play_status_g == PLAY){

				 TH0=(uint8_t)Timer_Reload_10ms_THx;
				 TL0=(uint8_t)Timer_Reload_10ms_TLx;

				 TF0=0;   // Clear overflow
				 TR0=1;   // Start Timer Running
		 	  GREENLED=ON;REDLED=OFF;YELLOWLED=OFF;AMBERLED=OFF;
				 BIT_EN =1;
				 while((DATA_REQ == ACTIVE ) && (TF0==0))
				 {

					 SPI_Transfer(buf2[index2], &temp8);

					 index2++;
					 if(index2>511) // Buffer2 Empty
					 {
						 if(index1>511) // Buffer1 Empty
						 {
							  if(play_status_g ==FINISH)
							  {
									play_status_g=STOP;
								}
								else
								{									
									state_g = LOAD_BUFFER_1;									
								}
						 }
						 else state_g = DATA_SEND_1;
						 TF0=1;	 
					 }	 
				 }
				 	 
				 if(DATA_REQ == INACTIVE )
				 {
					 if(index1>511)// Buffer2 Empty
					 { 	state_g = LOAD_BUFFER_1;
					 }
						else state_g = DATA_IDLE_2;
				 }
		  	  TR0 = 0; 
				 	BIT_EN =0;
			 }
				  break;
		}
	}


}



void Play_Song_State_Machine(uint32_t Start_Cluster)
{

   cluster_g = Start_Cluster;
   sector_base_g=First_Sector(cluster_g);
   sector_offset_g=0;
   //P3_2=ON;
   YELLOWLED=ON;
   index1=0;
   
   Read_Sector(sector_base_g+sector_offset_g, 512, buf1);

   sector_offset_g++;
   YELLOWLED=OFF;
   AMBERLED=ON;
   index2=0;
   
   Read_Sector(sector_base_g+sector_offset_g, 512, buf2);
   sector_offset_g++;
   AMBERLED=OFF; 

	  play_status_g = PLAY;
 
	 	state_g=DATA_IDLE_1;
   
	  temp8=0;
		
    Switch_init();
		

    TMOD &=0xF0;
		TMOD |=0x01;
		ET0=0;  //interrupts not used
			
		Timer2_Interrupt_Init();
	 while(1){
		if((play_status_g ==STOP) || (play_status_g ==FINISH))
		{

			ET2=0; // Disable Timer2 Interrupt
			EA=0;  // Disable Interrupt
			TR2=0;  //STOP Timer2
			break;	 
		}
	 }
	 
 }


 
 
 