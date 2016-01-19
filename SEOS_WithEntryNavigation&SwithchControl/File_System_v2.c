#include "AT89C51RC2.h"
#include <stdio.h>
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "SPI.h"
#include "SDCard.h"
#include "File_System_v2.h"
#include "print_bytes.h"
#include "Directory_Functions_struct.h"
#include "Read_Sector.h"
#include "Play_Song.h"


extern FS_values_t idata Drive_values;

// Read Value Functions

/***********************************************************************
DESC: Reads a specified byte from a block of memory 
INPUT: Offset to the byte, Starting address of the block of memory
RETURNS: uint8_t specifed byte
CAUTION: 
************************************************************************/
uint8_t read8(uint16_t offset, uint8_t * array_name)
{
   uint8_t  return_val;
   offset&=0x1FF;  // limit offset to 0 to 511
   return_val=*(array_name+offset);
   return return_val;
}

/***********************************************************************
DESC: Reads a specified integer (stored little endian) from a block of memory 
INPUT: Offset to the LSB of the integer, Starting address of the block of memory
RETURNS: uint16_t specfied integer
CAUTION: 
************************************************************************/

uint16_t read16(uint16_t offset, uint8_t * array_name)
{
   uint16_t  return_val,temp;
   offset&=0x1FF;  // limit offset to 0 to 511
   return_val=0;
   return_val=*(array_name+offset+1);
   return_val=return_val<<8;
   temp=*(array_name+offset);
   return_val|=temp;
   return return_val;
}

/***********************************************************************
DESC: Reads a specified word (stored little endian) from a block of memory 
INPUT: Offset to the LSB of the word, Starting address of the block of memory
RETURNS: uint32_t specfied word
CAUTION: 
************************************************************************/

uint32_t read32(uint16_t offset, uint8_t * array_name)
{
   uint32_t  return_val;
   uint8_t  temp, i;
   return_val=0;
   offset&=0x1FF;  // limit offset to 0 to 511
   for(i=0;i<4;i++)
   {   
       temp=*(array_name+offset+(3-i));
       return_val=return_val<<8;
       return_val|=temp;
   }
   return return_val;
}

/***********************************************************************
DESC: Determines FAT type, Start of the FAT, Root Directory Sector
      and Start of Data for an SD card
INPUT: Start of a block of memory in xdata that can be used 
       to read blocks from the SD card
RETURNS: uint8_t error code    Creates globals in idata for: 
idata uint32_t FATSz, RelSec, RootClus, FirstDataSec, StartofFAT;
idata uint16_t BytesPerSec, BytesPerSecM, RootEntCnt;
idata uint8_t SecPerClus, FATtype, SecShift;

CAUTION: 
************************************************************************/

uint8_t Mount_Drive(uint8_t xdata * array_in)
{
  uint32_t idata TotSec, DataSec, CntClus,MBR_RelSec, FATSz, RootClus;
  uint16_t idata RsvdSecCnt, RootEntCnt, temp16;
  uint8_t idata NumFAT, temp8;
  uint8_t xdata * values;
  FS_values_t * Drive_p;

  Drive_p=Export_Drive_values();
   
   values=array_in;
   printf("Locating Boot Sector...\n\r");
   Read_Sector(0,512,values);
   temp8=read8(0,values);
   if((temp8!=0xEB)&&(temp8!=0xE9))
   {
      MBR_RelSec=read32(MBR_RelSectors,values);
	  Read_Sector(MBR_RelSec,512,values);
	  printf("Relative Sectors = %ld\n\r",MBR_RelSec);
	  temp8=read8(0,values);
   }
   if((temp8!=0xEB)&&(temp8!=0xE9))
   {
      printf("Disk Error!\n\r");
	  return Disk_Error;
   }
   Drive_p->BytesPerSec=read16(BPB_BytesPerSector,values);
   Drive_p->BytesPerSecShift=0;
   temp16=Drive_p->BytesPerSec;
   while(temp16!=0x01)
   {
      Drive_p->BytesPerSecShift++;
      temp16=temp16>>1;
   }
   Drive_p->SecPerClus=read8(BPB_SectorsPerCluster,values);
   RsvdSecCnt=read16(BPB_RsvdSectorCount,values);
   NumFAT=read8(BPB_NumberFATs,values);
   RootEntCnt=read16(BPB_RootEntryCount,values);
   TotSec=read16(BPB_TotalSectors16,values);
   if(TotSec==0) TotSec=read32(BPB_TotalSectors32,values);
   FATSz=read16(BPB_FATsize16,values);
   if(FATSz==0)
   {
	  FATSz=read32(BPB_FATsize32,values);
      RootClus=read32(BPB_RootCluster,values);
   }
   Drive_p->RootDirSecs=((RootEntCnt * 32) + (Drive_p->BytesPerSec - 1))/Drive_p->BytesPerSec;
   DataSec=TotSec-(RsvdSecCnt+(NumFAT*FATSz)+Drive_p->RootDirSecs);
   CntClus=DataSec/Drive_p->SecPerClus;
   Drive_p->StartofFAT=RsvdSecCnt+MBR_RelSec;
   Drive_p->FirstDataSec=Drive_p->StartofFAT+(NumFAT*FATSz)+Drive_p->RootDirSecs;
   if(CntClus<65525)
   {
     Drive_p->FATtype=FAT16;
	 Drive_p->FATshift=FAT16_shift;
	 printf("FAT16 Detected...\n\r");
	 Drive_p->FirstRootDirSec=Drive_p->StartofFAT+(NumFAT*FATSz);

   }
   else
   {
     Drive_p->FATtype=FAT32;
	 Drive_p->FATshift=FAT32_shift;
	 printf("FAT32 Detected...\n\r");
     Drive_p->FirstRootDirSec=((RootClus-2)*Drive_p->SecPerClus)+Drive_p->FirstDataSec;
   }
   return No_Disk_Error;
}


