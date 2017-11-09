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

#endif
