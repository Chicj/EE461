/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: protocols.c contains the functions for following the network protocol          *
***********************************************************************************************/
#include <msp430f5438a.h>
#include <msp430.h>
#include <stdio.h>
#include "pins.h"
#include "protocol.h"
#include "radiocmds.h"
#include "peripheral.h"
#include "ISR.h"

/*********************************** Transmit Commands ***********************************/
unsigned char sync = 0x7E, source = 0x1;


// TODO Test this.
void send_packet(unsigned char dest, unsigned long clockData, unsigned char *info, unsigned char infoLength){
  unsigned int i, length = 0, FCS;
  unsigned char temp[64], cntrl;
  unsigned long clockSent;

  
  cntrl = infoLength;

  // Insert ADDR, and CNTRL
  temp[0] = (source << 4) | dest;
  temp[1] = cntrl;
  length = 2;

  // Insert TIMESTAMP
  clockSent = get_time_tick();
  for(i=0;i<4; i++){
    temp[length] = clockSent << (i*8); 
    length = length+1;
  }
  for(i=0;i<4; i++){
    temp[length] = clockData << (i*8);
    length = length+1;
  }

  // Insert INFO
  for(i=0;i<infoLength; i++){
    temp[length] = info[i];
    length = length + 1;
  }
  

  // Insert FCS
  insert_FCS(temp, &length);

  // Bitstuff
  bitstuff(temp, &length);
  
  // add sync, fill into packet
  TxBuffer[0] = sync;
  for(i=0;i<length;i++){
    TxBuffer[i+1] = temp[i];
  }
  TxBuffer_Len = length + 1;                                                // increment length for SYNC

  while(TxBuffer_Len<62){
    TxBuffer[TxBuffer_Len] = 0xaa;
    TxBuffer_Len = TxBuffer_Len + 1; 
  }

  // put the radio into transmit
  state = TX_START;
  radio_TX_state();
}

//TODO test this.
void insert_FCS(unsigned char *dat, unsigned int *datLength){
  unsigned int i,n;
  unsigned short SR=0xFFFF;
  unsigned short XORMask;
  char bytemask;

  // G=[0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 1]=0x1021 Generator polynomial
  // GFLIP=[1 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0]=0x8408 Flipped generator polynomial for XOR operation in loop
  const unsigned short GFLIP=0x8408;

  // Send the packet through a shift register, XOR the appropriate elements with the 
  for(n=0;n<*datLength;n++){       //loop through each char
    bytemask=0x80;                  //mask to select correct bit
    for(i=0;i<8;i++){               //loop through each bit in dat
      if((dat[n] & bytemask)!=0){
        if((SR & BIT0)!=0){
          SR=SR>>1;
          SR=SR^0x0000;
        }
        else{
          SR=SR>>1;
          SR=SR^GFLIP;
        }
      }
      else{
        if((SR & BIT0)!=0){
          SR=SR>>1;
          SR=SR^GFLIP;
        }
        else{
          SR=SR>>1;
          SR=SR^0x0000;
        }
      }       
      bytemask=bytemask>>1;
    }
  }
  
  SR = __bit_reverse_short(~SR);
  dat[*datLength] = SR>>8;
  dat[*datLength+1] = SR;
  *datLength = *datLength+2;
}

