/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: main file                                                                      *
***********************************************************************************************/

#include <__cross_studio_io.h>
#include <msp430f5438a.h>
#include <msp430.h>
#include <stdio.h>
#include "radiocmds.h"
#include "peripheral.h"
#include "protocol.h"
#include "ISR.h"
#include "pins.h"

unsigned long timer=0;
unsigned char inf[23] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};

void main(void){

  WDTCTL = WDTPW + WDTHOLD;

  // setup function calls
  Clock_Setup();                  
  Radio_SPI_setup();
  Reset_Radio();
  __delay_cycles(800);                          // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
  Write_RF_Settings();                
  Radio_Interrupt_Setup();
  Radio_Strobe(TI_CCxxx0_SRX);                  //Initialize CC2500 in Rx mode
  UART_INIT();                                  // UART is set to 460800 baud | odd parity| LSB| 8 bit| one stop bit
  TimerA_Setup();                               // Start an ACLK fed TIMERA capture compare 
  Send_UART("EE646 WSN code.\r\n");

  P1DIR |= BIT0 | BIT1;                         // Set LED pin DIR
  P1OUT |= BIT0;                                // turn on a LED to indicate power
 
   _EINT();                                    // set global IR enable 
   LPM0;

}



// NOTE 
/*
Add transmit on a button push ? 
*/

/*
  for(;;){
    //streamCmd(argv[]); // make things easy to see on the CXA
    timer++;
      status = Radio_Read_Status(TI_CCxxx0_MARCSTATE);
      state=status&(~(BIT7|BIT6|BIT5));         // get the state of the radio from the full status byte
      sprintf(UARTBuff,"Radio State: 0x%02x \n\r",state);
      Send_UART(UARTBuff);
    // Only do things once every second
    if (timer%1000000 == 0){
      //send_packet(0x01, 0x01, timer, timer, inf);      
      P1OUT ^= BIT0;                            // turn on a LED to indicate power
    }

    // if timer is reaching the limit of unsigned long, reset it
    if (timer > 4000000000){
      timer = 0;
    }
  }
  */