/***********************************************************************
DESC: Calculates the First Sector of a given Cluster 
INPUT: uint32_t Cluster number
RETURNS: uint32_t sector number 
CAUTION: 
************************************************************************/
uint32_t First_Sector (uint32_t Cluster_num)
{
   uint32_t Sector_num;
 //  FS_values_t * Drive_p;

  // Drive_p=Export_Drive_values();
   if(Cluster_num==0) 
   {
       Sector_num=Drive_values.FirstRootDirSec;
   }
   else
   {
      // Sector_num=((Cluster_num-2)*Drive_p->SecPerClus)+Drive_p->FirstDataSec;
		 Sector_num=((Cluster_num-2)*Drive_values.SecPerClus)+Drive_values.FirstDataSec;
   }
   return Sector_num;
}



/***********************************************************************
DESC: Prints all short file name entries for a given directory 
INPUT: Starting Sector of the directory and the pointer to a 
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint16_t number of entries found in the directory
CAUTION: Supports FAT16, SD_shift must be set before using this function
************************************************************************/

/*

uint16_t  Print_Directory(uint32_t Sector_num, uint8_t xdata * array_in)
{ 
   uint32_t Sector, max_sectors;
   uint16_t i, entries;
   uint8_t temp8, j, attr, out_val, error_flag;
   uint8_t * values;

   values=array_in;
   entries=0;
   i=0;
   if (Sector_num<Drive_values.FirstDataSec)  // included for FAT16 compatibility
   { 
      max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
   }
   else
   {
      max_sectors=Drive_values.SecPerClus;
   }
   Sector=Sector_num;
   AMBERLED=0;
   nCS0=0;
   error_flag=SEND_COMMAND(17,(Sector<<Drive_values.SDtype));
   if(error_flag==no_errors) error_flag=read_block(values,512);
   nCS0=1;
   AMBERLED=1;
   if(error_flag==no_errors)
   {
          YELLOWLED=0;
     do
     {
 
	    temp8=read8(0+i,values);  // read first byte to see if empty
        if((temp8!=0xE5)&&(temp8!=0x00))
	    {  
	       attr=read8(0x0b+i,values);
		   	YELLOWLED=1;
		   if((attr&0x0E)==0)   // if hidden, system or Vol_ID bit is set do not print
		   {
		      entries++;
			  printf("%5d. ",entries);  // print entry number with a fixed width specifier
		      for(j=0;j<8;j++)
			  {
			     out_val=read8(i+j,values);   // print the 8 byte name
			     putchar(out_val);
			  }
              if((attr&0x10)==0x10)  // indicates directory
			  {
			     for(j=8;j<11;j++)
			     {
			        out_val=read8(i+j,values);
			        putchar(out_val);
			     }
			     printf("[DIR]\n");
			  }
			  else       // print a period and the three byte extension for a file
			  {
			     putchar(0x2E);       
			     for(j=8;j<11;j++)
			     {
			        out_val=read8(i+j,values);
			        putchar(out_val);
			     }
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
              nCS0=0;
              error_flag=SEND_COMMAND(17,(Sector<<Drive_values.SDtype));
              if(error_flag==no_errors) error_flag=read_block(values,512);
			  if(error_flag!=no_errors)
			    {
			      entries=0;   // no entries found indicates disk read error
				  temp8=0;     // forces a function exit
			    }
                nCS0=1;
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

 */

/***********************************************************************
DESC: Uses the same method as Print_Directory to locate short file names,
      but locates a specified entry and returns and cluster  
INPUT: Starting Sector of the directory, an entry number and a pointer to a 
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint32_t with cluster in lower 28 bits.  Bit 28 set if this is 
         a directory entry, clear for a file.  Bit 31 set for error.
CAUTION: 
************************************************************************/

