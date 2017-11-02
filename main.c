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
#include "ISR.h"
#include "pins.h"


void main(void){
  WDTCTL = WDTPW + WDTHOLD;
// setup function calls
  Clock_Setup();              // Setup Functions
  Radio_SPI_setup();
  __delay_cycles(800);
  Reset_Radio();
  __delay_cycles(800);          // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
  Write_RF_Settings();                
  Radio_Interrupt_Setup();
  Radio_Strobe(TI_CCxxx0_SRX);  //Initialize CC2500 in Rx mode
  __delay_cycles(100000);       // let things settle <-- otherwise we dont see start up message
  UART_INIT();                  // UART is set to 460800 baud | odd parity| LSB| 8 bit| one stop bit

 SendUART("EE646 WSN code.\r\n");

 //NOTE this will mess up our interrupts on P1 !
  P1DIR |= BIT0;    // Set LED pin DIR
  P1OUT |= BIT0;    // turn on a LED to indicate power

  _EINT();  // set global IR enable 
 // NOTE This is for testing
  for(;;){
    char status = Radio_Read_Status(TI_CCxxx0_MARCSTATE);
    state=status&(~(BIT7|BIT6|BIT5)); // get the state of the radio from the full status byte
    sprintf(TxBuffer,"The status of the radio is %i \r\n",state);
    SendUART(TxBuffer);
  __delay_cycles(8000000);                   // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable

  }
}
