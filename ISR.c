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
#include "protocol.h"

void Radio_Interrupt_Setup(void){                                                           // Enable RX interrupts only!  TX interrupt enabled in TX Start
  // Use GDO0 and GDO2 as interrupts to control TX/RX of radio
  P1DIR = 0;                                                                                // Port 1 configured as inputs (i.e. GDO0 and GDO2 are inputs)
  P1IES = 0;                                                                                // flag is set with a low to high transition
  P1IFG = 0;                                                                                // Clear all flags <-- do this after IES as it will set a BIT2 high (pg 413 family user guide)
  P1IE |= CC2500_GDO2|CC2500_GDO0;                                                          // Enable GDO0 and GDO2 interrupts
}

void  Port1_ISR (void) __interrupt[PORT1_VECTOR]{
    //read P1IV to determine which interrupt happened
    //reading automatically resets the flag for the returned state
    unsigned int i;
    switch(P1IV){
      case CC2500_GDO0_IV:                                                                  // [CC2500_GDO0] RX is set up to assert when RX FIFO is greater than FIFO_THR.  This is an RX function only
        Radio_Read_Burst_Registers(TI_CCxxx0_RXFIFO, RxTemp, RxThrBytes);
        WSN_RX(RxTemp, RxThrBytes);                                                         // The master receive function. It does all.
        /* NOTE I wrote a command that flushes the radios. Do we need the RX FLAG still?
        RXflag++;
        if(RXflag == 1){
        
          Radio_Strobe(TI_CCxxx0_SFRX);                 // flush the RX FIFO
          Radio_Strobe(TI_CCxxx0_SRX);                  //put CC2500 in Rx mode
          __delay_cycles(16000);
          sprintf(UARTBuff,"RX Trigger... Status: 0x%x \r\n",Radio_Read_Status(TI_CCxxx0_MARCSTATE));
          Send_UART(UARTBuff);
          RXflag = 0;
        }
        */
      break;
    // TX state
      case CC2500_GDO2_IV:                                                                  //[CC2500_GDO2] TX is set up to assert when TX FIFO is above FIFO_THR threshold.          
        radio_TX_state();
      /* NOTE I wrote a command that flushes the radios. Do we need the flag still?
        TXflag++;
        if(TXflag == 1){
        
          Radio_Strobe(TI_CCxxx0_SFTX);                 //flush the TX FIFO
          Radio_Strobe(TI_CCxxx0_SRX);                  //put CC2500 in Rx mode at the end of TX
          __delay_cycles(16000);
          sprintf(UARTBuff,"TX Trigger... Radio Status: 0x%x \r\n",Radio_Read_Status(TI_CCxxx0_MARCSTATE));
          Send_UART(UARTBuff);
          TXflag = 0;
        }
        */
      break;
      default: 
      break;
    }
}

//**************************************************************** UART ISR ************************************************************
unsigned char globali = 0;
// UCA1 UART interrupt service routine 
void UART_IR(void) __interrupt[USCI_A1_VECTOR]{

  switch(UCA1IV){
    case  USCI_UCRXIFG:
        //save UART input
            UARTBuff[globali]=UCA1RXBUF;    // save UART into buffer 
            UCA1TXBUF = UARTBuff[globali];  // loop input chars back to terminal 
            UARTBuff[globali+1] = NULL;     // make sure the UARTBuff ends with a null 
            while(!(UCA1IFG & UCTXIFG));    // delay for UART TX

          if(UARTBuff[globali] == '\r' ||UARTBuff[globali] == '\n'){ // check input string for new line [pars stuff in here]
            char error = parse_UART(UARTBuff);    // parsing code
            UCA1TXBUF = '\n';                     // start a new line
            while(!(UCA1IFG & UCTXIFG));          // delay for UART TX
            globali = 0;                          // reset global counter    
          }
          else if(UARTBuff[globali] == '\b'){     // handle back space
            UARTBuff[globali] = 0x00;             // erase data
            UCA1TXBUF = ' ';                      // start a new line and make it pretty
            while(!(UCA1IFG & UCTXIFG));          // delay for UART TX
            UCA1TXBUF = '\b';                      
            while(!(UCA1IFG & UCTXIFG));          // delay for UART TX
            globali--;
          }
          else{                                   // keep chuggin
            globali++;
          }
      break;
  }
}

//******************************************************************* TIMER A *********************************************************
void TimerA_Setup(void){ 
  TA0CTL |= TASSEL_2|MC_2|TACLR;   // use SMCLK | count mode | clear TAR
  TA0CCTL1 |= CCIE;                // enables interrupts on capture compare mode 
  TA0CCR1 = 1;                     //set ACLK capture point, ACLK = 32.84 kHz,
}

void TIMER_A0_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
  switch(TA0IV){
    case TA0IV_TA0CCR1:       // dont use TA0IV_TA0CCR0 
      P1OUT ^=BIT0;           // blink a led
      TA0CCR1 += 32;          //NOTE ... i think this is what we want... sets increment to 1024 
      time_tick++;            // increment for time info
    break;
    default:
    break;
   }
}


//*********************************** TIMER A Modified *******************************************************//
/*
// I am sure how to integrate data with CCIS PIN that's why put a switch and I don't have any switch to check the code//
void TimerA_Setup(void){ 
 // enables interrupts on capture mode //P8.1 to give input from board for now a switch, capture both edge of CCI1B/ synchronized/ Int enabled
  TA0CCTL1 |= CM_3| CCIS_1| SCS| CAP| CCIE;         
  TA0CTL |= TASSEL__ACLK| ID_1| MC__CONTINUOUS | TACLR;   // use AMCLK--->can use SMCLK |no pre-scaler count mode | clear TAR                   //set ACLK capture point, ACLK = 32.84 kHz,
}

/*
ISR called on capture for TACCR0.CCIFG
*/
/*
void TIMER_A0_ISR(void)
__interrupt[TIMER0_A1_VECTOR]          //flag will be cleared
{
unsigned int Tlast = 0;    // when captured last time
Counts= TA0CCR0 - Tlast;   // Interval
Tlast=TA0CCR0;             // time stored for next capture
  
}
*/