/*

uint32_t Read_Dir_Entry(uint32_t Sector_num, uint16_t Entry, uint8_t xdata * array_in)
{ 
   uint32_t Sector, max_sectors, return_clus;
   uint16_t i, entries;
   uint8_t temp8, attr, error_flag;
   uint8_t * values;

   values=array_in;
   entries=0;
   i=0;
   return_clus=0;
   if (Sector_num<Drive_values.FirstDataSec)  // included for FAT16 compatibility
   { 
      max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
   }
   else
   {
      max_sectors=Drive_values.SecPerClus;
   }
   Sector=Sector_num;
   nCS0=0;
   error_flag=SEND_COMMAND(17,(Sector<<Drive_values.SDtype));
   if(error_flag==no_errors)  error_flag=read_block(values,512);
   nCS0=1;
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
                   return_clus&=0x0F;            // makes sure upper four bits are clear
				   return_clus=return_clus<<8;
                   return_clus|=read8(20+i,values);
                   return_clus=return_clus<<8;
                }
                return_clus|=read8(27+i,values);
			    return_clus=return_clus<<8;
                return_clus|=read8(26+i,values);
			    attr=read8(0x0b+i,values);
			    if(attr&0x10) return_clus|=directory_bit;
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
              nCS0=0;
              error_flag=SEND_COMMAND(17,(Sector<<Drive_values.SDtype));
              if(error_flag==no_errors)  error_flag=read_block(values,512);
              nCS0=1;
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

*/


/***********************************************************************
DESC: Finds the next cluster number of a file in the FAT with an input
      parameter of the current cluster number.
INPUT: Current cluster number.
RETURNS: Next Cluster number from the FAT
CAUTION: 
************************************************************************/


uint32_t Find_Next_Clus(uint32_t Cluster_num, uint8_t xdata * array_name)
{
   uint32_t Sector, return_clus;
   uint16_t FAToffset;
   uint8_t * values;
 //  FS_values_t * Drive_p;

  // Drive_p=Export_Drive_values();
//Drive_values
   values=array_name;
	 Sector=(Cluster_num>>(Drive_values.BytesPerSecShift-Drive_values.FATshift))+Drive_values.StartofFAT;
		Read_Sector(Sector, Drive_values.BytesPerSec,values);
     FAToffset=(uint16_t)((Cluster_num<<(Drive_values.FATshift))&(Drive_values.BytesPerSec-1));
   if(Drive_values.FATtype==FAT32)    // FAT32
   {
       return_clus=(read32(FAToffset,values)&0x0FFFFFFF);
   }
   else if(Drive_values.FATtype==FAT16)    // FAT16
   {
       return_clus=(uint32_t)(read16(FAToffset,values));
   }
	
 ///  Sector=(Cluster_num>>(Drive_p->BytesPerSecShift-Drive_p->FATshift))+Drive_p->StartofFAT;
 //  Read_Sector(Sector, Drive_p->BytesPerSec,values);
  // FAToffset=(uint16_t)((Cluster_num<<Drive_p->FATshift)&(Drive_p->BytesPerSec-1));
//   if(Drive_p->FATtype==FAT32)    // FAT32
//   {
//       return_clus=(read32(FAToffset,values)&0x0FFFFFFF);
//   }
//   else if(Drive_p->FATtype==FAT16)    // FAT16
//   {
//       return_clus=(uint32_t)(read16(FAToffset,values));
//   }
   return return_clus;
}
   



/***********************************************************************
DESC: Prints the sectors of a file until the user hits X
      Prints the Cluster Number and Sector number of each sector as well
INPUT: Starting Cluster of the file and the start of a 
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint8_t 0
CAUTION: 
************************************************************************/


//uint8_t Open_File(uint32_t Cluster_num, uint8_t xdata * array_in)
//{
//   uint32_t Sector, SecOffset;
//   uint16_t entries;
//   uint8_t temp8;
//   uint8_t * values;
//   FS_values_t * Drive_p;

//   Drive_p=Export_Drive_values();
//   values=array_in;
//   entries=0;
//   SecOffset=0;
//   
//   
//   do
//   {
//      if(SecOffset==0) Sector=((Cluster_num-2)*Drive_p->SecPerClus)+Drive_p->FirstDataSec;
//      printf("Cluster#: %9lu,  Sector#: %9lu,  SecOffset: %lu\n",Cluster_num,(Sector+SecOffset),SecOffset);
//      Read_Sector((Sector+SecOffset), Drive_p->BytesPerSec, values);      
//      print_memory(512,values);
//      SecOffset++;
//      if(SecOffset==Drive_p->SecPerClus)
//      {
//         Cluster_num=Find_Next_Clus(Cluster_num,values);
//         SecOffset=0;
//      }
//      printf("Press Space to Continue or X to exit\n");
//      do
//      {
//        temp8=getchar();
//      }while((temp8!=0x20)&&(temp8!=0x58)&&(temp8!='P'));
//      if(temp8=='P')
//      {
//         Play_Song(Cluster_num);
//      }
//   }while(temp8==0x20);
//   putchar(0x0d);
//   putchar(0x0a);
//return 0;
//}




