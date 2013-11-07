#ifndef _USI_H_INCLUDED
#define _USI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

#define USI_MODE0 0
#define USI_MODE1 1

class USIClass {
public:
  inline static byte transfer(byte _data);

  // USI Configuration methods

  inline static void attachInterrupt();
  inline static void detachInterrupt(); // Default

  static void begin(byte _SCLK, byte _MOSI, byte _MISO, byte _SS, byte _swapPins = 1); // Default
  static void end();

  static void setDataMode(uint8_t);
};

extern USIClass USI;


byte USIClass::transfer(byte _data) {
  USIDR = _data;
  USISR = _BV(USIOIF); // clear flag
  
  /*while ( (USISR & _BV(USIOIF)) == 0 ) {
   USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  }*/
  byte oldSREG = SREG;
  cli(); //disable interrupts during data transfer
  
  //each instruction toggles the USCK value and transferes the data in three-wire (SPI) mode
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  
  //after 16 instuctions the flag will set and the byte will be transfered
    
  SREG = oldSREG; //restore SREG to previous value
  
  return USIDR;
}

void USIClass::attachInterrupt() {
  USICR |= _BV(USIOIE);
}

void USIClass::detachInterrupt() {
  USICR &= ~_BV(USIOIE);
}

#endif
