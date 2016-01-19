#ifndef _SWITCH_H
#define _SWITCH_H

#include "Main.H"

#define DEBOUNCE_VALUE 20


typedef enum {
	NONE,
	SW_NOT_PRESSED,
	DEBOUNCE,
	SW_PRESSED,
	SW_HELD,
	R_DEBOUNCE
}Sw_State_t;

typedef struct {
	Sw_State_t sw_state;
	uint8_t debounce_time;
}Sw_global_t;

void Switch_init();
void Switch_Get_Input(uint8_t sw_mask ,Sw_global_t * sw_number);

#endif