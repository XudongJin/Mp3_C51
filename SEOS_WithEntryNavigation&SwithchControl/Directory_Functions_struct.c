#include "AT89C51RC2.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "SPI.h"
#include "SDCard.h"
#include "File_System_v2.h"
#include "print_bytes.h"
#include "Directory_Functions_struct.h"
#include "Read_Sector.h"



FS_values_t idata Drive_values;

/***********************************************************************
DESC: Returns a pointer to the global structure Drive_values to export to other files
INPUT: void
RETURNS: Pointer to the structure Drive_values
CAUTION: 
************************************************************************/

FS_values_t * Export_Drive_values(void)
{
   return &Drive_values;
}


/***********************************************************************
DESC: Returns a pointer to the global structure Drive_values to export to other files
INPUT: void
RETURNS: Pointer to the structure Drive_values
CAUTION: 
************************************************************************/
//name is store as unicode ,so a character occupy two types
//uint16_t 	Print_LongFile(uint16_t i, uint8_t * array_in,uint16_t entries,uint32_t * sector_p){
//	
//	uint16_t index;
//  uint16_t total_entries,out_val_long,j,offset;
//  uint8_t temp8,file_attr;
//  uint8_t * values;
//	uint32_t Sector;
//	bool   next_sector;

//	values=array_in;
//	Sector=*sector_p;
//	
//	 temp8=read8(0+i,values);  // read first byte to see if this is the last entry for long name
//	 if((temp8&0x40)==0x40){
//		 total_entries=temp8&0x1F;
//	  // printf("total_entries=%d\n",total_entries);	 
//		 if((i+total_entries*32)>510)
//		 {
//          Read_Sector(Sector+1,Drive_values.BytesPerSec,values);
//         //  i=((i+total_entries*32)-512)/32
//			    file_attr=read8(0x0b+((i+total_entries*32)-512),values);
//			    next_sector=true;
//		 }else{
//		     file_attr=read8(0x0b+i+total_entries*32,values);	 
//			   next_sector=false;
//		 }
//		 
//		 
//		 if((file_attr&0x0E)==0)   // if hidden, system or Vol_ID bit is set do not print
//		 {
//			   printf("%5d. ",entries+1);  // print entry number with a fixed width specifier
//			 	for(index=0;index<total_entries;++index){
//					
//						if(((i+(total_entries-1-index)*32)>510))
//					 {
//						    if(next_sector==false)
//								{
//									Read_Sector(Sector+1,Drive_values.BytesPerSec,values);
//								  next_sector=true;
//								}								
//						    offset=(i+(total_entries-1-index)*32)-512;
//								
//					 }else{

//						   if(next_sector==true)
//							 {	 Read_Sector(Sector,Drive_values.BytesPerSec,values);
//							     next_sector=false;
//							 }
//							 offset=(i+(total_entries-1-index)*32);
//					 }
//		 
//		 	  temp8=read8(0x0b+offset,values);			
//			  if(temp8==0x0F){		 
//		 		for(j=1;j<10;)
//			  {
//			     out_val_long=read16(offset+j,values);   // print the 8 byte name			
//					 if(out_val_long==0x0000)break;
//					 putchar(out_val_long);
//					 j+=2;
//			  }
//				if(out_val_long==0x0000)break;
//				for(j=14;j<25;)
//			  {
//			     out_val_long=read16(offset+j,values);   // print the 8 byte name
//			     if(out_val_long==0x0000)break;
//					 putchar(out_val_long);
//					 j+=2;
//			  }
//				if(out_val_long==0x0000)break;
//				for(j=28;j<31;)
//			  {
//			     out_val_long=read16(offset+j,values);   // print the 8 byte name
//			     if(out_val_long==0x0000)break;
//					 putchar(out_val_long);
//					 j+=2;
//			  } 
//				
//			}else
//		    printf("Print Long File Name Error 0!%bu\n",temp8);
//			}
//				 
//		 }
//		 
//		 
//		 if((i+total_entries*32)>510)
//		 {
//			    (*sector_p)=Sector+1;
//          Read_Sector(Sector+1,Drive_values.BytesPerSec,values);
//			    return ((i+total_entries*32)-512);	 
//		 }else{
//					return i+total_entries*32; 
//		 }
//	 }
//	 else
//	 {
//		 printf("Print Long File Name Error!%2.2bX,%2.2bX,%p\n",temp8,temp8&0x40,i+values);
//		 return i;
//	 }	 	
//}

