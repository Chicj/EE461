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

unsigned char sync = 0x7E;
unsigned char source = 0x1;

#endif
