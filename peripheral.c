/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: peripheral.c will contain all peripherals for the microcontroller              *
***********************************************************************************************/

#include <msp430f5438a.h>
#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "peripheral.h"
#include "radiocmds.h"
#include "pins.h"

char UARTBuff[100]; // scratch pad for UART buffer
unsigned int  TX_state = IDLE;

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
  
  /*// (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 511;                          // Setting the freq multiplication factor * 32768 for final clk freq @ 16744448 Hz
  UCSCTL1 |= DCORSEL_4;                   // This sets the DCO frequency pg26 data sheet 12.3 -- 28.3 MHz
  UCSCTL4 |= SELA__DCOCLKDIV + SELA_0;    // set output of FLLREFCLK --> DCOCLKDIV to input of ACLK | setup ACLK 
 */
 }

//**************************************************************** UART *************************************
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
}

void Send_UART(char * mystring){
  int i = 0;

    while(mystring[i] != '\0'){   //print # of chars input 
    UCA1TXBUF = mystring[i];      // spit out # of entered chars
    while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
    i++;                          // increment counter
  }
}

// ADD TERMINAL COMMANDS HERE
int parse_UART(char *UARTBuff){ 
  if(~(strcmp(UARTBuff,"Tx")  | strcmp(UARTBuff,"TX") | strcmp(UARTBuff,"tx"))){
    Send_Dummy(); // TODO replace this with actual Tx command
  }
  else if(~(strcmp(UARTBuff,"status")  | strcmp(UARTBuff,"Status"))){
    status = Radio_Read_Status(TI_CCxxx0_MARCSTATE);
    state=status&(~(BIT7|BIT6|BIT5));         // get the state of the radio from the full status byte
    sprintf(UARTBuff,"Radio State: 0x%02x \n\r",state);
    Send_UART(UARTBuff);
    while(!(UCA1IFG & UCTXIFG));
  }
  else if(~(strcmp(UARTBuff,"dummy")  | strcmp(UARTBuff,"Dummy"))){
    Send_Dummy();
    sprintf(UARTBuff,"Dummy Packet sent at %il\r\n",get_time_tick());
    Send_UART(UARTBuff);
    while(!(UCA1IFG & UCTXIFG));
  }
  else if(~(strcmp(UARTBuff,"Reset Radio")  | strcmp(UARTBuff,"Radio Reset")  | strcmp(UARTBuff,"reset radio")  | strcmp(UARTBuff,"radio reset"))){
    Reset_Radio();
    __delay_cycles(800);     // Wait for radio to be ready before writing registers.cc2500.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
    Radio_Strobe(TI_CCxxx0_SRX);                  //Initialize CC2500 in Rx mode
    sprintf(UARTBuff,"Radio reset\r\n");
    Send_UART(UARTBuff);
    while(!(UCA1IFG & UCTXIFG));
  }
  else{
    sprintf(UARTBuff,"Enter a valid system command\r\n");
    Send_UART(UARTBuff);
    while(!(UCA1IFG & UCTXIFG));
    return 1;
  }
return 0;
}
//**************************************************************** TIMER A *************************************
unsigned long time_tick = 0; // This is the virtual clock var

//get current ticker time
unsigned long get_time_tick(void){
  unsigned long tmp;
  int en =_DINT();                    // disable all interrupts --> GIE = 0 (LOW)
  tmp=time_tick;
  if(en){
  _EINT();                            // enable all interrupts --> GIE = 1 (HIGH)
  }
  return tmp;
}

//set current ticker time
void set_time_tick(unsigned long nt){
  int en = _DINT();                   // disable all interrupts --> GIE = 0 (LOW)
  time_tick=nt;
  if(en){
   _EINT();                           // enable all interrupts --> GIE = 1 (HIGH)
  }
}

//set current ticker time and return old time
unsigned long setget_time_tick(unsigned long nt){
  unsigned long tmp;
  int en = _DINT();                   // disable all interrupts --> GIE = 0 (LOW)
  tmp=time_tick;
  time_tick=nt;
  if(en){
  _EINT();                            // enable all interrupts --> GIE = 1 (HIGH)
  }
  return tmp;
}
