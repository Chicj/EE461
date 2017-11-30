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
unsigned char sync = 0x7E;
unsigned char source = 0x1;

// TODO Test this.
void send_packet(unsigned char dest, unsigned long clockData, unsigned char *info, unsigned char infoLength){
  unsigned int i, length, FCS;
  char timeSent[4], timeData[4];
  unsigned char temp[63], packet[64], cntrl;
  unsigned long clockSent;

  
  cntrl = infoLength;

  // Insert ADDR, and CNTRL
  temp[0] = (source << 4) | dest;
  temp[1] = cntrl;
  length = 2;

  // Insert TIMESTAMP
  clockSent = get_time_tick();
  sprintf(timeSent, "%lu", clockSent);
  sprintf(timeData, "%lu", clockData);
  for(i=0;i<4; i++){
    temp[length] = clockSent << (i*8); 
    length++;
  }
  for(i=0;i<4; i++){
    temp[length] = clockData << (i*8);
    length++;
  }

  // Insert INFO
  for(i=0;i<infoLength; i++){
    temp[length] = info[i];
    length++;
  }

  // Insert FCS
  insert_FCS(temp, &length);

  // Bitstuff
  bitstuff(temp, &length);
  
  // add sync, fill into packet
  packet[0] = sync;
  for(i=0;i<sizeof(temp);++i){
    packet[i+1] = temp[i];
  }

  Radio_Write_Register(TI_CCxxx0_PKTLEN, sizeof(packet));                         // Set packet length
  Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x00);                                 // Set to fixed byte mode
  Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, packet, sizeof(packet));          // Write TX data

  Radio_Strobe(TI_CCxxx0_STX);                                                    // Set radio to transmit
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
unsigned int RX_SR = 0; dump, bit, RX_ST;
unsigned char RX_SR17 = 0, datmask, RXFLAG = 0, RXMASK = 0x80, RxBit = 0;

// Unscramble the RX buffer
// TODO Test this
void unscramble(unsigned char *indat){

  unsigned int j, k, inlen = sizeof(indat);
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
void find_sync(unsigned char *indat, unsigned int inlen){

  unsigned int i,j,k, ones = 0, remainingBytes;
  unsigned char firsteight;
  
  
  for(k=0;k<inlen;k++){
  datmask = 0x80;
    for(j=0;j<8;j++){
      switch(RXFLAG)
      {
        case 0:
          RxBufferPos = 0;
          RXMASK = 0x80;
          RxBit = 0;
          firsteight = 0;
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
          if(firsteight == 0){
            firsteight = 1;
          }

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
                firsteight=0;
                sprintf(UARTBuff,"Address is 0x%x\r\n",RxBuffer[RxBufferPos]);
                Send_UART(UARTBuff);
                // NOTE printing beginning of packet here
                for(i=0; i<5; i++){
                  sprintf(UARTBuff,"0x%02x, ",RxTemp[i]);
                  Send_UART(UARTBuff);
                }
            }
          }
        break;
        case 9:                                                         // Read Packet Length
          if(firsteight == 0){
            firsteight = 1;
          }

          // Fill byte into RX Buffer
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
            remainingBytes = (RxBuffer[RxBufferPos])+10;
            sprintf(UARTBuff,"CNTRL is 0x%x\r\n",RxBuffer[RxBufferPos]);
            Send_UART(UARTBuff);
            sprintf(UARTBuff,"packet length is %i bytes\r\n", remainingBytes);
            Send_UART(UARTBuff);
            RXMASK = 0x80;                                              // reset outdatmask to point to MSB
            RxBit=0;                                                    // reset bit pointer for RxBuffer[k]
            RxBufferPos=RxBufferPos+1;                                  // increment to next byte in RxBuffer
            ones = 0;
            RXFLAG = 10;
            firsteight=0;
          }
        break;
        case 10:                                                          // Pull the RX buffer and unstuff
          if(firsteight == 0) {
            firsteight = 1;
          }
          if((indat[k] & datmask) != 0){                                  //put in one
            RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] | RXMASK;
            ones = ones+1;
            if(RxBit < 7){                                                // if not last bit in RxBuffer[k]
              RXMASK = RXMASK>>1;                                         // shift to next bit in RxBuffer[k]
              RxBit=RxBit+1;                                              // increment bit counter for RxBuffer[k]
            } else {                                                      // just processed last bit in RxBuffer[k]
              RXMASK = 0x80;                                              // reset RXMASK to point to MSB
              RxBit=0;                                                    // reset bit pointer for RxBuffer[k]
              if(remainingBytes != 0){                          // If didn't receive another FLAG byte, save else dump byte
                RxBufferPos=RxBufferPos+1;                                // increment to next byte in RxBuffer
                remainingBytes = remainingBytes-1;
              } else {                                                    // Just received END FLAG BYTE
                RXFLAG = 11;
              }
            }
          } else {                                                        // put in zero, or not if it was a bit stuff zero
            if(ones != 5) {                                               // If not five ones then save the zero and increment
              RxBuffer[RxBufferPos] = RxBuffer[RxBufferPos] & ~RXMASK;
              if(RxBit < 7){                                                // if not last bit in RxBuffer[k]
                RXMASK = RXMASK>>1;                                         // shift to next bit in RxBuffer[k]
                RxBit=RxBit+1;                                              // increment bit counter for RxBuffer[k]
              } else {                                                      // just processed last bit in RxBuffer[k]
                RXMASK = 0x80;                                              // reset RXMASK to point to MSB
                RxBit=0;                                                    // reset bit pointer for RxBuffer[k]
                if(remainingBytes != 0){                          // If didn't receive another FLAG byte, save else dump byte
                  RxBufferPos=RxBufferPos+1;                                // increment to next byte in RxBuffer
                  remainingBytes = remainingBytes-1;
                } else {                                                    // Just received END FLAG BYTE
                  RXFLAG = 11;
                }
              }
            } else {                                                      // If five ones then don't save the zero and don't increment
            }
            ones = 0;
          }
        break;
        case 11: // TODO is there anything else that needs to be done here?
          RxBuffer_Len = RxBufferPos;
          RXFLAG = 0;
          ones = 0;
          for(i=0; i<RxBuffer_Len; i++){
            sprintf(UARTBuff,"0x%02x, ",RxBuffer[i]);
            Send_UART(UARTBuff);
          }
        break;
      }
      datmask = datmask>>1;
    }
  }

}

  
