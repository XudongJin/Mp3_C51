#include "AT89C51RC2.h"
#include "PORT.H"
#include "main.h"
#include "SDCard.h"
#include "print_bytes.h"
#include "SPI.h"
#include <stdio.h>

static uint8_t  idata SD_Card_Type;

/***********************************************************************
DESC:    Sends Commands to SD card using the SPI peripheral
         cmnd=Command number to be sent
         argum=32bit argument for a command
RETURNS: error flag
CAUTION: Must be followed by a read to capture SD card response
         nCS0 must be set/cleared outside of this routine 
************************************************************************/
uint8_t SEND_COMMAND(uint8_t cmnd, uint32_t argum)
{
 uint8_t SPI_send, return_val, SPI_return, error_flag;

 return_val=no_errors;
 if(cmnd<64)
 {
   SPI_send=cmnd | 0x40;
   error_flag=SPI_Transfer(SPI_send,&SPI_return);
   if((error_flag)==no_errors)
   {
     SPI_send=argum>>24;   // MSB
     error_flag=SPI_Transfer(SPI_send,&SPI_return);
   }
   else
   {
     return_val=SPI_error;
   }
   if((return_val==no_errors)&&(error_flag==no_errors))
   {
     argum=argum & 0x00ffffff;
     SPI_send=argum>>16;  // BYTE2
     error_flag=SPI_Transfer(SPI_send,&SPI_return);
   }
   else
   {
     return_val=SPI_error;
   }
   if((return_val==no_errors)&&(error_flag==no_errors))
   {
     argum=argum & 0x0000ffff;
     SPI_send=argum>>8;   // BYTE1
     error_flag=SPI_Transfer(SPI_send,&SPI_return);
   }
   else
   {
     return_val=SPI_error;
   }     
   if((return_val==no_errors)&&(error_flag==no_errors))
   {
     SPI_send=argum & 0x000000ff;  // LSB
     error_flag=SPI_Transfer(SPI_send,&SPI_return);
   }
   else
   {
     return_val=SPI_error;
   }
   if((return_val==no_errors)&&(error_flag==no_errors))
   {         
      if (cmnd == 0)
      {
         SPI_send=0x95;  // CRC7 and end bit for CMD0
      }
      else if (cmnd == 8)
      {
         SPI_send=0x87;   // CRC7 and end bit for CMD8
      }
      else
      {
         SPI_send=0x01;  // end bit only for other commands
      }
      error_flag=SPI_Transfer(SPI_send,&SPI_return);
    }
 }
 else
 {
   return_val=illegal_cmnd;
 }
 return return_val;  
}
/***********************************************************************
DESC:    Read values from SPI port, 0xff sent for each byte read 
		 num_bytes=number of bytes to read
RETURNS: error flag
         *valout=address of array of bytes read
CAUTION: Used to capture SD card response
         nCS0 must be set/cleared outside of this routine 
************************************************************************/
uint8_t response(uint8_t num_bytes, uint8_t * valout)
{
   uint8_t index,return_val,error_flag, SPI_return;

   return_val=no_errors;
   do
   {
      error_flag=SPI_Transfer(0xFF,&SPI_return);
      index++;
   }while(((SPI_return&0x80)==0x80)&&(index!=0)&&(error_flag==no_errors));
   if(error_flag!=no_errors)
   {
      return_val=SPI_error;
   }
   else if(index==0)
   {
      return_val=timeout_error;
   }
   else
   {
     *valout=SPI_return;
     if((SPI_return==0x00)||(SPI_return==0x01))
     {
       if(num_bytes>1)
       {
         for(index=1;index<num_bytes;index++)
         {
            error_flag=SPI_Transfer(0xFF,&SPI_return);
            *(valout+index)=SPI_return;
         }
       }
     }
     else
     {
        return_val=response_error;
     }
   }
   error_flag=SPI_Transfer(0xFF,&SPI_return);  // send 8 more clock cycles to complete read
   return return_val;
}


