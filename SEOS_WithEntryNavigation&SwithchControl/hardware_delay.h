#ifndef _Hardware_Delay_H
#define _Hardware_Delay_H

#include "Main.H"

#define one_msec (1)

#define Timer_Reload_1ms (65536-((OSC_FREQ*one_msec)/(1020*OSC_PER_INST)))  
#define Timer_Reload_1ms_THx (Timer_Reload_1ms>>8)
#define Timer_Reload_1ms_TLx (Timer_Reload_1ms&0xff)


#define Timer_Reload_100us (65536-((OSC_FREQ)/(10020*OSC_PER_INST)))  
#define Timer_Reload_100us_THx (Timer_Reload_100us>>8)
#define Timer_Reload_100us_TLx (Timer_Reload_100us&0xff)
// ------ Public function prototypes -------------------------------


void DELAY_1ms_T1(uint16_t duration);
void DELAY_100us_T1(uint16_t duration);


#endif