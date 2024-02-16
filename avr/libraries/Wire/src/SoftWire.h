/*
  SoftWire.h - A Wire compatible wrapper for SoftI2CMaster
  Copyright (c) 2016 Bernhard Nebel.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _SoftWire_h
#define _SoftWire_h

#include <SoftI2CMaster.h>
#include <inttypes.h>
#include "Stream.h"

#define BUFFER_LENGTH 32

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

class SoftWire : public Stream
{
private:
  uint8_t rxBuffer[BUFFER_LENGTH];
  uint8_t rxBufferIndex;
  uint8_t rxBufferLength;
  uint8_t transmitting;
  uint8_t error;
public:
  SoftWire(void) {
  }

  void begin(void) {
    rxBufferIndex = 0;
    rxBufferLength = 0;
    error = 0;
    transmitting = false;

    i2c_init();
  }

  void end(void) {
  }

  void setClock(uint32_t _clock) {
  }

  void beginTransmission(uint8_t address) {
    if (transmitting) {
      error = (i2c_rep_start((address<<1)|I2C_WRITE) ? 0 : 2);
    } else {
      error = (i2c_start((address<<1)|I2C_WRITE) ? 0 : 2);
    }
    // indicate that we are transmitting
    transmitting = 1;
  }

  void beginTransmission(int address) {
    beginTransmission((uint8_t)address);
  }

  uint8_t endTransmission(uint8_t sendStop) {
    uint8_t transError = error;
    if (sendStop) {
      i2c_stop();
      transmitting = 0;
    }
    error = 0;
    return transError;
  }

  //  This provides backwards compatibility with the original
  //  definition, and expected behaviour, of endTransmission
  //
  uint8_t endTransmission(void) {
    return endTransmission(true);
  }

  size_t write(uint8_t data) {
    if (i2c_write(data)) {
      return 1;
    } else {
      if (error == 0) error = 3;
      return 0;
    }
  }

  size_t write(const uint8_t *data, size_t quantity) {
    size_t trans = 0;
    for (size_t i = 0; i < quantity; ++i) {
      trans += write(data[i]);
    }
    return trans;
  }

  uint8_t requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop) {
    uint8_t localerror = 0;
    if (isize > 0) {
      // send internal address; this mode allows sending a repeated start to access
      // some devices' internal registers. This function is executed by the hardware
      // TWI module on other processors (for example Due's TWI_IADR and TWI_MMR registers)
      beginTransmission(address);
      // the maximum size of internal address is 3 bytes
      if (isize > 3) {
          isize = 3;
      }
      // write internal register address - most significant byte first
      while (isize-- > 0)
          write((uint8_t)(iaddress >> (isize*8)));
      endTransmission(false);
    }
    // clamp to buffer length
    if (quantity > BUFFER_LENGTH) {
      quantity = BUFFER_LENGTH;
    }
    localerror = !i2c_rep_start((address<<1) | I2C_READ);
    if (error == 0 && localerror) error = 2;
    // perform blocking read into buffer
    for (uint8_t cnt = 0; cnt < quantity; cnt++)
      rxBuffer[cnt] = i2c_read(cnt == quantity-1);
    // set rx buffer iterator vars
    rxBufferIndex = 0;
    rxBufferLength = quantity;
    if (sendStop) {
      transmitting = 0;
      i2c_stop();
    }
    return quantity;
  }

  uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0, (uint8_t)0, (uint8_t)sendStop);
  }

  uint8_t requestFrom(int address, int quantity, int sendStop) {
    return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
  }


  uint8_t requestFrom(uint8_t address, uint8_t quantity) {
    return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
  }

  uint8_t requestFrom(int address, int quantity) {
    return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
  }

  int available(void) {
    return rxBufferLength - rxBufferIndex;
  }

  int read(void) {
    int value = -1;
    if (rxBufferIndex < rxBufferLength) {
      value = rxBuffer[rxBufferIndex];
      ++rxBufferIndex;
    }
    return value;
  }

  int peek(void) {
    int value = -1;

    if (rxBufferIndex < rxBufferLength) {
      value = rxBuffer[rxBufferIndex];
    }
    return value;
  }

  void flush(void) {
  }

  inline size_t write(unsigned long n) { return write((uint8_t)n); }

  inline size_t write(long n) { return write((uint8_t)n); }

  inline size_t write(unsigned int n) { return write((uint8_t)n); }

  inline size_t write(int n) { return write((uint8_t)n); }

  using Print::write;
};


#endif
