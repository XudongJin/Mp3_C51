#ifndef _print_bytes_H
#define _print_bytes_H

#include "main.h"

// ASCII characters

#define CR (0x0D)
#define LF (0x0A)




// ------ Public function prototypes -------------------------------

void print_hex(uint8_t in);

void print_16bytes(uint8_t * array_in);

void print_memory(uint16_t number_of_bytes, uint8_t * array_in);


#endif