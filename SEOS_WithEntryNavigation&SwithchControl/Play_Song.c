#include "stdio.h"
#include "main.h"
#include "PORT.H"
#include "SPI.h"
#include "File_System_v2.h"
#include "Read_Sector.h"
#include "Play_Song.h"
extern uint8_t xdata buf1[512];
extern uint8_t xdata buf2[512]; 

uint32_t sector_base_g,sector_offset_g;
uint16_t index1, index2;
uint8_t temp8;

void Play_Song(uint32_t Start_Cluster)
{
   
   uint8_t buffer1, buffer2;
   printf("Starting Cluster = %lu\n\r",Start_Cluster);

   sector_base_g=First_Sector(Start_Cluster);
   printf("Starting Sector = %lu\n\r",sector);
   sector_offset_g=0;
   buffer1=1;
   buffer2=0;
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
	 
	 
//   do
//  {      
//     do
//     {
//        if(DATA_REQ==0)
//        {
//           GREENLED=ON;
//           BIT_EN=1;
//           SPI_Transfer(buf1[index1], &temp8);
//	       GREENLED=OFF;
//	       index1++;
//           if(index1>511)
//           {
//              if(index2>511)
//              {
//                  BIT_EN=0;              
//                  AMBERLED=ON;
//				  index2=0;
//				  
//				  Read_Sector(sector+sector_offset, 512, buf2);
//				  sector_offset++;
//                  AMBERLED=OFF;
//              }
//              buffer1=0;
//              buffer2=1;

//          }
//       }
//       else
//       {
//          if(index2>511)
//          {
//              BIT_EN=0;
//              AMBERLED=ON;
//			  index2=0;
//			  
//			  Read_Sector(sector+sector_offset, 512, buf2);
//			  sector_offset++;
//              AMBERLED=OFF;
//          }
//          else
//          {
//              if(index1>511)
//              {
//                  buffer1=0;
//                  buffer2=1;
//              }
//          }
//      }
//   }while(buffer1==1);
//		 
//   do
//   {
//      if(DATA_REQ==0)
//      {
//				
//          REDLED=ON;
//          BIT_EN=1;
//          SPI_Transfer(buf2[index2], &temp8);
//          REDLED=OFF;
//          index2++;
//          if(index2>511)
//          {
//              if(index1>511)
//              {
//                  BIT_EN=0; 
//                  YELLOWLED=ON;
//				  index1=0;
//				  
//				  Read_Sector(sector+sector_offset, 512, buf1);
//				  sector_offset++;
//                  YELLOWLED=OFF;
//              }
//              buffer2=0;
//              buffer1=1;
//         
//           }
//        }
//        else
//        {
//           if(index1>511)
//           {
//              BIT_EN=0; 
//              YELLOWLED=ON;
//			  index1=0;
//			  
//			  Read_Sector(sector+sector_offset, 512, buf1);
//			  sector_offset++;
//              YELLOWLED=OFF;
//           }
//           else
//           {
//               if(index2>511)
//               {
//                  buffer2=0;
//                  buffer1=1;
//               }
//           }
//        }
//      }while(buffer2==1);
//  }while(sector_offset<512);
//P3_2=OFF;
} 

/*

void Play_Song2(uint32_t Start_Cluster)
{
   uint16_t index1;
   uint8_t buff_emp, * p_out;
   uint32_t sector, sector_offset;
printf("Starting Cluster = %lu\n\r",Start_Cluster);
sector=First_Sector(Start_Cluster);
printf("Starting Sector = %lu\n\r",sector);
//P3_2=ON;
sector_offset=0;
YELLOWLED=ON;
nCS0=0;
SEND_COMMAND(17,sector+sector_offset);
read_block(512,buf1);
index1=0;
sector_offset++;
nCS0=1;
YELLOWLED=OFF;
//AMBERLED=ON;
//nCS0=0;
//SEND_COMMAND(17,sector+sector_offset);
//read_block(buf2,512);
//sector_offset++;
//nCS0=1;
//AMBERLED=OFF;


   p_out=buf1;
   buff_emp=0;
   do
   {
      if(DATA_REQ==0)
      {
         GREENLED=ON;
         BIT_EN=1;
         while((SPSTA&0x80)!=0x80);
         SPDAT=*(buf1+index1);
         index1++;
         if(index1==512)
         {
            buff_emp|=1;           
         }
		 if(index1==1024)
		 {
		    index1=0;
			buff_emp|=2;
	     }
         if(index1==768)
         {
           BIT_EN=0;
           GREENLED=OFF;
           if((buff_emp & 0x01)==0x01)
           {
              YELLOWLED=ON;
              nCS0=0;
              SEND_COMMAND(17,sector+sector_offset);
              read_block(512,buf1);
              nCS0=1;             
              YELLOWLED=OFF;
              buff_emp &= 0xFE;
              sector_offset++;            
           }
         }
         if(index1==256)
         {
           BIT_EN=0;
           GREENLED=OFF;
           if((buff_emp & 0x02)==0x02)
           {
              AMBERLED=ON;
              nCS0=0;
              SEND_COMMAND(17,sector+sector_offset);
              read_block(512,buf2);
              nCS0=1;             
              AMBERLED=OFF;
              buff_emp &= 0xFD;
              sector_offset++;            
           }
         }                        
       }
       else
       {
          GREENLED=OFF;
          BIT_EN=0;
          if((buff_emp & 0x01)==0x01)
          {
             YELLOWLED=ON;
             nCS0=0;
             SEND_COMMAND(17,sector+sector_offset);
             read_block(512,buf1);
             nCS0=1;             
             YELLOWLED=OFF;
             buff_emp &= 0xFE;
             sector_offset++;
 //            print_hex(1);
 //            print_hex(i);
            
          }
          else if((buff_emp & 0x02)==0x02)
          {
             AMBERLED=ON;
             nCS0=0;
             SEND_COMMAND(17,sector+sector_offset);
             read_block(512,buf2);
             nCS0=1;             
             AMBERLED=OFF;
             buff_emp &= 0xFD;
             sector_offset++;
          }
       }
   }while(sector_offset<128);   
   GREENLED=1;
//   P3_2=OFF;
  }  

 */  