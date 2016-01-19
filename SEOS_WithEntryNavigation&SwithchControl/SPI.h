#ifndef _SPI_H
#define _SPI_H

#include "Main.H"

//------- Public Constant definitions --------------------------------
// error values
#define init_okay (0)
#define no_errors (0)
#define SPI_ERROR (0)
#define illegal_clockrate (0x0F)
#define TIMEOUT_ERROR (0x80)


// ------ Public function prototypes -------------------------------
uint8_t SPI_Master_Init(uint32_t clock_rate);
uint8_t SPI_Transfer(uint8_t data_input, uint8_t * data_output);

#endif