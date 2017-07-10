/*
  TWSWire.h - TWI HW slave and software master based TWI/I2C library for Arduino
  Copyright (c) 2017 j54n1n.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Based on TwoWire form Arduino https://github.com/arduino/Arduino.
*/
#if defined(TWSD) && !defined(__AVR_ATtiny1634__)
#include "TWSWire.h"
#include <string.h>

#if defined(__AVR_ATtiny841__) || defined(__AVR_ATtiny441__)
#define SCL_PIN 4
#define SCL_PORT PORTA
#define SDA_PIN 6
#define SDA_PORT PORTA
#elif defined(__AVR_ATtiny828__)
#define SCL_PIN 3
#define SCL_PORT PORTD
#define SDA_PIN 0
#define SDA_PORT PORTD
#else
#error No pins for software I2C defined
#endif
#define I2C_TIMEOUT 100
#include "SoftWire.h"
static SoftWire softWire;

#include "WireS.h"

// Constructors ////////////////////////////////////////////////////////////////

TWSWire::TWSWire() : slaveMode(false) {
}

// Public Methods //////////////////////////////////////////////////////////////

void TWSWire::begin(void) {
  softWire.begin();
  slaveMode = false;
}

void TWSWire::begin(uint8_t address) {
  TinyWireS.begin(address, 0);
  slaveMode = true;
}

void TWSWire::begin(int address) {
  begin((uint8_t)address);
}

void TWSWire::end(void) {
	// XXX: to be implemented.
}

void TWSWire::setClock(uint32_t clock) {
  // XXX: to be implemented.
  (void)clock; //disable warning
}

uint8_t TWSWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint32_t iaddress, uint8_t isize,
                             uint8_t sendStop) {
  if(!slaveMode) {
    return softWire.requestFrom(address, quantity, iaddress, isize, sendStop);
  }
  return 0;
}

uint8_t TWSWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0,
                     (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TWSWire::requestFrom(uint8_t address, uint8_t quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TWSWire::requestFrom(int address, int quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TWSWire::requestFrom(int address, int quantity, int sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TWSWire::beginTransmission(uint8_t address) {
  if(!slaveMode) {
    softWire.beginTransmission(address);
  }
}

void TWSWire::beginTransmission(int address) {
  beginTransmission((uint8_t)address);
}

uint8_t TWSWire::endTransmission(uint8_t sendStop) {
  if(!slaveMode) {
    return softWire.endTransmission(sendStop);
  }
  return 4; //other error
}

uint8_t TWSWire::endTransmission(void) {
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TWSWire::write(uint8_t data) {
  size_t numBytes = 0;
  if(!slaveMode) {
    numBytes = softWire.write(data);
  } else {
    numBytes = TinyWireS.write(data);
  }
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TWSWire::write(const uint8_t *data, size_t quantity) {
  size_t numBytes = 0;
  for (size_t i = 0; i < quantity; ++i){
    numBytes += write(data[i]);
  }
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TWSWire::write(const char *str) {
  if (str == NULL) return 0;
  return write((const uint8_t *)str, strlen(str));
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TWSWire::available(void) {
  int value = -1;
  if(!slaveMode) {
    value = softWire.available();
  } else {
    value = TinyWireS.available();
  }
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TWSWire::read(void) {
  int value = -1;
  if(!slaveMode) {
    value = softWire.read();
  } else {
    value = TinyWireS.read();
  }
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TWSWire::peek(void) {
  int value = -1;
  if(!slaveMode) {
    value = softWire.peek();
  } else {
    value = TinyWireS.peek();
  }
  return value;
}

void TWSWire::flush(void) {
  // XXX: to be implemented.
}

// sets function called on slave write
void TWSWire::onReceive( void (*function)(int) ) {
  TinyWireS.onReceive(function);
}

// sets function called on slave read
void TWSWire::onRequest( void (*function)(void) ) {
  TinyWireS.onRequest(function);
}

// Preinstantiate Objects //////////////////////////////////////////////////////

TWSWire Wire = TWSWire();
#endif // TWSD
