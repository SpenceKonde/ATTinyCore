/*
  USIWire.h - USI based TWI/I2C library for Arduino
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
#ifndef USIWire_h
#define USIWire_h

#ifdef USIDR
#include <inttypes.h>

// Buffer sizes are defined in USI_TWI_Slave/USI_TWI_Slave.h
extern const uint8_t WIRE_BUFFER_LENGTH;
#define BUFFER_LENGTH (WIRE_BUFFER_LENGTH)

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

class TwoWire {
  private:
    static uint8_t *Buffer;
    static uint8_t BufferIndex;
    static uint8_t BufferLength;

    static uint8_t transmitting;
  public:
    TwoWire();
    void begin();
    void begin(uint8_t);
    void begin(int);
    void end();
    void setClock(uint32_t);
    void beginTransmission(uint8_t);
    void beginTransmission(int);
    uint8_t endTransmission(void);
    uint8_t endTransmission(uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint32_t, uint8_t, uint8_t);
    uint8_t requestFrom(int, int);
    uint8_t requestFrom(int, int, int);
    size_t write(uint8_t);
    size_t write(const uint8_t *, size_t);
    size_t write(const char *);
    int available(void);
    int read(void);
    int peek(void);
    void flush(void);
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );
    uint8_t isActive(void);

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
};

extern TwoWire Wire;

#endif
#endif
