#include "main.h"
#include "PORT.H"
#include "SEOS.h"
#include "Switch.h"


Sw_global_t  stop_button,play_button,scrollup_button,scrolldown_button;

uint8_t  DEBOUNCE_TIME_G;

void Switch_init(){
	
		DEBOUNCE_TIME_G=0;
		stop_button.sw_state = SW_NOT_PRESSED;
		stop_button.debounce_time=0;
	
		play_button.sw_state = SW_NOT_PRESSED;
		play_button.debounce_time=0;
	
		scrollup_button.sw_state = SW_NOT_PRESSED;
		scrollup_button.debounce_time=0;
	
		scrolldown_button.sw_state = SW_NOT_PRESSED;
		scrolldown_button.debounce_time=0;
	
}

void Switch_Get_Input(uint8_t sw_mask ,Sw_global_t * sw_number)
{
	switch(sw_number->sw_state)
	{
		case SW_NOT_PRESSED:
		{
			if((sw_mask & SWITCH_PORT)==0x00)
			{
				sw_number->sw_state = DEBOUNCE;
				DEBOUNCE_TIME_G=0;
				sw_number->debounce_time=DEBOUNCE_VALUE;
				
			}
			break;
		}
		case DEBOUNCE:
		{
			DEBOUNCE_TIME_G+=(uint8_t)ISR_TICK;
			if(DEBOUNCE_TIME_G>=(sw_number->debounce_time))
			{
				if((sw_mask & SWITCH_PORT)==0x00)	
					sw_number->sw_state= SW_PRESSED;
				else 
					sw_number->sw_state = SW_NOT_PRESSED;
				
			}
			break;
		}
		
		case SW_PRESSED:
		{ 
      //ACTION PERFORM()
			sw_number->sw_state = SW_HELD;
			break;
		}
	
		case SW_HELD:
		{

			if((sw_mask & SWITCH_PORT)==sw_mask)
			{	
					sw_number->sw_state = R_DEBOUNCE;
					DEBOUNCE_TIME_G=0;
			}
			break;
		}
		
		case R_DEBOUNCE:
		{
			DEBOUNCE_TIME_G+=(uint8_t)ISR_TICK;
			if(DEBOUNCE_TIME_G>=(sw_number->debounce_time))
			{

				if((sw_mask & SWITCH_PORT)==sw_mask)
					sw_number->sw_state = SW_NOT_PRESSED;
				else 
					sw_number->sw_state= SW_HELD;
				
			}
			break;
		}
	}
	
	
}