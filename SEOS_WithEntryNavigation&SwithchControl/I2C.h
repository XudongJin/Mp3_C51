#ifndef _I2C_H
#define _I2C_H

#include "Main.H"




#define no_errors (0)
#define bus_busy_error (0x81)
#define NACK_error (0x82)

// ------ Public function prototypes -------------------------------
uint8_t I2C_Write(uint8_t device_addr,uint8_t num_bytes,uint8_t * send_array);
uint8_t I2C_Read(uint8_t device_addr,uint8_t num_bytes,uint8_t * rec_array);





#endif