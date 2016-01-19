#ifndef _STA013_Config_H
#define _STA013_Config_H

#include "Main.H"



#define STA013 (0x43)

// ------ Public function prototypes -------------------------------
uint8_t STA013_init(void);
extern uint8_t code CONFIG;
extern uint8_t code CONFIG2;


#endif