//TODO test this
void bitstuff(unsigned char *dat, unsigned int *len){
  unsigned int j,k;
  char m, n, ones;
  unsigned char datmask, scratchmask;

  // Search through packet bitwise (after flag) for consecutive 1's.  Need to stuff a 0 bit in packet AFTER the fifth consecutive 1.

  n=0;                    //counter for bits in byte
  k=0;                    //counter for bytes
  ones = 0;               //counter for consecutive ones
  scratchmask = 0x80;     //initialize scratchmask to MSB

  for(j=0;j<*len;j++){                                    // counter for dat bytes - search through entire packet
    datmask = 0x80;                                       // initialize datmask to MSB
    for(m=0;m<8;m++){                                     // shift through every bit in dat[j]
      if(ones == 5){                                      // if we had 5 ones
        dat[k] = dat[k] & (~scratchmask);                 // insert a 0
        if(n < 7){                                        // if not last bit in scratch[k]
          scratchmask = scratchmask>>1;                   // shift to next bit in scratch[k]
          n=n+1;                                          // increment bit counter for scratch[k]
        } 
        else {                                            // else we just processed last bit in scratch[k]
          scratchmask = 0x80;                             // reset scratchmask to point to MSB
          n=0;                                            // reset bit pointer for scratch[k]
          k = k+1;                                        // increment k
        }
        ones = 0;                                         // reset ones counter
      }

      if(((dat[j] & datmask) != 0) && (ones < 5)){        // if dat bit = 1, but its not the fifth
        dat[k] = dat[k] | scratchmask;                    // insert (keep) a 1
        if(n < 7){                                        // if not last bit in scratch[k]
          scratchmask = scratchmask>>1;                   // shift to next bit in scratch[k]
          n=n+1;                                          // increment bit counter for scratch[k]
        } 
        else {                                            // else we just processed last bit in scratch[k]
          scratchmask = 0x80;                             // reset scratchmask to point to MSB
          n=0;                                            // reset bit pointer for scratch[k]
          k = k+1;                                        // increment k
        }
        ones = ones+1;                                    // increment ones counter
      } 
      
      else {                                              // if dat bit = 0
        dat[k] = dat[k] & (~scratchmask);         // insert a 0
        ones = 0;                                         // reset ones counter
        if(n < 7){                                        // if not last bit in scratch[k]
          scratchmask = scratchmask>>1;                   // shift to next bit in scratch[k]
          n=n+1;                                          // increment bit counter for scratch[k]
        } 
        else {                                            // else we just processed last bit in scratch[k]
          scratchmask = 0x80;                             // reset scratchmask to point to MSB
          n=0;                                            // reset bit pointer for scratch[k]
          k = k+1;                                        // increment k
        }
      }

      datmask = datmask>>1;                               // shift to look at next bit
    }
  }

  *len = k;
}

/************************************* Receive Commands ********************************************/
unsigned int RX_SR = 0, dump, bit, RX_ST, remainingBytes = 0;
unsigned char RX_SR17 = 0, datmask, RXFLAG = 0, RXMASK = 0x80, RxBit = 0, ones = 0;

// Unscramble the RX buffer
// TODO Test this
void unscramble(unsigned char *indat, unsigned int inlen){

  unsigned int j, k;
  unsigned char SR12, newbit;

  for(k=0;k<inlen;k++){
    datmask = 0x80;

    for(j=0;j<8;j++){
      if((indat[k] & datmask) == 0){        // Grab the current bit in dat[k]
        bit = 0;
      } else {
        bit = 1;
      }
      if((RX_SR & 0x0800) == 0){            // Grab the twelfth bit in RX_SR
        SR12 = 0;
      } else {
        SR12 = 1;
      }
      newbit = bit ^ (SR12 ^ RX_SR17);      // Unscramble the bit
      if(newbit == 0){
        indat[k] = indat[k] & ~datmask;     // put back in the unscrambled bit
      } else {
        indat[k] = indat[k] | datmask;
      }
      if((RX_SR & 0x8000) == 0){            // set up RX_SR17 for next loop;
        RX_SR17 = 0;
      } else {
        RX_SR17 = 1;
      }
      
      datmask = datmask>>1;                 // shift to next bit in dat[k]
      
      RX_SR = RX_SR<<1;                     // shift left
      if(bit != 0){
        RX_SR = RX_SR | 0x0001;             // shift in bit (at LSB)
      } 
    }    
  }
}

// Reverse the transition encoding of the packet. MUST be called after unscramble.
// TODO Test this
void untransition(unsigned char *indat){

  unsigned int j, k, inlen = sizeof(indat);

  for(k=0;k<inlen;k++){
    datmask = 0x80;
    for(j=0;j<8;j++){
      if((indat[k] & datmask) == 0){
        bit = 0;
      } else {
        bit = 1;
      }
      if(RX_ST == bit){
        indat[k] = indat[k] | datmask;      // put in 1
      } else {
        indat[k] = indat[k] & ~datmask;     // put in 0
      }
      RX_ST = bit;
      datmask = datmask>>1;
    }  
  }
}

