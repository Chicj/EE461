/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: radiocommands.c will contain all functions for the CC2500 radio                *
***********************************************************************************************/

#include <msp430f5438a.h>
#include <msp430.h>
#include <stdio.h>
#include "radiocmds.h"
#include "pins.h"
#include "peripheral.h"
#include "protocol.h"

char status;
unsigned char TxBuffer[512], RxBuffer[70], RxTemp[40];
unsigned int TxBuffer_Len, TxBufferPos=0, TxBytesRemaining, RxBuffer_Len=0, RxBufferPos=0, RxBytesRemaining, state;


/******************************* Fundamental Radio Commands ***********************************/
// select CS lines for SPI
void radio_SPI_sel(void){       
     P3OUT &= ~CS_CC2500;
}

// de-select CS lines for SPI
void radio_SPI_desel(void){     
     P3OUT |= CS_CC2500;
}

// read a radio register 
char Radio_Read_Register(char addr){
  char x;
    
  radio_SPI_sel ();                             // set SPI CS
 
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = (addr | TI_CCxxx0_READ_SINGLE);   // Adding 0x80 to address tells the radio to read a single byte
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = 0;                                // Dummy write so we can read data
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  x = UCB0RXBUF;                                // Read data`

  radio_SPI_desel();                            // de-select SPI CS

  return x;
}

// Function to read a multiple bytes from the radio registers
void Radio_Read_Burst_Registers(char addr, unsigned char *buffer, int count){
  char i;

  radio_SPI_sel (); // set SPI CS

  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = (addr | TI_CCxxx0_READ_BURST);    // Adding 0xC0 to address tells the radio to read multiple bytes
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  UCB0TXBUF = 0;                                // Dummy write to read 1st data byte
                                                // Addr byte is now being TX'ed, with dummy byte to follow immediately after
  UCB0IFG &= ~UCRXIFG;                          // Clear flag
  while (!(UCB0IFG & UCRXIFG));                 // Wait for end of 1st data byte TX
                                                // First data byte now in RXBUF
  for (i = 0; i < (count-1); i++)
  {
    UCB0TXBUF = 0;                              //Initiate next data RX, meanwhile..
    buffer[i] = UCB0RXBUF;                      //NOTE Store data from last data RX... need to print buffer to be able to see this ! 
    while (!(UCB0IFG & UCRXIFG));               // Wait for RX to finish
  }
  
  buffer[count-1] = UCB0RXBUF;                  // Store last RX byte in buffer
  
  radio_SPI_desel();                            // de-select SPI CS
}

// Function to read the radio status
char Radio_Read_Status(char addr){
  char status;

  radio_SPI_sel (); // set SPI CS

  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = (addr | TI_CCxxx0_READ_BURST);    // Send address
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = 0;                                // Dummy write so we can read data
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  status = UCB0RXBUF;                           // Read data

  radio_SPI_desel();                            // de-select SPI CS

  return status;
}

//Function that sends single address to radio initiating a state or mode change 
//(e.g. sending addr 0x34 writes to SRX register initiating radio in RX mode
char Radio_Strobe(char strobe){
  char status;

  radio_SPI_sel ();                             // set SPI CS

  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = strobe;                           // Send strobe
                                                // Strobe addr is now being TX'ed
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  status = UCB0RXBUF;                           // Read data

  radio_SPI_desel();                            // de-select SPI CS
}

//Function to write a single byte to the radio registers
void Radio_Write_Register(char addr, char value){
  radio_SPI_sel ();                 // set SPI CS

  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = addr;                             // Send address
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = value;                            // Send data
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete

  radio_SPI_desel();                            // de-select SPI CS
}

// Function to write multiple bytes to the radio registers
void Radio_Write_Burst_Registers(char addr, unsigned char *buffer, int count){
  int i;

  radio_SPI_sel ();                             // set SPI CS
  
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = addr | TI_CCxxx0_WRITE_BURST;     // Adding 0x40 to address tells radio to perform burst write rather than single byte write

  for (i = 0; i < count; i++)
  {
    while (!(UCB0IFG & UCTXIFG));               // Wait for TXBUF ready
    UCB0TXBUF = buffer[i];                      // Send data
  }

  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  
  radio_SPI_desel();                            // de-select SPI CS
}

