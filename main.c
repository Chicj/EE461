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
extern unsigned char mainFlag;


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
  mainFlag = 0;

  P1DIR |= BIT0 | BIT1;                         // Set LED pin DIR
  P1OUT |= BIT0;                                // turn on a LED to indicate power

   _EINT();                                    // set global IR enable 
   
   while(1){
   
     if(mainFlag==1){
       if((time_tick % 500) == 0){
          unsigned long timeTemp=0;
          timeTemp = get_time_tick();
          send_packet(0x01,timeTemp,inf,19);
          sprintf(UARTBuff,"Packet sent at %li\r\n",timeTemp);
          Send_UART(UARTBuff);
          while(!(UCA1IFG & UCTXIFG));
          radio_flush();
          mainFlag = 0;
      }
    }
    
  }

}