// Find the syncs within the RX buffer, and also unstuff the extra zeros!
// TODO Test this
void WSN_RX(unsigned char *indat, unsigned int inlen){
  unsigned char temp[64];
  unsigned int i,j,k,tempLength=0;

  sprintf(UARTBuff,"RX Interrupt Triggered\r\n");
  Send_UART(UARTBuff);

  for(k=0;k<inlen;k++){
  datmask = 0x80;
    for(j=0;j<8;j++){
      switch(RXFLAG)
      {
        case 0:
          RxBufferPos = 0;
          RXMASK = 0x80;
          RxBit = 0;
          dump = 0;
          ones = 0;
          if((indat[k] & datmask) == 0){
            RXFLAG = 1;
          } else {
            RXFLAG = 0;
          }
        break;
        case 1:                               // Found first 0.  Find next six 1's (case 1-6)
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            if((indat[k] & datmask) == 0){    // Bit = 0, still at first zero
              RXFLAG = 1;
            } else {
              RXFLAG = RXFLAG + 1;            // Bit = 1, counting six one's
            }
            break;
        case 7:                               // Found 0111 111, looking for last 0
            if((indat[k] & datmask) == 0){    // Bit = 0, FOUND FLAG BYTE
              RXFLAG = 8;
              sprintf(UARTBuff,"SYNC found!\r\n");
              Send_UART(UARTBuff);
            } else {                          // Bit = 1, Start over from beginning
              RXFLAG = 0;
            }
        break;
        case 8:                               // Found SYNC. Now check the address.
          // Fill first byte into RX Buffer
          if((indat[k] & datmask) != 0){                                //put in one
             RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] | RXMASK;
             ones = ones+1;
          } else {                                                      // put in zero
             RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] & ~RXMASK;
             ones = 0;
          }

          if(RxBit < 7){                                                // if not last bit in RxBuffer[k]
            RXMASK = RXMASK>>1;                                         // shift to next bit in RxBuffer[k]
            RxBit=RxBit+1;                                              // increment bit counter for RxBuffer[k]
          } else {                                                      // just processed last bit in RxBuffer[k]
             RXMASK = 0x80;                                             // reset outdatmask to point to MSB
             RxBit=0;                                                   // reset bit pointer for RxBuffer[k]
             if(RxBuffer[RxBufferPos] != sync){                         // If didn't receive another FLAG byte, save else dump byte
                RxBufferPos=RxBufferPos+1;                              // increment to next byte in RxBuffer
                ones = 0;
                RXFLAG = 9;
                sprintf(UARTBuff,"Address is 0x%x\r\n",RxBuffer[RxBufferPos]);
                Send_UART(UARTBuff);
            }
          }
        break;
        case 9:                                                         // Read Packet Length
          // Fill byte into RX Buffer
          if((indat[k] & datmask) != 0){                                // put in one
             RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] | RXMASK;
             ones = ones+1;
          } else {                                                      // put in zero
             RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] & ~RXMASK;
             ones = 0;
          }

          if(RxBit < 7){                                                // if not last bit in RxBuffer[k]
            RXMASK = RXMASK>>1;                                         // shift to next bit in RxBuffer[k]
            RxBit=RxBit+1;                                              // increment bit counter for RxBuffer[k]
          } else {                                                      // just processed last bit in RxBuffer[k]
            remainingBytes = (RxBuffer[RxBufferPos])+10;
            sprintf(UARTBuff,"total packet length is %i bytes\r\n", remainingBytes+2);
            Send_UART(UARTBuff);
            RXMASK = 0x80;                                              // reset outdatmask to point to MSB
            RxBit=0;                                                    // reset bit pointer for RxBuffer[k]
            RxBufferPos=RxBufferPos+1;                                  // increment to next byte in RxBuffer
            ones = 0;
            RXFLAG = 10;
          }
        break;
        case 10:                                                          // Pull into the RX buffer and unstuff
          if((indat[k] & datmask) != 0){                                  // put in one
            RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] | RXMASK;
            ones = ones+1;
            if(RxBit < 7){                                                // if not last bit in RxBuffer[k]
              RXMASK = RXMASK>>1;                                         // shift to next bit in RxBuffer[k]
              RxBit=RxBit+1;                                              // increment bit counter for RxBuffer[k]
            } else {                                                      // just processed last bit in RxBuffer[k]
              RXMASK = 0x80;                                              // reset RXMASK to point to MSB
              RxBit=0;                                                    // reset bit pointer for RxBuffer[k]
              if(remainingBytes != 0){                                    // If we haven't processed the whole packet
                RxBufferPos=RxBufferPos+1;                                // increment to next byte in RxBuffer
                remainingBytes = remainingBytes-1;  
              } else {                                                    // else we're done and can go check FCS
                sprintf(UARTBuff,"Decoded packet. INFO was:\r\n ----- Begin INFO -----\r\n");
                Send_UART(UARTBuff);
                RxBufferPos=RxBufferPos+1;  
                RXFLAG = 11;
                for(i=0; i<RxBuffer_Len; i++){
                  sprintf(UARTBuff,"0x%02x, ",RxBuffer[i]);
                  Send_UART(UARTBuff);
                }
                sprintf(UARTBuff,"\r\n ----- End INFO -----\r\n");
                Send_UART(UARTBuff);
              }
            }
          } else {                                                        // put in zero, or not if it was a bit stuff zero
            if(ones != 5) {                                               // If not five ones then save the zero and increment
              RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] & ~RXMASK;
              if(RxBit < 7){                                              // if not last bit in RxBuffer[k]
                RXMASK = RXMASK>>1;                                       // shift to next bit in RxBuffer[k]
                RxBit=RxBit+1;                                            // increment bit counter for RxBuffer[k]
              } else {                                                    // just processed last bit in RxBuffer[k]
                RXMASK = 0x80;                                            // reset RXMASK to point to MSB
                RxBit=0;                                                  // reset bit pointer for RxBuffer[k]
                if(remainingBytes != 0){                                  // If we haven't processed the whole packet yet
                  RxBufferPos=RxBufferPos+1;                              // increment to next byte in RxBuffer
                  remainingBytes = remainingBytes-1;
                } else {                                                  // else we're done and can go check FCS
                  sprintf(UARTBuff,"Decoded packet. INFO was:\r\n ----- Begin INFO -----\r\n");
                  Send_UART(UARTBuff);
                  RxBufferPos=RxBufferPos+1;  
                  RXFLAG = 11;
                  for(i=0; i<RxBuffer_Len; i++){
                    sprintf(UARTBuff,"0x%02x, ",RxBuffer[i]);
                    Send_UART(UARTBuff);
                  }
                  sprintf(UARTBuff,"\r\n ----- End INFO -----\r\n");
                  Send_UART(UARTBuff);
                }
              }
            } else {                                                      // If five ones then don't save the zero and don't increment
            }
            ones = 0;
          }
        break;
        case 11:                                                          // Check the FCS
          for(i=0; i < (RxBufferPos-2); i++){
            temp[i] = RxBuffer[i];
            tempLength = tempLength +1;
          }
          insert_FCS(temp, &tempLength);
          if(temp == RxBuffer){
            sprintf(UARTBuff,"FCS was correct, sucessful packet\r\n");
            Send_UART(UARTBuff);
          } else {
            sprintf(UARTBuff,"FCS was not correct, unsucessful packet\r\n");
            Send_UART(UARTBuff);
          }
          RxBuffer_Len = RxBufferPos;
          RXFLAG = 0;
          ones = 0;
          radio_flush();
        break;
      }
      datmask = datmask>>1;
    }
  }

}

  
