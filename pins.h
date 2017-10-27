// This will be the file to define existing pins on the dev. board

#ifndef _PINS_H
#define _PINS_H

//TODO Change Clock Definitions
#define CLOCK_DIR         P1DIR
#define CLOCK_SEL0        P1SEL0
#define CLOCK_SEL1        P1SEL1

// UCA2 Pins for radio
#define UART_TX_PIN       BIT4
#define UART_RX_PIN       BIT5
#define UART_TX_PIN_NUM   4                           // P9.4
#define UART_RX_PIN_NUM   5                           // P9.5
#define UART_PINS         (UART_RX_PIN|UART_TX_PIN)
#define UART_PORT         9                           // P9

// UCBO P3 Serial pins for radio SPI 
#define RADIO_PIN_SIMO BIT1 
#define RADIO_PIN_SOMI BIT2 
#define RADIO_PIN_ClK  BIT3

//P5.5 this is the chip select for the SPI bus
#define CS_CC2500      BIT0   //P3.0

#define RADIO_PINS_SPI (RADIO_PIN_SOMI | RADIO_PIN_SIMO | RADIO_PIN_ClK)

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