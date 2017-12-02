/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: protocol.h is the header file to set declarations for protocol.c               *
***********************************************************************************************/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

// function list
void insert_FCS(unsigned char *dat, unsigned int *datLength);
void bitstuff(unsigned char *dat, unsigned int *len);
void send_packet(unsigned char dest, unsigned long clockData, unsigned char *info, unsigned char infoLength);
void find_sync(unsigned char *indat, unsigned int inlen);
void unscramble(unsigned char *indat, unsigned int inlen);
void packetReceived(void);

extern unsigned char RXMASK, RxBit, source;

#endif