/***********************************************************************
DESC: Prints all short file name entries for a given directory 
INPUT: Starting Sector of the directory and the pointer to a 
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint16_t number of entries found in the directory
CAUTION: Supports FAT16, SD_shift must be set before using this function
************************************************************************/



uint16_t  Print_Directory(uint32_t Sector_num, uint8_t xdata * array_in)
{ 
	 uint32_t idata Sector, max_sectors, file_size;
   uint16_t idata i, entries;
   uint8_t idata temp8, j, attr, out_val, error_flag;
   uint8_t xdata * values;
	 bool Is_longfile;
	

   values=array_in;
   entries=0;
	 file_size=0;
   i=0;
   if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
   { 
      max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
   }
   else
   {
      max_sectors=Drive_values.SecPerClus;
   }
   Sector=Sector_num;
   error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
   if(error_flag==no_errors)
   {
     do
     {
			 
//			  temp8=read8(0x0b+i,values);
//			  if(temp8==0x0F){					
//			   i=Print_LongFile(i,values,entries,&Sector); 
//				 Is_longfile=true;
//				}else Is_longfile=false;	
				Is_longfile=false;
        temp8=read8(0+i,values);  // read first byte to see if empty
        if((temp8!=0xE5)&&(temp8!=0x00))
	    {  
	       attr=read8(0x0b+i,values);
		   if((attr&0x0E)==0)   // if hidden, system or Vol_ID bit is set do not print
		   {
		      entries++;
				if(Is_longfile==false){ 
						printf("%5d. ",entries);  // print entry number with a fixed width specifier
							for(j=0;j<8;j++)
						{
							out_val=read8(i+j,values);   // print the 8 byte name
							putchar(out_val);
						}
					}
         if((attr&0x10)==0x10)  // indicates directory
			  {
					if(Is_longfile==false){
			     for(j=8;j<11;j++)
			     {
			        out_val=read8(i+j,values);
			        putchar(out_val);
			     }
				  }
			     printf("[DIR]\n");
			  }
			  else       // print a period and the three byte extension for a file
			  {
					 if(Is_longfile==false){
			     putchar(0x2E);       
			     for(j=8;j<11;j++)
			     {
			        out_val=read8(i+j,values);
			        putchar(out_val);
			     }
				  }
					 file_size=read8(31+i,values);
								
					 file_size=file_size<<8;
                
					 file_size|=read8(30+i,values);
                
					 file_size=file_size<<8;
                
					 file_size|=read8(29+i,values);
			          
					 file_size=file_size<<8;
                
					 file_size|=read8(28+i,values);
					 printf("           File Size:%lu Bytes",file_size);
			     putchar(0x0d);
                 putchar(0x0a);
			  }
		    }
		}
		    i=i+32;  // next entry
		    if(i>510)
		    {
			  Sector++;
              if((Sector-Sector_num)<max_sectors)
			  {
                 error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
			     if(error_flag!=no_errors)
			     {
			        entries=0;   // no entries found indicates disk read error
				    temp8=0;     // forces a function exit
			     }
                 i=0;
			  }
			  else
			  {
			     entries=entries|more_entries;  // set msb to indicate more entries in another cluster
			     temp8=0;                       // forces a function exit
			  }
		    }
         
	  }while(temp8!=0);
	}
	else
	{
	   entries=0;    // no entries found indicates disk read error
	}
   return entries;
 }


/***********************************************************************
DESC: Uses the same method as Print_Directory to locate short file names,
      but locates a specified entry and returns and cluster  
INPUT: Starting Sector of the directory, an entry number and a pointer to a 
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint32_t with cluster in lower 28 bits.  Bit 28 set if this is 
         a directory entry, clear for a file.  Bit 31 set for error.
CAUTION: 
************************************************************************/