// Function to power cycle the radio
void Reset_Radio(void){
  P3OUT |= CS_CC2500;   //deselecting                        // Toggle CS with delays to power up radio
  TI_CC_Wait(30);
  P3OUT &= ~CS_CC2500;  //selecting
  TI_CC_Wait(30);
  P3OUT |= CS_CC2500;   //deselecting
  TI_CC_Wait(45);
  P3OUT &= ~CS_CC2500;  //selecting                        // CS enable
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = TI_CCxxx0_SRES;                   // Send strobe
                                                // Strobe addr is now being TX'ed
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  P3OUT |= CS_CC2500; //deselect                // CS disable

  P1IFG = 0;                                    // Clear flags that were set 
}

// Function to wait a given number of cycles
void TI_CC_Wait(unsigned int cycles){
  while(cycles>15)                              // 15 cycles consumed by overhead
    cycles = cycles - 6;                        // 6 cycles consumed each iteration
}

// Function to check and reset radio states
void radio_flush(void){
  // Check for TX Underflow
  if (Radio_Read_Status(TI_CCxxx0_MARCSTATE) == 0x16){
    Radio_Strobe(TI_CCxxx0_SFTX);
    Radio_Strobe(TI_CCxxx0_SRX);
    __delay_cycles(16000);
    sprintf(UARTBuff,"Underflow Error, TX FIFO flushed, radio state now: 0x%x \r\n",Radio_Read_Status(TI_CCxxx0_MARCSTATE));
    Send_UART(UARTBuff);
  }

  // Check for RX FIFO overflow
  if (Radio_Read_Status(TI_CCxxx0_MARCSTATE) == 0x11){
    Radio_Strobe(TI_CCxxx0_SFRX);
    Radio_Strobe(TI_CCxxx0_SRX);
    __delay_cycles(16000);
    sprintf(UARTBuff,"Overflow Error, RX FIFO flushed, radio state now: 0x%x \r\n",Radio_Read_Status(TI_CCxxx0_MARCSTATE));
    Send_UART(UARTBuff);
  }

  // Check for Idle
  if (Radio_Read_Status(TI_CCxxx0_MARCSTATE) == 0x01){
    Radio_Strobe(TI_CCxxx0_SRX);
    __delay_cycles(16000);
    sprintf(UARTBuff,"Radio went idle, reset to RX, radio state now: 0x%x \r\n",Radio_Read_Status(TI_CCxxx0_MARCSTATE));
    Send_UART(UARTBuff);
  }
}

// Function to deal with the various TX states
void radio_TX_state(void){                                                                      // Disable TX interrupt so we don't interrupt ourselves
  switch(state){
    case IDLE:
    break;
    case TX_START:
      TxBufferPos = 0;
      TxBytesRemaining = TxBuffer_Len;
      radio_flush();
      if(TxBytesRemaining > 64){
        if(TxBytesRemaining > 256){
          Radio_Write_Register(TI_CCxxx0_PKTLEN, (TxBuffer_Len % 256));                       // Pre-program the packet length
          Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x02);                                     // Infinite byte mode
        } else {
          Radio_Write_Register(TI_CCxxx0_PKTLEN, TxBuffer_Len);                               // Pre-program packet length
          Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x00);                                     // Fixed byte mode
        }
        TxBytesRemaining = TxBytesRemaining - 64;
        state = TX_RUNNING;
        Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, TxBuffer+TxBufferPos, 64);              // Write first 64 TX data bytes to TX FIFO
        TxBufferPos = TxBufferPos + 64;
      } else {
        TxBytesRemaining = 0;
        state = TX_END;
        Radio_Write_Register(TI_CCxxx0_PKTLEN, TxBuffer_Len);                                 // Pre-program packet length
        Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x00);                                       // Fixed byte mode
        Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, TxBuffer, TxBuffer_Len);    // Write TX data
        TxBufferPos = TxBufferPos+TxBuffer_Len;
      }
      Radio_Strobe(TI_CCxxx0_STX);                                                            // Set radio state to Tx
    break;
    case TX_RUNNING:
      if(TxBytesRemaining > TxThrBytes){
         TxBytesRemaining = TxBytesRemaining - TxThrBytes;
         state = TX_RUNNING;
         Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, TxBuffer+TxBufferPos, TxThrBytes);
         TxBufferPos += TxThrBytes;
      } else {
         TxBytesRemaining = 0;
         state = TX_END;
         Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x00);                                     // Enter fixed length mode to transmit the last of the bytes
         Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, TxBuffer+TxBufferPos, TxBytesRemaining);
         TxBufferPos += TxBytesRemaining;
      }
    break;
    case TX_END:
      while (Radio_Read_Status(TI_CCxxx0_MARCSTATE) == 0x13){                               // wait for end of transmission
         __delay_cycles(500);
      }

      Radio_Write_Register(TI_CCxxx0_PKTLEN,0xFF);                                          //Reset PKTLEN
      Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x02);                                       //Reset infinite packet length mode set
      sprintf(UARTBuff,"TX End\r\n");
      Send_UART(UARTBuff);
      radio_flush();
    break;
  }
}