/**********************************************************************
DESC:    Reads a specifed number of bytes from SD card,
         0xff sent for each byte read 
RETURNS: error flag
         block of data stored in array_out[]
CAUTION: Used to capture SD card response
         nCS0 must be set/cleared outside of this routine 
************************************************************************/
uint8_t read_block(uint16_t num_bytes, uint8_t * array_out)
{
   uint8_t error_flag,return_val,SPI_return;
   uint16_t index;
   index=0;
   return_val=no_errors;
   do
   {
      error_flag=SPI_Transfer(0xFF,&SPI_return);
      index++;
   }while(((SPI_return&0x80)==0x80)&&(index!=0)&&(error_flag==no_errors));
   if(error_flag!=no_errors)
   {
      return_val=SPI_error;
   }
   else if(index==0)
   {
      return_val=timeout_error;
   }
   else
   {
     if (SPI_return==0x00)
     {
        index=0;
	    do
        { 
           error_flag=SPI_Transfer(0xFF,&SPI_return); 
           index++;
        }while((SPI_return==0xFF)&&(index!=0)&&(error_flag==no_errors));          // wait for data token response  
        if(error_flag!=no_errors)
        {
           return_val=SPI_error;
        }
        else if(index==0)
        {
          return_val=timeout_error;
        }
        else if(SPI_return==0xfe)
        {
          for(index=0;index<num_bytes;index++)
          {
             error_flag=SPI_Transfer(0xFF,&SPI_return);
             *(array_out + index)=SPI_return;
          }
          error_flag=SPI_Transfer(0xFF,&SPI_return); // discard byte 1 of CRC16	
          error_flag=SPI_Transfer(0xFF,&SPI_return); // discard byte 2 of CRC16
        }
	    else
	    {
	      return_val=data_error;
	    }
     }
     else
     {
        return_val=response_error;
     }
   }
 error_flag=SPI_Transfer(0xFF,&SPI_return);// send 8 more clock cycles to complete read
return return_val;
}


