/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: main file                                                                      *
***********************************************************************************************/

#include <__cross_studio_io.h>
#include <msp430f5438a.h>
#include <msp430.h>
#include "radiocmds.h"
#include "peripheral.h"
#include "ISR.h"
#include "pins.h"



void main(void){
  WDTCTL = WDTPW + WDTHOLD;

  // Set LED pin DIR
  P1DIR |= 0xFF;

  // turn on a LED to indicate power
  P1OUT |= BIT0;

  // Setup Functions
  Clock_Setup();

  Radio_SPI_setup();
  Reset_Radio();
  __delay_cycles(800);                   // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
  Write_RF_Settings();                
  Radio_Interrupt_Setup();
  Radio_Strobe(TI_CCxxx0_SRX);          //Initialize CC2500 in Rx mode

 // NOTE This is for testing
  for(;;){
 // Radio_Read_Registers(0x26);
   Send_Dummy ();
      __delay_cycles(800);                   // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
  }
  //TODO explain this part to Justin
  //_EINT();  // set global IR enable 
  //LPM0;
}
