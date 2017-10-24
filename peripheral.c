#include <msp430f6779a.h>
#include "peripheral.h"
#include "radiocmds.h"

/* Will contain all peripheral code 
 eUSCI_A2 is hardwired to the CP2102 and therefore should not be used for anything other then UART to serial connection
 eUSCI_B1 will be the SPI driver talking to the CC2500 radio

*/

//******************************************************** SPI **************************************
//radio connection SPI setup
void Radio_SPI_setup(void){
//Radio SPI on P4: P4.2=UCB1SIMO, P4.1=USB1SOMI, P4.0=UCB1CLK
  UCB1CTLW0 |= UCSWRST;                           // Put UCB1 into reset
  UCB1CTLW0  = UCCKPH|UCMSB|UCMST|UCMODE_0|UCSYNC|UCSSEL_2|UCSWRST;  // Data Read/Write on Rising Edge
                                                  // MSB first, Master mode, 3-pin SPI
                                                  // Synchronous mode
                                                  // SMCLK
  UCB1BRW = 16;                                   // Set frequency divider so SPI runs at 16/16 = 1 MHz
  UCB1CTLW0 &= ~UCSWRST;  //Bring UCB1 out of reset state
 
// ************************************************* PIN setup 
  //Radio CS P5.1=CC2500_CS_1 (ENABLE1), P5.2=CC2500_CS_2 (ENABLE2), 
  //Initial state for CS is High, CS pulled low to initiate SPI
  P5DIR |= CS_CC2500;                     //Set output for CC2500_1 CS

  radio_SPI_desel();                 // Ensure CS for CC1101 is disabled

  //Set pins for SPI usage
  P4SEL0 |= RADIO_PINS_SPI;
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
