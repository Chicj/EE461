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
void packetReceived(void);

extern unsigned char RXMASK, RxBit, source, sync;
extern long delta_sum,delta_avg,Rxcounter;

// Define the possible roles of the node and the corresponding address
#define CH0 = 0x00;
#define node01 = 0x01;
#define node02 = 0x02;
#define node03 = 0x03;
#define CH1 = 0x04;
#define node11 = 0x05;
#define node12 = 0x06;
#define node13 = 0x07;

#endif
