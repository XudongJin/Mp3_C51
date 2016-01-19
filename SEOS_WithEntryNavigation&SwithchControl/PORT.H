/*------------------------------------------------------------------*-

   Port.H (v1.00)

  ------------------------------------------------------------------

   Port Header (see Chap 5) for the project DINOSAUR (Chap 8)

   COPYRIGHT
   ---------

   This code is associated with the book:

   EMBEDDED C by Michael J. Pont 
   [Pearson Education, 2002: ISBN: 0-201-79523-X].

   This code is copyright (c) 2001 by Michael J. Pont.
 
   See book for copyright details and other information.

-*------------------------------------------------------------------*/

#ifndef _PORT_H
#define _PORT_H

// ------ MP3 Player, includes I2C connections --------------------------

// Port 0
sbit P0_0 = P0^0;
sbit P0_1 = P0^1;
sbit P0_2 = P0^2;
sbit P0_3 = P0^3;
sbit P0_4 = P0^4;
sbit P0_5 = P0^5;
sbit P0_6 = P0^6;
sbit P0_7 = P0^7; 

// Port 1
sbit SDA = P1^0;
sbit SCL = P1^1;
sbit DATA_REQ = P1^2;
sbit BIT_EN = P1^3;
sbit nCS0 = P1^4;
sbit MISO_pin = P1^5;
sbit SCK_pin = P1^6;
sbit MOSI_pin = P1^7;

#define ON (0)
#define OFF (1)

#define STOP_MASK (1<<0)
#define PLAY_MASK (1<<1)
#define SCROLLUP_MASK (1<<2)
#define SCROLLDOWN_MASK (1<<3)
// Port 2
//sbit SW1 = P2^0;
//sbit SW2 = P2^1;
//sbit SW3 = P2^2;
//sbit SW4 = P2^3;
sbit REDLED = P2^4;
sbit YELLOWLED = P2^5;
sbit AMBERLED = P2^6;
sbit GREENLED = P2^7;

// Port 3
sbit TxD_pin = P3^0;
sbit RxD_pin = P3^1;
sbit P3_2 = P3^2;
sbit STA013_RESET = P3^3;
sbit P3_4 = P3^4;
sbit P3_5 = P3^5;
sbit RS = P3^6;
sbit E  = P3^7; 


sfr SWITCH_PORT = 0xA0;
sfr LED_port = 0xA0;
sfr LCD_Data_Port = 0x80;

#endif

/*------------------------------------------------------------------*-
  ---- END OF FILE -------------------------------------------------
-*------------------------------------------------------------------*/