/******************************************** Specialized Radio Commands **************************************************/
void Radio_Rx(void){
      // Triggered by GDO0 interrupt     
      // Entering here indicates that the RX FIFO is more than half filed.
      // Need to read RXThrBytes into RXBuffer then move RxBufferPos by RxThrBytes
      // Then wait until interrupt received again.
      //(char addr, unsigned char *buffer, int count, int radio_select)
      int i;
      Radio_Read_Burst_Registers(TI_CCxxx0_RXFIFO, RxTemp, RxThrBytes);
      //TODO ADD DECODE HERE 
      status = Radio_Read_Status(TI_CCxxx0_MARCSTATE);
      state=status&(~(BIT7|BIT6|BIT5));                   // get the state of the radio from the full status byte
      sprintf(UARTBuff,"Radio State: 0x%02x\r\n",state);
      Send_UART(UARTBuff);
      sprintf(UARTBuff,"Received Stuff was:\r\n-------begin packet-------\r\n",RxTemp);
      Send_UART(UARTBuff);
      for(i=0; i<sizeof(RxTemp); i++){
        sprintf(UARTBuff,"0x%02x, ",RxTemp[i]);
        Send_UART(UARTBuff);
      }
      sprintf(UARTBuff,"\r\n-----end packet-----\r\n");
      Send_UART(UARTBuff);
}

// Function to transmit a known dummy packet
void Send_Dummy(void){
  unsigned int i, DummyBufferLength;
  unsigned char DummyBuffer[64];

  for(i=0;i<61;i++){
  DummyBuffer[i] = 0xAA;
  }
  DummyBuffer[62] = 0x7E;
  DummyBuffer[63] = 0x7E;

  DummyBufferLength = sizeof(DummyBuffer);

  Radio_Write_Register(TI_CCxxx0_PKTLEN, DummyBufferLength);                     // Set packet length
  Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x00);                                // Set to fixed byte mode
  Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, DummyBuffer, DummyBufferLength);  // Write TX data

  Radio_Strobe(TI_CCxxx0_STX);                                                    // Set radio to transmit
}

//TODO for testing ? this is easy to see on a VNA
void radio_stream(char *argv){



}


