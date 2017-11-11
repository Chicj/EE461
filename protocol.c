/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: protocols.c contains the functions for following the network protocol          *
***********************************************************************************************/
#include <msp430f5438a.h>
#include <msp430.h>
#include "pins.h"
#include "protocol.h"

/*********************************** Node Protocol Commands ***********************************/

// TODO Test this.
void send_packet(unsigned char dest, unsigned char cntrl, unsigned char *time, unsigned char *info){
  unsigned short i, length, FCS;
  unsigned char *temp, *packet;
  
  // Insert ADDR, and CNTRL
  temp[0] = (source << 4) | dest;
  temp[1] = cntrl;
  length = 2;

  // Insert TIMESTAMP
  for(i=0;i<8; i++){
    temp[length] = time[i];
    length++;
  }

  // Insert INFO
  for(i=0;i<sizeof(info); i++){
    temp[length] = info[i];
    length++;
  }

  // Insert FCS
  insert_FCS(temp);

  // Bitstuff
  bitstuff(temp);
  
  // add sync, fill into packet
  packet [0] = sync;
  for(i=0;i<sizeof(temp);i++){
  packet[i+1] = temp[i];
  }

}

//TODO test this.
void insert_FCS(unsigned char *dat){
  unsigned int i,n, len;
  unsigned short SR=0xFFFF;
  unsigned short XORMask;
  char bytemask;

  // G=[0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 1]=0x1021 Generator polynomial
  // GFLIP=[1 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0]=0x8408 Flipped generator polynomial for XOR operation in loop
  const unsigned short GFLIP=0x8408;

  // Send the packet through a shift register, XOR the appropriate elements with the 
  for(n=0;n<sizeof(dat);n++){       //loop through each char
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
  
  len = sizeof(dat);
  SR = __bit_reverse_short(~SR);
  dat[len] = SR>>8;
  dat[len+1] = SR;
}

//TODO test this
void bitstuff(unsigned char *dat){
  unsigned char *scratch;
  unsigned int j,k, len;
  char m, n, ones;
  unsigned char datmask, scratchmask;

  len = sizeof(dat);

  // Search through packet bitwise (after flag) for consecutive 1's.  Need to stuff a 0 bit in packet AFTER the fifth consecutive 1.

  n=0;                    //counter for bits in byte
  k=0;                    //counter for bytes
  ones = 0;               //counter for consecutive ones
  scratchmask = 0x80;     //initialize scratchmask to MSB

  for(j=0;j<len;j++){                                    // counter for dat bytes - search through entire packet
    datmask = 0x80;                                       // initialize datmask to MSB
    for(m=0;m<8;m++){                                     // shift through every bit in dat[j]
      if(ones == 5){                                      // if we had 5 ones, insert a 0(i.e. do nothing but increment)
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
        scratch[k] = scratch[k] | (scratchmask & 1);      // insert a 1
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
      
      else {                                              // if dat bit = 0, insert a 0(i.e. do nothing but increment)
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

  // fill back into dat
  for(j=0;j<sizeof(scratch);j++){
    dat[j]=scratch[j];
  }


}