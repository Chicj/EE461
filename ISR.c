/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: ISR.c will contain interrupt service routines for the microcontroller          *
***********************************************************************************************/

#include <msp430f5438a.h>
#include "ISR.h"
#include "radiocmds.h"
#include "pins.h"

/*
 Will contain all interrupt code
 P1IV<-- TX/RX interrupts from the CC2500
*/

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
       case CC2500_GDO0_IV: // [CC2500_GDO0] RX is set up to assert when RX FIFO is greater than FIFO_THR.  This is an RX function only
            //TODO read buffer here
        break;
    // TX state
        case CC2500_GDO2_IV: //[CC2500_GDO2] TX is set up to assert when TX FIFO is above FIFO_THR threshold.  
        // Actual interrupt SR                
         /* switch(state) // TODO state will need to be updated.... not sure where to do this yet.
          {
              case IDLE:  
                   break;
              case TX_START:  //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Packet in progress
                    state = TX_RUNNING;
                    
                   break;
              case TX_RUNNING: //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Packet in progress
                    
                   break;
              case TX_END:  //Called on falling edge of GDO2, Tx FIFO < threshold, Radio in TX mode, Last part of packet to transmit
                   state = IDLE;
                   
                   break;
              default:
                break;         
          } */ 
        break;
        default:
        break;
        }
}

//TODO Add UART driver 
