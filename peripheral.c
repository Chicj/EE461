/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: peripheral.c will contain all peripherals for the microcontroller              *
***********************************************************************************************/

#include <msp430f5438a.h>
#include "peripheral.h"
#include "radiocmds.h"
#include "pins.h"

/******************************************* SPI  Setup ***************************************/

//TODO Explain this to Justin
//Function for radio connection SPI setup
void Radio_SPI_setup(void){

  UCB0CTLW0 |= UCSWRST;                                             // Put UCB0 into reset
  UCB0CTLW0  = UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC|UCSSEL_2|UCSWRST; // Data Read/Write on Rising Edge 
                                                                    // MSB first, Master mode, 3-pin SPI
                                                                    // Synchronous mode
                                                                    // SMCLK
  UCB0BRW = 16;                                                     // Set frequency divider so SPI runs at 16/16 = 1 MHz
  UCB0CTLW0 &= ~UCSWRST;                                            // Bring UCB0 out of reset state
 
  //Initial state for CS is High, CS pulled low to initiate SPI
  P3DIR |= CS_CC2500;                                               // Set CS for radio to low

  radio_SPI_desel();                                                // Disable CS for radio

  P3SEL |= RADIO_PINS_SPI;                                          // Set pins for SPI usage

}
/***************************************** Clock  Setup *****************************************/
 //TODO test this and clean up the code
 void Clock_Setup(void){ // will set MCLK to 25 MHz
  unsigned short   old_level; 

  // read the previous Vcore level 
  old_level = PMMCTL0 & (PMMCOREV_3); 
  //increment Voltage in the core until we get to level 3
  while (old_level < PMMCOREV_3) {
      IncrementVcore();
  }
  // set MCLK to 25 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 762; // Setting the freq multiplication factor * 1024
  UCSCTL1 |= DCORSEL_6;     // This sets the DCO frequency pg26 data sheet
  UCSCTL4 |= SELM__DCOCLKDIV + SELM__DCOCLKDIV;  // set output of FLLREFCLK --> DCOCLKDIV to input of MCLK and SMCLK  <-- This is default
 
  // setup MSP430f5xxx breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT1;  // Set Port 11 pin 1 direction as an output
  P11SEL  |= BIT1; // Set port 11 pin 1 as MCLK signal 
  
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL  |= BIT2; // Set port 11 pin 2 as SMCLK signal 

  // board PWM pin setups
  P8DIR |= BIT6;  // set up P8.6 as an OUT1 in TimerA1  25%
  P7DIR |= BIT3;  // set up P7.3 as an OUT2 TimerA1 75%

  P8SEL |= BIT6;  // set up P8.6 as an Output mode 
  P7SEL |= BIT3;  // set up P7.3 as an Output mode 

  // disable SMCLK requests
  UCSCTL8 |= ~SMCLKREQEN;

  // TAxCCRn = Content of capture/compare register n will set the count up to val
  TA1CCR0 = TOTALCOUNT; // set count up val 
  TA1CCR1 = 94;   //P8.6  75%
  TA1CCR2 = 32;   //P7.3  25%
 }
