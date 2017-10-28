/**********************************************************************************************
* EE 641 Project - Wireless Sensor Network                                                    *
* Authors: Chic O'Dell, Justin Long, Rowshon Munny                                            *
*                                                                                             *
* Description: pins.h is the header file to set all pins on the microcontroller and radio     *
***********************************************************************************************/

#ifndef _PINS_H
#define _PINS_H

//TODO Change Clock Definitions
#define CLOCK_DIR         P1DIR
#define CLOCK_SEL0        P1SEL0
#define CLOCK_SEL1        P1SEL1

//NOTE UCAx Pins for radio change this when munny makes UART driver 
#define UART_TX_PIN       BIT4
#define UART_RX_PIN       BIT5
#define UART_TX_PIN_NUM   4                           // TX = P9.4
#define UART_RX_PIN_NUM   5                           // RX = P9.5
#define UART_PINS         (UART_RX_PIN|UART_TX_PIN)
#define UART_PORT         9                           // P9

// UCB0 P3 Serial pins for radio SPI 
#define RADIO_PIN_SIMO BIT1                           // SIMO = P3.1
#define RADIO_PIN_SOMI BIT2                           // SOMI = P3.2
#define RADIO_PIN_ClK  BIT3                           // CLK = P3.3
#define CS_CC2500      BIT0                           // CS = P3.0
#define RADIO_PINS_SPI (RADIO_PIN_SOMI | RADIO_PIN_SIMO | RADIO_PIN_ClK)

// Radio interrupt pins
#define CC2500_GDO0       BIT7                        // GDO0 = P1.7 (RX)  
#define CC2500_GDO2       BIT3                        // GDO2 = P1.3 (TX)

#endif

/*
LOOK I DREW A THING ! (its me running away from code...)
        \                           /
         \                         /
          \                       /
           ]                     [    ,'|
           ]                     [   /  |
           ]___               ___[ ,'   |
           ]  ]\             /[  [ |:   |
           ]  ] \           / [  [ |:   |
           ]  ]  ]         [  [  [ |:   |
           ]  ]  ]__     __[  [  [ |:   |
           ]  ]  ] ]\ _ /[ [  [  [ |:   |
           ]  ]  ] ] (#) [ [  [  [ :===='
           ]  ]  ]_].nHn.[_[  [  [
           ]  ]  ]  HHHHH. [  [  [
           ]  ] /   `HH("N  \ [  [
           ]__]/     HHH  "  \[__[
           ]         NNN         [
           ]         N/"         [
           ]         N H         [
          /          N            \
         /           q,            \
        /                           \
*/