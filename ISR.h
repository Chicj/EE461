#ifndef _ISR_H
#define _ISR_H

//P1IV definitions
#define CC2500_GDO0_IV      P1IV_P1IFG7  // interrupt on P1.0
#define CC2500_GDO2_IV      P1IV_P1IFG3  // interrupt on P1.1

void Radio_Interrupt_Setup(void); // Enable RX interrupts only!  TX interrupt enabled in TX Start (transmit.c)!



#endif
