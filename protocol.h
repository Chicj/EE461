/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: protocol.h is the header file to set declarations for protocol.c               *
***********************************************************************************************/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

// function list
void FCS(unsigned char *dat, unsigned int *len);
void bitstuff(unsigned char *dat, unsigned int *len);

sync = 0x7E;

#endif