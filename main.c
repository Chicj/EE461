#include <__cross_studio_io.h>
#include <msp430f6779a.h>
#include "radiocmds.h"
#include "peripheral.h"
#include "ISR.h"

void main(void){
  //set all LED pins DIR/ turn on a LED
  P7DIR |= 0xFF;
  P7OUT |= BIT0;

  debug_printf("WSN project\n");
  debug_exit(0);

  // all setup functions could be bundled later.... 
  Radio_SPI_setup();
  Write_RF_Settings();                
  Radio_Interrupt_Setup();
  Radio_Strobe(TI_CCxxx0_SRX);          //Initialize CC2500 in Rx mode


  _EINT();  // set global IR enable 
//  WDT_KICK(); 
  LPM0;
}
