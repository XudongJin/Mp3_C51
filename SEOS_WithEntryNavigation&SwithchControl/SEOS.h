#ifndef _SEOS_H
#define _SEOS_H

#include "Main.H"








#define ACTIVE 0
#define INACTIVE 1

#define ISR_TICK (11)


#define TASK_TIMEOUT (ISR_TICK-1)
#define Timer_Reload_10ms ((65536)-((OSC_FREQ*TASK_TIMEOUT)/(1020*OSC_PER_INST)))  
#define Timer_Reload_10ms_THx (Timer_Reload_10ms>>8)
#define Timer_Reload_10ms_TLx (Timer_Reload_10ms&0xff)

#define  Timer2_Reload ((65536)-((OSC_FREQ*ISR_TICK)/(1020*OSC_PER_INST)))
#define	 Timer2_Reload_H (Timer2_Reload/256)
#define	 Timer2_Reload_L (Timer2_Reload%256)

//typedef enum {
//	TRAFFIC_RED,
//	TRAFFIC_YELLOW,
//	TRAFFIC_GREEN,
//	TRAFFIC_AMBER,
//  TRAFFIC_START,
//	TRAFFIC_STOP,
//	TRAFFIC_RED_GREEN,
//	TRAFFIC_RED_YELLOW,
//	TRAFFIC_GREEN_YELLOW
//}Traffic_State_t;

typedef enum {
	STOP,
	PLAY,
	REWIND,
  FINISH,
	PAUSE
}Play_State_t;



typedef enum {
	DATA_IDLE_1,
	DATA_SEND_1,
	LOAD_BUFFER_1,
	FIND_CLUSTER_1,
	DATA_IDLE_2,
	DATA_SEND_2,
	LOAD_BUFFER_2,
	FIND_CLUSTER_2
}Mp3_State_t;








//#define STA013 (0x43)

// ------ Public function prototypes -------------------------------


void Play_Song_State_Machine(uint32_t Start_Cluster);




#endif