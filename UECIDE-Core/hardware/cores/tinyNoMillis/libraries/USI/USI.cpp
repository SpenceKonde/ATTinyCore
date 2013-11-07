#include "pins_arduino.h"
#include "USI.h"

USIClass USI;


void USIClass::begin(byte _SCLK, byte _MOSI, byte _MISO, byte _SS, byte _swapPins) {
  // Set direction register for SCK and MOSI pin.
  // MISO pin automatically overrides to INPUT.
  // When the SS pin is set as OUTPUT, it can be used as
  // a general purpose output port (it doesn't influence
  // USI operations).
  //7,9,8,6
  pinMode(_SCLK, OUTPUT);
  pinMode(_MOSI, OUTPUT);
  pinMode(_SS, OUTPUT);
  pinMode(_MISO,INPUT_PULLUP);
  
  if (_swapPins) {
    USIPP |= _BV(USIPOS);
  } else {
    USIPP &= ~_BV(USIPOS);
  }
  
  digitalWrite(_SCLK, LOW);
  digitalWrite(_MOSI, LOW);
  digitalWrite(_SS, HIGH);

  USICR = (_BV(USIWM0) | _BV(USICS1));
}

void USIClass::end() {
  USICR &= ~_BV(USIWM0);
}

void USIClass::setDataMode(uint8_t mode)
{
  if(mode == USI_MODE1) {
    USICR |= _BV(USICS0);
  } else {
    USICR &= ~_BV(USICS0);
  }
}