uint32_t Read_Dir_Entry(uint32_t Sector_num, uint16_t Entry, uint8_t xdata * array_in ,uint8_t  * mp3_name,uint8_t  * file_attr)
{ 
   uint32_t idata Sector, max_sectors, return_clus, file_size;
   uint16_t idata i, j,entries;
   uint8_t  idata temp8, attr,error_flag;
   uint8_t xdata * values;

   values=array_in;
   entries=0;
   i=0;
   return_clus=0;
   if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
   { 
      max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
   }
   else
   {
      max_sectors=Drive_values.SecPerClus;
   }
   Sector=Sector_num;
   error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
   if(error_flag==no_errors)
   {
     do
     {
        temp8=read8(0+i,values);  // read first byte to see if empty
        if((temp8!=0xE5)&&(temp8!=0x00))
	    {  
	       attr=read8(0x0b+i,values);
		   if((attr&0x0E)==0)    // if hidden do not print
		   {
		      entries++;
              if(entries==Entry)
              {
			    if(Drive_values.FATtype==FAT32)
                {
                   return_clus=read8(21+i,values);
				           return_clus=return_clus<<8;
                   return_clus|=read8(20+i,values);
                   return_clus=return_clus<<8;
                }
                return_clus|=read8(27+i,values);
			          return_clus=return_clus<<8;
								return_clus|=read8(26+i,values);

								
							  for(j=0;j<16;j++)
								file_attr[j]=0x20;
								mp3_name[0]   = (uint8_t)(((Entry>>8)&0xff)+0x30);
					    	mp3_name[1]   =(uint8_t)((Entry &0xff)+0x30);
								mp3_name[2]   ='.';
								
								attr=read8(0x0b+i,values);
								if(attr&0x10){
									return_clus|=directory_bit;
									for(j=0;j<8;j++)
									{
									// print the 8 byte name
										mp3_name[j+3]=read8(i+j,values);  
									} 
									for(j=8;j<11;j++)
									{
										mp3_name[j+3]=read8(i+j,values);  
									}
										mp3_name[j+3]=0x20;  
									  mp3_name[j+4]=0x20; 
																	
								 file_attr[0]='D';							               
					       file_attr[1]='I';           
					       file_attr[2]='R';              
							}
					      else
								{
								for(j=0;j<8;j++)
								{
									// print the 8 byte name
									mp3_name[j+3]=read8(i+j,values);  
								}
								mp3_name[j+3]=0x2E;      
								for(j=8;j<11;j++)
								{
									mp3_name[j+4]=read8(i+j,values);  
								}
								 mp3_name[j+4]=0x20; 
								
								 file_size=read8(31+i,values);
								
								 file_size=file_size<<8;
                
								 file_size|=read8(30+i,values);
                
								 file_size=file_size<<8;
                
								 file_size|=read8(29+i,values);
			          
								 file_size=file_size<<8;
                
								 file_size|=read8(28+i,values);
								 sprintf(file_attr, "%lu", file_size); 
								 file_attr[11]='B';
								 file_attr[12]='Y';
								 file_attr[13]='T';	
								 file_attr[14]='E';	
								 file_attr[15]='S';								
								}
                temp8=0;    // forces a function exit
              }
              
		    }
		}
		    i=i+32;  // next entry
		    if(i>510)
		    {
			  Sector++;
			  if((Sector-Sector_num)<max_sectors)
			  {
                 error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
			     if(error_flag!=no_errors)
			     {
			         return_clus=no_entry_found;
                     temp8=0; 
			     }
			     i=0;
			  }
			  else
			  {
			     temp8=0;                       // forces a function exit
			  }
		    }
         
	  }while(temp8!=0);
	}
	else
	{
	   return_clus=no_entry_found;
	}
	if(return_clus==0) return_clus=no_entry_found;
   return return_clus;
 }