/**********************************************************************
DESC:    Sends the commands needed to initialize the SD card 
RETURNS: error flag
CAUTION:  
************************************************************************/
uint8_t SD_card_init(void)
{
   uint8_t i,error_status,error_flag,valsout[8],SPI_return;
   uint32_t argument;
   uint16_t timeout;
   error_status=no_errors;
   SD_Card_Type=unknown;
   nCS0=1;
   printf("SD Card Initialization ... \n\r");
   for(i=0;i<10;i++)
   {
       error_flag=SPI_Transfer(0xFF,&SPI_return);
   }
   GREENLED=0;
   printf("CMD0 sent ... ");   
   nCS0=0;  // CS card CS low
   error_flag=SEND_COMMAND(CMD0,0);
   if(error_flag==no_errors)
   {
     error_flag=response(1,valsout);
	 nCS0=1;
     GREENLED=1;
     printf("Response = %2.2bX\n\r",valsout[0]);
   }
   else
   {
     nCS0=1;
	 GREENLED=1;
	 REDLED=0;  // indicates error has occured.
   }
   if(error_flag!=no_errors)
   {
     error_status=error_flag;
   }
   if(error_status==no_errors)
   {
     GREENLED=0;
     printf("CMD8 sent ... ");
     nCS0=0;
     error_flag=SEND_COMMAND(CMD8,0x000001AA);
	 if(error_flag==no_errors)
     {
        error_flag=response(5,valsout);
		nCS0=1;
		GREENLED=1;
	    printf("Response = ");
        for(i=0;i<5;i++)
		{   
		        printf("%2.2bX ",valsout[i]);
		}
        putchar(CR);
        putchar(LF);
		if(valsout[4]!=0xAA)
		{
		   error_flag=response_error;
		}
     }
     else
     {
        nCS0=1;
        GREENLED=1;
	    REDLED=0;  // indicates error has occured.
     }
	 if(error_flag!=no_errors)
	 {
        if(error_flag==illegal_cmnd)
        {
           error_status=no_errors;
		   SD_Card_Type=Standard_Capacity;
		   printf("Version 1 SD Card detected.\n\r");
		   printf("Standard Capacity Card detected.\n\r");
        }
		else
		{
		   error_status=error_flag;
		}
	 }
	 else
	 {
		SD_Card_Type=Ver2;
		printf("Version 2 SD Card detected.\n\r");
	 }
   }
   if(error_status==no_errors)
   {
     GREENLED=0;
     printf("CMD58 sent ... ");
     nCS0=0;
     error_flag=SEND_COMMAND(CMD58,0);
	 if(error_flag==no_errors)
     {
        error_flag=response(5,valsout);
		nCS0=1;
		GREENLED=1;
	    printf("Response = ");
        for(i=0;i<5;i++)
		{   
		     printf("%2.2bX ",valsout[i]);
		}
        putchar(CR);
        putchar(LF);
		if((valsout[2]&0xFC)!=0xFC)
		{
		   error_flag=voltage_error;
		}
     }
     else
     {
        nCS0=1;
        GREENLED=1;
	    REDLED=0;  // indicates error has occured.
     }
	 if(error_flag!=no_errors)
	 {		
	    error_status=error_flag;
	 }
   }
   if(error_status==no_errors)
   {
     if(SD_Card_Type==Ver2)
	 {
	    argument=0x40000000;
	 }
	 else
	 {
	    argument=0;
	 }
	 timeout=0;
     GREENLED=0;
     printf("ACMD41 sent ... ");
     nCS0=0;
	 do
	 {
	    P3_5=0;
	    error_flag=SEND_COMMAND(CMD55,0);
		if(error_flag==no_errors)  error_flag=response(1,valsout);
		P3_5=1;
	    if((valsout[0]==0x01)||(valsout[0]==0x00))
		   error_flag=SEND_COMMAND(ACMD41,argument);
	    if(error_flag==no_errors) response(1,valsout);
		timeout++;
		if(timeout==0) error_flag=timeout_error;
	   }while(((valsout[0]&0x01)==0x01)&&(error_flag==no_errors));
	 if(error_flag==no_errors)
     {
		nCS0=1;
        GREENLED=1;
	    printf("Response = %2.2bX\n\r",valsout[0]);
     }
     else
     {
        nCS0=1;
        GREENLED=1;
	    REDLED=0;  // indicates error has occured.
     }
	 if(error_flag!=no_errors)
	 {		
	    error_status=error_flag;
	 }
   }
   if((error_status==no_errors)&&(SD_Card_Type==Ver2))
   {
     GREENLED=0;
     printf("CMD58 sent ... ");
     nCS0=0;
     error_flag=SEND_COMMAND(CMD58,0);
	 if(error_flag==no_errors)
     {
        error_flag=response(5,valsout);
		nCS0=1;
        GREENLED=1;
	    printf("Response = ");
        for(i=0;i<5;i++)
		{   
           printf("%2.2bX ",valsout[i]);
		}
        putchar(CR);
        putchar(LF);
		if((valsout[1]&0x80)!=0x80)
		{
		   error_flag=card_inactive;
		}
		else
		{
		   if((valsout[1]&0xC0)==0xC0)
		   {
		      SD_Card_Type=High_Capacity;
		      printf("High Capacity Card Detected\n\r");
		   }
		   else
		   {
		      SD_Card_Type=Standard_Capacity;
			  printf("Standard Capacity Card Detected\n\r");
			  GREENLED=0;
              printf("CMD16 sent ... ");
              nCS0=0;
              error_flag=SEND_COMMAND(CMD16,512);
	          if(error_flag==no_errors)
              {
                  error_flag=response(1,valsout);
		          nCS0=1;
                  GREENLED=1;
	              printf("Response = %2.2bX \n\r",valsout[0]);
                  printf("Block size set to 512 bytes\n\r");
		      }
			}
		 }
     }
     else
     {
        nCS0=1;
        GREENLED=1;
	    REDLED=0;  // indicates error has occured.
     }
	 if(error_flag!=no_errors)
	 {		
	    error_status=error_flag;
		print_error(error_status);
	 }
   }
 if(error_status!=no_errors)
 {		
	print_error(error_status);
 }
return error_status;
}

/**********************************************************************
DESC:    Returns the SD card type 
RETURNS: SD_Card_Type
CAUTION: SD card must be initialized first
************************************************************************/

uint8_t Return_SD_Card_Type(void)
{
   return SD_Card_Type;
}

/**********************************************************************
DESC:    Prints an error text based on the error code  
RETURNS: nothing
CAUTION: Mainly used for debugging
***********************************************************************/


void print_error(uint8_t error)
{
   if(error==timeout_error) printf("Timeout Error");
   else if(error==illegal_cmnd) printf("Illegal Command\n\r");
   else if(error==response_error) printf("Response Error");
   else if(error==data_error) printf("Data Token Error");
   else if(error==voltage_error) printf("Incompatible Voltage");
   else if(error==card_inactive) printf("Card is Inactive");
   else if(error==SPI_error) printf("SPI or Timeout Error");
   else printf("Unknown Error");
}

