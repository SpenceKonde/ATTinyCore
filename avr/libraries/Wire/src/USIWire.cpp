/*
  USIWire.cpp - USI based TWI/I2C library for Arduino
  Copyright (c) 2017 Puuu.  All right reserved.

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
#ifdef USIDR
extern "C" {
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "USI_TWI_Slave/USI_TWI_Slave.h"
#include "USI_TWI_Master/USI_TWI_Master.h"
}

#include "USIWire.h"

const uint8_t WIRE_BUFFER_LENGTH = TWI_BUFFER_SIZE - 1; //reserve slave addr

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t *TwoWire::Buffer = TWI_Buffer;
uint8_t TwoWire::BufferIndex = 0;
uint8_t TwoWire::BufferLength = 0;

uint8_t TwoWire::transmitting = 0;

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire() {
}

// Public Methods //////////////////////////////////////////////////////////////

void TwoWire::begin(void) {
  BufferIndex = 0;
  BufferLength = 0;

  transmitting = 0;

  USI_TWI_Master_Initialise();
}

void TwoWire::begin(uint8_t address) {
  BufferIndex = 0;
  BufferLength = 0;

  transmitting = 0;
  USI_TWI_Slave_Initialise(address);
}

void TwoWire::begin(int address) {
  begin((uint8_t)address);
}

void TwoWire::end(void) {
  USI_TWI_Slave_Disable();
  DDR_USI_CL &= ~(1 << PIN_USI_SCL); // Enable SCL as input.
  DDR_USI &= ~(1 << PIN_USI_SDA); // Enable SDA as input.
  PORT_USI &= ~(1 << PIN_USI_SDA); // Disable pullup on SDA.
  PORT_USI_CL &= ~(1 << PIN_USI_SCL); // Disable pullup on SCL.
}

void TwoWire::setClock(uint32_t clock) {
  // XXX: to be implemented.
  (void)clock; //disable warning
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint32_t iaddress, uint8_t isize,
                             uint8_t sendStop) {
  if (isize > 0) {
    // send internal address; this mode allows sending a repeated
    // start to access some devices' internal registers. This function
    // is executed by the hardware TWI module on other processors (for
    // example Due's TWI_IADR and TWI_MMR registers)

    beginTransmission(address);

    // the maximum size of internal address is 3 bytes
    if (isize > 3) {
      isize = 3;
    }

    // write internal register address - most significant byte first
    while (isize-- > 0) {
      write((uint8_t)(iaddress >> (isize*8)));
    }
    endTransmission(false);
  }

  // reserve one byte for slave address
  quantity++;
  // clamp to buffer length
  if (quantity > TWI_BUFFER_SIZE) {
    quantity = TWI_BUFFER_SIZE;
  }
  // set address of targeted slave and read mode
  Buffer[0] = (address << TWI_ADR_BITS) | (1 << TWI_READ_BIT);
  // perform blocking read into buffer
  uint8_t ret = USI_TWI_Start_Transceiver_With_Data_Stop(Buffer, quantity,
                                                         sendStop);
  // set rx buffer iterator vars
  BufferIndex = 1; // ignore slave address
  // check for error
  if (ret == FALSE) {
    BufferLength = BufferIndex;
    return 0;
  }
  BufferLength = quantity;

  return quantity - 1; // ignore slave address
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0,
                     (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address) {
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave and write mode
  Buffer[0] = (address << TWI_ADR_BITS) | (0 << TWI_READ_BIT);
  // reset tx buffer iterator vars
  BufferIndex = 1; // reserved by slave address
  BufferLength = BufferIndex;
}

void TwoWire::beginTransmission(int address) {
  beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(uint8_t sendStop) {
  // transmit buffer (blocking)
  uint8_t ret = USI_TWI_Start_Transceiver_With_Data_Stop(Buffer,
                                                         BufferLength,
                                                         sendStop);
  // reset tx buffer iterator vars
  BufferIndex = 0;
  BufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  // check for error
  if (ret == FALSE) {
    switch (USI_TWI_Get_State_Info()) {
    case USI_TWI_DATA_OUT_OF_BOUND:
      return 1; //data too long to fit in transmit buffer
    case USI_TWI_NO_ACK_ON_ADDRESS:
      return 2; //received NACK on transmit of address
    case USI_TWI_NO_ACK_ON_DATA:
      return 3; //received NACK on transmit of data
    }
    return 4; //other error
  }
  return 0; //success
}

uint8_t TwoWire::endTransmission(void) {
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data) {
  if (transmitting) { // in master transmitter mode
    // don't bother if buffer is full
    if (BufferLength >= TWI_BUFFER_SIZE) {
      return 0;
    }
    // put byte in tx buffer
    Buffer[BufferIndex] = data;
    ++BufferIndex;
    // update amount in buffer
    BufferLength = BufferIndex;
  } else { // in slave send mode
    // don't bother if buffer is full
    if (!USI_TWI_Space_In_Transmission_Buffer()) {
      return 0;
    }
    // reply to master
    USI_TWI_Transmit_Byte(data);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity) {
  size_t numBytes = 0;
  for (size_t i = 0; i < quantity; ++i){
    numBytes += write(data[i]);
  }
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const char *str) {
  if (str == NULL) return 0;
  return write((const uint8_t *)str, strlen(str));
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  if (BufferLength) {
    return BufferLength - BufferIndex;
  } else {
    return USI_TWI_Data_In_Receive_Buffer();
  }
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  int value = -1;

  // get each successive byte on each call
  if (available()) {
    if (BufferLength) {
      value = Buffer[BufferIndex];
      ++BufferIndex;
    } else {
      value = USI_TWI_Receive_Byte();
    }
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void) {
  int value = -1;

  if (available()) {
    if (BufferLength) {
      value = Buffer[BufferIndex];
    } else {
      value = USI_TWI_Peek_Receive_Byte();
    }
  }

  return value;
}

void TwoWire::flush(void) {
  // XXX: to be implemented.
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) ) {
  USI_TWI_On_Slave_Receive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) ) {
  USI_TWI_On_Slave_Transmit = function;
}

// return true on I2C/TWI activity
uint8_t TwoWire::isActive(void) {
  return USI_TWI_Slave_Is_Active();
}

// Preinstantiate Objects //////////////////////////////////////////////////////

TwoWire Wire = TwoWire();
#endif
