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


  // Set LED pin DIR
  P1DIR |= 0xFF;

  // turn on a LED to indicate power
  P1OUT |= BIT0;

  // Setup Functions
  Clock_Setup();
  Radio_SPI_setup();
  Write_RF_Settings();                
  Radio_Interrupt_Setup();
  Radio_Strobe(TI_CCxxx0_SRX);          //Initialize CC2500 in Rx mode

  //TODO explain this part to Justin
  _EINT();  // set global IR enable 
//  WDT_KICK(); 
  
  LPM0;
}