/********************************************* Radio Settings ************************************************************/
void Write_RF_Settings(void)
{

/*
* Write CC2500 register settings
* baud : 38.4 kbs 
* Product = CC2500
* Crystal accuracy = 40 ppm
* X-tal frequency = 26 MHz
* RF output power = 1 dBm
* RX filterbandwidth = 540.000000 kHz
* Deviation = 38.085938 kHz
* Return state:  Return to RX state upon leaving either TX or RX
* Datarate = 2.39897 kBaud
* Modulation = (7) FSK
* Manchester enable = (0) Manchester disabled
* RF Frequency = 2433.000000 MHz
* Channel spacing = 199.950000 kHz
* Channel number = 0
* Optimization = Sensitivity
* Sync mode = (3) 30/32 sync word bits detected
* Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
* CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
* Forward Error Correction = (0) FEC disabled
* Length configuration = (1) Variable length packets, packet length configured by the first received byte after sync word.
* Packetlength = 255
* Preamble count = (2)  4 bytes
* Append status = 1
* Address check = (0) No address check
* FIFO autoflush = 0
* Device address = 0
* GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
* GDO2 signal selection = (11) Serial Clock
*/

Radio_Write_Register(TI_CCxxx0_IOCFG2,   0x02);  // GDO2 output pin config. TX
Radio_Write_Register(TI_CCxxx0_IOCFG0,   0x00);  // GDO0 output pin config. RX
Radio_Write_Register(TI_CCxxx0_FIFOTHR,  0x07);  // FIFO Threshold: 21 byte in TX FIFO and 44 in RX FIFO

Radio_Write_Register(TI_CCxxx0_PKTLEN,   0xFF); // Packet length.
Radio_Write_Register(TI_CCxxx0_PKTCTRL1, 0x04); // Packet automation control.
Radio_Write_Register(TI_CCxxx0_PKTCTRL0, 0x05); // Packet automation control.
Radio_Write_Register(TI_CCxxx0_ADDR,     0x01); // Device address.
Radio_Write_Register(TI_CCxxx0_CHANNR,   0x00); // Channel number.
Radio_Write_Register(TI_CCxxx0_FSCTRL1,  0x08); // Freq synthesizer control.
Radio_Write_Register(TI_CCxxx0_FSCTRL0,  0x00); // Freq synthesizer control.
Radio_Write_Register(TI_CCxxx0_FREQ2,    0x5C); // Freq control word, high byte
Radio_Write_Register(TI_CCxxx0_FREQ1,    0x4E); // Freq control word, mid byte.
Radio_Write_Register(TI_CCxxx0_FREQ0,    0xC3); // Freq control word, low byte.
Radio_Write_Register(TI_CCxxx0_MDMCFG4,  0x2B); // Modem configuration.
Radio_Write_Register(TI_CCxxx0_MDMCFG3,  0xF8); // Modem configuration.
Radio_Write_Register(TI_CCxxx0_MDMCFG2,  0x03); // Modem configuration. FSK
Radio_Write_Register(TI_CCxxx0_MDMCFG1,  0x22); // Modem configuration.
Radio_Write_Register(TI_CCxxx0_MDMCFG0,  0xF8); // Modem configuration.
Radio_Write_Register(TI_CCxxx0_DEVIATN,  0x50); // Modem dev (when FSK mod en) for FSK(47.607 kHz Deviation)
Radio_Write_Register(TI_CCxxx0_MCSM1 ,   0x30); // MainRadio Cntrl State Machine
Radio_Write_Register(TI_CCxxx0_MCSM0 ,   0x18); // MainRadio Cntrl State Machine
Radio_Write_Register(TI_CCxxx0_FOCCFG,   0x1D); // Freq Offset Compens. Config
Radio_Write_Register(TI_CCxxx0_BSCFG,    0x1C); // Bit synchronization config.
Radio_Write_Register(TI_CCxxx0_AGCCTRL2, 0x00); // AGC control.
Radio_Write_Register(TI_CCxxx0_AGCCTRL1, 0x58); // AGC control.
Radio_Write_Register(TI_CCxxx0_AGCCTRL0, 0x91); // AGC control.
Radio_Write_Register(TI_CCxxx0_FREND1,   0x00); // Front end RX configuration.
Radio_Write_Register(TI_CCxxx0_FREND0,   0x10); // Front end RX configuration.
Radio_Write_Register(TI_CCxxx0_FSCAL3,   0xA9); // Frequency synthesizer cal.
Radio_Write_Register(TI_CCxxx0_FSCAL2,   0x0A); // Frequency synthesizer cal.
Radio_Write_Register(TI_CCxxx0_FSCAL1,   0x00); // Frequency synthesizer cal.
Radio_Write_Register(TI_CCxxx0_FSCAL0,   0x11); // Frequency synthesizer cal.
Radio_Write_Register(TI_CCxxx0_FSTEST,   0x59); // Frequency synthesizer cal.
Radio_Write_Register(TI_CCxxx0_TEST2,    0x88); // Various test settings.
Radio_Write_Register(TI_CCxxx0_TEST1,    0x31); // Various test settings.
Radio_Write_Register(TI_CCxxx0_TEST0,    0x0B); // Various test settings.
}
