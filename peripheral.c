/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: peripheral.c will contain all peripherals for the microcontroller              *
***********************************************************************************************/

#include <msp430f5438a.h>
#include <msp430.h>
#include "peripheral.h"
#include "radiocmds.h"
#include "pins.h"

/******************************************* SPI  Setup ***************************************/

//Function for radio connection SPI setup
void Radio_SPI_setup(void){
  unsigned short x;
  UCB0CTL1 |= UCSWRST;                                             // Put UCB0 into reset
  UCB0CTL0  |= UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC; // Data Read/Write on Rising Edge 
  UCB0CTL1 |= UCSSEL_2;
  //UCB0CTLW0 = 0xA9C1;                                             // MSB first, Master mode, 3-pin SPI
                                                                    // Synchronous mode
                                                                    // SMCLK
  UCB0CTLW0 &= ~UCSWRST;                                            // Bring UCB0 out of reset state
 
  //Initial state for CS is High, CS pulled low to initiate SPI
  P3DIR |= CS_CC2500;                                               // Set CS as an output DONT UNSELECT WHEN IN STUFF THIS AGAIN 

  radio_SPI_desel();                                                // Disable CS for radio

  P3SEL |= RADIO_PINS_SPI;                                          // Set pins for SPI usage

}
/***************************************** Clock  Setup *****************************************/
 //TODO test this and clean up the code
 void Clock_Setup(void){ // will set MCLK to 25 MHz and SMCLK to 17 MHz
  // setup MSP430f5xxx breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT1;  // Set Port 11 pin 1 direction as an output
  P11SEL  |= BIT1; // Set port 11 pin 1 as MCLK signal 
  
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL  |= BIT2; // Set port 11 pin 2 as SMCLK signal

   DecrementVcore();
   DecrementVcore();
   DecrementVcore();
 
/*
  // board PWM pin setups
  P8DIR |= BIT6;  // set up P8.6 as an OUT1 in TimerA1  25%
  P7DIR |= BIT3;  // set up P7.3 as an OUT2 TimerA1 75%

  P8SEL |= BIT6;  // set up P8.6 as an Output mode 
  P7SEL |= BIT3;  // set up P7.3 as an Output mode 
*/

  // TAxCCRn = Content of capture/compare register n will set the count up to val
  TA1CCR0 = TOTALCOUNT; // set count up val 
  TA1CCR1 = 94;   //P8.6  75%
  TA1CCR2 = 32;   //P7.3  25%
  
 }

//**************************************************************** UART*************************************
// UART UCA1 on MSP-EXP430F5438 is connected to USB
//SET UP UART 
void UART_INIT(void){
  // setup pins 
  P5SEL |= BIT6|BIT7;//UCA1STE|UCA1TXD|UCA1RXD

  UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA1CTL1 |= UCSSEL_2;                     // SMCLK
  UCA1BR0 = 6;                              // 1MHz 9600 (see User's Guide)
  UCA1BR1 = 0;                              // 1MHz 9600
  UCA1MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
                                            // over sampling
  UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
  /*
  UCA1CTL0   |= UCPEN;// --> a good idea but works w/out
  //selected parity enable |odd parity| LSB| 8 bit| one stop bit| UART| async mode
  UCA1CTL1   |= UCSSEL__SMCLK|UCSWRST;
  // use SMCLK| no  erroneous RX will IR| break chars do no IR| not in sleep mode (UCDORM)| next frame transmitted is data| next frame transmitted is not a break| software rest enabled 
  //Set UART bit rate from pg 955 fam guide "commonly used baud rate settings"  
  UCA1BR0 = 2;
  // Modulation control register pg 951 fam guide 
  //17MHZ/460800 = 36.89 = 32 + 4.89 = 2*16 + 4 + .89*8 = 7.12 = 7 
  UCA1MCTL  |= UCBRF_4|UCBRS_7|UCOS16; //set modulation | enabled oversampling mode <-- note UCBRF 5 dosent work when using DCORSEL_4
  UCA1CTL1  &= ~UCSWRST; // enable UART lock UART settings 
  UCA1IE |= UCRXIE; // Enable RX interrupts */
}

void Send_UART(char * mystring){
  int i = 0;

    while(mystring[i] != '\0'){   //print # of chars input 
    UCA1TXBUF = mystring[i];      // spit out # of entered chars
    while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
    i++;                    // increment counter
  }
}

