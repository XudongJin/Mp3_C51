#include "Main.H"
#include "PORT.H"
#include "SDCard.h"
#include "Read_Sector.h"
#include "stdio.h"




uint8_t Read_Sector(uint32_t sector_number, uint16_t sector_size, uint8_t * array_for_data)
{
	uint8_t SDtype,error_flag=No_Disk_Error;   

	SDtype=Return_SD_Card_Type();
	nCS0=0;
    error_flag=SEND_COMMAND(17,(sector_number<<SDtype));
    if(error_flag==no_errors) error_flag=read_block(sector_size,array_for_data);
    nCS0=1;

	if(error_flag!=no_errors)
	{
       error_flag=Disk_Error;
    }
    return error_flag;
}
