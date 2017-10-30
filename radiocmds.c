/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: radiocommands.c will contain all functions for the CC2500 radio                *
***********************************************************************************************/

#include <msp430f5438a.h>
#include <msp430.h>
#include "radiocmds.h"
#include "pins.h"

// TODO Wasn't TxBuffer already 
unsigned char TxBuffer[600], RxBuffer[600], RxTemp[30];
unsigned int TxBuffer_Len, TxBufferPos=0, TxBytesRemaining, RxBuffer_Len=0, RxBufferPos=0, RxBytesRemaining, state;

/******************************* Fundamental Radio Commands ***********************************/
// select CS lines for SPI
void radio_SPI_sel(void){        
     P3DIR &= ~CS_CC2500;
     P3OUT &= ~CS_CC2500;
}

// de-select CS lines for SPI
void radio_SPI_desel(void){     
     P3DIR |= CS_CC2500;
     P3OUT |= CS_CC2500;
}

// read a radio register 
char Radio_Read_Registers(char addr){
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
void Radio_Write_Registers(char addr, char value){
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
  P3DIR |= CS_CC2500;
  P3OUT |= CS_CC2500;                           // Toggle CS with delays to power up radio
  TI_CC_Wait(30);
  P3DIR &= ~CS_CC2500;
  P3OUT &= ~CS_CC2500;
  TI_CC_Wait(30);
  P3DIR |= CS_CC2500;
  P3OUT |= CS_CC2500;
  TI_CC_Wait(45);
  P3DIR &= ~CS_CC2500;
  P3OUT &= ~CS_CC2500;                          // CS enable
  while (!(UCB0IFG & UCTXIFG));                 // Wait for TXBUF ready
  UCB0TXBUF = TI_CCxxx0_SRES;                   // Send strobe
                                                // Strobe addr is now being TX'ed
  while (UCB0STAT & UCBUSY);                    // Wait for TX to complete
  P3DIR |= CS_CC2500;
  P3OUT |= CS_CC2500;                           // CS disable

  P1IFG = 0;                                    // Clear flags that were set 
}

// Function to wait a given number of cycles
void TI_CC_Wait(unsigned int cycles){
  while(cycles>15)                              // 15 cycles consumed by overhead
    cycles = cycles - 6;                        // 6 cycles consumed each iteration
}

/******************************************** Specialized Radio Commands **************************************************/

// Function to transmit a known dummy packet
void Send_Dummy(void){
  unsigned char DummyBuffer[18] = {0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E};
  unsigned int DummyBufferLength = sizeof(DummyBuffer);

  Radio_Write_Registers(TI_CCxxx0_PKTLEN, DummyBufferLength);                     // Set packet length
  Radio_Write_Registers(TI_CCxxx0_PKTCTRL0, 0x00);                                // Set to fixed byte mode
  Radio_Write_Burst_Registers(TI_CCxxx0_TXFIFO, DummyBuffer, DummyBufferLength);  // Write TX data

  Radio_Strobe(TI_CCxxx0_STX);                                                    // Set radio to transmit
}

//TODO add transmit commands 

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

Radio_Write_Registers(TI_CCxxx0_IOCFG2,   0x02);  // GDO2 output pin config. TX
Radio_Write_Registers(TI_CCxxx0_IOCFG0,   0x00);  // GDO0 output pin config. RX
Radio_Write_Registers(TI_CCxxx0_FIFOTHR,  0x07);  // FIFO Threshold: 21 byte in TX FIFO and 44 in RX FIFO

Radio_Write_Registers(TI_CCxxx0_PKTLEN,   0xFF); // Packet length.
Radio_Write_Registers(TI_CCxxx0_PKTCTRL1, 0x04); // Packet automation control.
Radio_Write_Registers(TI_CCxxx0_PKTCTRL0, 0x05); // Packet automation control.
Radio_Write_Registers(TI_CCxxx0_ADDR,     0x01); // Device address.
Radio_Write_Registers(TI_CCxxx0_CHANNR,   0x00); // Channel number.
Radio_Write_Registers(TI_CCxxx0_FSCTRL1,  0x08); // Freq synthesizer control.
Radio_Write_Registers(TI_CCxxx0_FSCTRL0,  0x00); // Freq synthesizer control.
Radio_Write_Registers(TI_CCxxx0_FREQ2,    0x5C); // Freq control word, high byte
Radio_Write_Registers(TI_CCxxx0_FREQ1,    0x4E); // Freq control word, mid byte.
Radio_Write_Registers(TI_CCxxx0_FREQ0,    0xC3); // Freq control word, low byte.
Radio_Write_Registers(TI_CCxxx0_MDMCFG4,  0x2B); // Modem configuration.
Radio_Write_Registers(TI_CCxxx0_MDMCFG3,  0xF8); // Modem configuration.
Radio_Write_Registers(TI_CCxxx0_MDMCFG2,  0x03); // Modem configuration. FSK
Radio_Write_Registers(TI_CCxxx0_MDMCFG1,  0x22); // Modem configuration.
Radio_Write_Registers(TI_CCxxx0_MDMCFG0,  0xF8); // Modem configuration.
Radio_Write_Registers(TI_CCxxx0_DEVIATN,  0x50); // Modem dev (when FSK mod en) for FSK(47.607 kHz Deviation)
Radio_Write_Registers(TI_CCxxx0_MCSM1 ,   0x30); // MainRadio Cntrl State Machine
Radio_Write_Registers(TI_CCxxx0_MCSM0 ,   0x18); // MainRadio Cntrl State Machine
Radio_Write_Registers(TI_CCxxx0_FOCCFG,   0x1D); // Freq Offset Compens. Config
Radio_Write_Registers(TI_CCxxx0_BSCFG,    0x1C); // Bit synchronization config.
Radio_Write_Registers(TI_CCxxx0_AGCCTRL2, 0x00); // AGC control.
Radio_Write_Registers(TI_CCxxx0_AGCCTRL1, 0x58); // AGC control.
Radio_Write_Registers(TI_CCxxx0_AGCCTRL0, 0x91); // AGC control.
Radio_Write_Registers(TI_CCxxx0_FREND1,   0x00); // Front end RX configuration.
Radio_Write_Registers(TI_CCxxx0_FREND0,   0x10); // Front end RX configuration.
Radio_Write_Registers(TI_CCxxx0_FSCAL3,   0xA9); // Frequency synthesizer cal.
Radio_Write_Registers(TI_CCxxx0_FSCAL2,   0x0A); // Frequency synthesizer cal.
Radio_Write_Registers(TI_CCxxx0_FSCAL1,   0x00); // Frequency synthesizer cal.
Radio_Write_Registers(TI_CCxxx0_FSCAL0,   0x11); // Frequency synthesizer cal.
Radio_Write_Registers(TI_CCxxx0_FSTEST,   0x59); // Frequency synthesizer cal.
Radio_Write_Registers(TI_CCxxx0_TEST2,    0x88); // Various test settings.
Radio_Write_Registers(TI_CCxxx0_TEST1,    0x31); // Various test settings.
Radio_Write_Registers(TI_CCxxx0_TEST0,    0x0B); // Various test settings.
}
