#include <msp430f5438a.h>
#include "peripheral.h"
#include "radiocmds.h"
#include "pins.h"

/* Will contain all peripheral code 
 eUSCI_A2 is hardwired to the CP2102 and therefore should not be used for anything other then UART to serial connection
 eUSCI_B1 will be the SPI driver talking to the CC2500 radio

*/

//******************************************************** SPI **************************************
//radio connection SPI setup
void Radio_SPI_setup(void){
//Radio SPI on P4: P4.2=UCB1SIMO, P4.1=USB1SOMI, P4.0=UCB1CLK
  UCB0CTLW0 |= UCSWRST;                           // Put UCB1 into reset
  UCB0CTLW0  = UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC|UCSSEL_2|UCSWRST;  // Data Read/Write on Rising Edge
                                                  // MSB first, Master mode, 3-pin SPI
                                                  // Synchronous mode
                                                  // SMCLK
  UCB0BRW = 16;                                   // Set frequency divider so SPI runs at 16/16 = 1 MHz
  UCB0CTLW0 &= ~UCSWRST;  //Bring UCB1 out of reset state
 
// ************************************************* PIN setup 
  //Radio CS P3.0=CC2500_CS (ENABLE1) 
  //Initial state for CS is High, CS pulled low to initiate SPI
  P3DIR |= CS_CC2500;                     //Set output for CC2500_1 CS
  
  radio_SPI_desel();                 // Ensure CS for CC2500 is disabled

  //Set pins for SPI usage
  P3SEL |= RADIO_PINS_SPI;

}
//******************************************************** Clock **************************************
 //TODO test this
 void ClockSetUp(void){ // will set MCLK to 20 MHz
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
 
  /*// setup MSP430f5xxx breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT1;  // Set Port 11 pin 1 direction as an output
  P11SEL  |= BIT1; // Set port 11 pin 1 as MCLK signal 
  
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL  |= BIT2; // Set port 11 pin 2 as SMCLK signal 
  */

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

//******************************************************** UART **************************************
// UART is connected UART_Tx P3.5 UCA1, UART_Rx P3.4 
/*int UART_INIT(){
  int status =-1;

  P5SEL |= BIT4|BIT5; // select UART functionality 
//  USCI should be in reset before configuring - only configure once 
    if (UCA1CTL1 & UCSWRST) {
 
        // Set clock source to SMCLK 
        UCA1CTL1 |= UCSSEL_2;
 
        // Find the settings from the baud rate table 
        for (int i = 0; i < ARRAY_SIZE(baud_tbl); i++) {
            if (baud_tbl[i].baud == config->baud) {
                break;
            }
        }
 
        if (i < ARRAY_SIZE(baud_tbl)) {
            // Set the baud rate 
            UCA1BR0 = baud_tbl[i].UCAxBR0;
            UCA1BR1 = baud_tbl[i].UCAxBR1;
            UCA1MCTL = baud_tbl[i].UCAxMCTL;
 
            // Enable the USCI peripheral (take it out of reset) 
            UCA0CTL1 &= ~UCSWRST;
            status = 0;
        }
    }

  return status;
}

*/
