/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: peripheral.h is the header file to set declarations for peripheral.c           *
***********************************************************************************************/

#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H
#define TOTALCOUNT 125

// functions list
void Radio_SPI_setup(void);
void Clock_Setup(void);
int IncrementVcore(void);
int DecrementVcore(void);
void UART_INIT(void);
void Send_UART(char * mystring);
unsigned long get_time_tick(void);
void set_time_tick(unsigned long);
unsigned long setget_time_tick(unsigned long);


extern char UARTBuff[];  // UART scratch pad
extern unsigned int TX_state;
extern unsigned long time_tick;
#endif
