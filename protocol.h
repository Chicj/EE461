/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: protocol.h is the header file to set declarations for protocol.c               *
***********************************************************************************************/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

// function list
void insert_FCS(unsigned char *dat);
void bitstuff(unsigned char *dat);
void send_packet(unsigned char dest, unsigned long clockData, unsigned char *info);

extern unsigned char RxBuffer[], RXMASK, RxBit;
extern unsigned int RxBuffer_len, RxBufferPos, RxBytesRemaining, RXBuffer_Len;

#endif
