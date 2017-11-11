/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: ISR.c will contain interrupt service routines for the microcontroller          *
***********************************************************************************************/

#include <msp430f5438a.h>
#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "ISR.h"
#include "radiocmds.h"
#include "peripheral.h"
#include "pins.h"

void Radio_Interrupt_Setup(void){ // Enable RX interrupts only!  TX interrupt enabled in TX Start
  // Use GDO0 and GDO2 as interrupts to control TX/RX of radio
  P1DIR = 0;          // Port 1 configured as inputs (i.e. GDO0 and GDO2 are inputs)
  P1IES = 0;          // flag is set with a low to high transition
  P1IFG = 0;          // Clear all flags <-- do this after IES as it will set a BIT2 high (pg 413 family user guide)
  P1IE |= CC2500_GDO2|CC2500_GDO0; // Enable GDO0 and GDO2 interrupts
}

void  Port1_ISR (void) __interrupt[PORT1_VECTOR]{
    //read P1IV to determine which interrupt happened
    //reading automatically resets the flag for the returned state
    switch(P1IV){
       case CC2500_GDO0: // [CC2500_GDO0] RX is set up to assert when RX FIFO is greater than FIFO_THR.  This is an RX function only
            Radio_Rx(); // read Rx buffer
        break;
    // TX state
        case CC2500_GDO2: //[CC2500_GDO2] TX is set up to assert when TX FIFO is above FIFO_THR threshold.            
          switch(TX_state) 
          {
              case IDLE:  
                   break;
              case TX_START:  //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Packet in progress
                    //Radio_TX();
                   break;
              case TX_RUNNING: //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Packet in progress
                    Radio_TX_Running();

                   break;
              case TX_END:     //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Last part of packet to transmit
                   Radio_TX_End();
                   
                   break;
              default:
                break;         
          } 
        break;
        default:
        break;
        }
}

//**************************************************************** UART ISR ************************************************************
unsigned char globali = 0;
unsigned char mystring[50];
// UCA1 UART interrupt service routine 
void UART_IR(void) __interrupt[USCI_A1_VECTOR]{

  switch(UCA1IV){
    case  USCI_UCRXIFG:
        //save UART input
            RxBuffer[globali]=UCA1RXBUF;    // save UART into buffer 
            UCA1TXBUF = RxBuffer[globali];  // loop input chars back to terminal 
            RxBuffer[globali+1] = NULL;         // make sure the RxBuffer ends with a null 
            while(!(UCA1IFG & UCTXIFG));  // delay for UART TX

          if(RxBuffer[globali] == '\r' ||RxBuffer[globali] == '\n'){ // check input string for new line
            UCA1TXBUF = '\n';             // start a new line
            while(!(UCA1IFG & UCTXIFG));  // delay for UART TX

            
            globali = 0;        // reset global counter       
            while(mystring[globali] != '\0'){ //print # of chars input 
              UCA1TXBUF = mystring[globali]; // spit out # of entered chars
              while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
              globali++;  // increment counter
            }
            globali=0;  // reset global counter 

          }
      break;
   }
}
