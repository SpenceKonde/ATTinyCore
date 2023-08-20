/*
  Wire.h - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 2017~2019 by Spence Konde (spencekonde@gmail.com) to create the
  universal Wire library for ATTinyCore.

  This has caused some confusion - here's how the universal wire library works:
  The src directory (the one with this file in it) contains the files needed
  for THREE implementations of I2C. #ifdef's are used to select the
  appropriate implementation for the hardware:

  * ATtiny 88: Standard Wire implementation - it has a hardware
        I2C master/slave peripheral (twi.h/twi.cpp)
  * ATtiny 828, 441, 841: Software I2C master (no hardware I2C or USI
        on the chip), hardware I2C slave (WireS.h/WireS.cpp)
  * All others, including the 1634 - USI (USIWire.h/USIWire.cpp)

  Each of these may pull in additional files if required, refer to the
  source code for those files for more information.

  Regardless of which implementation is used, the class is called TwoWire,
  and the library provides an instance of it named Wire
*/

#include <Arduino.h>



#ifdef TWDR
  #if ((RAMEND + 1) - RAMSTART) > 128
    #define BUFFER_LENGTH 32
  #else
    #define BUFFER_LENGTH 16
  #endif

  #ifndef TwoWire_h
  #define TwoWire_h

  #include <inttypes.h>
  #include "Stream.h"

  // WIRE_HAS_END means Wire has end()
  #define WIRE_HAS_END 1

  class TwoWire : public Stream
  {
    private:
      static uint8_t rxBuffer[];
      static uint8_t rxBufferIndex;
      static uint8_t rxBufferLength;

      static uint8_t txAddress;
      static uint8_t txBuffer[];
      static uint8_t txBufferIndex;
      static uint8_t txBufferLength;

      static uint8_t transmitting;
      static void (*user_onRequest)(void);
      static void (*user_onReceive)(int);
      static void onRequestService(void);
      static void onReceiveService(uint8_t*, int);
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
      virtual size_t write(uint8_t);
      virtual size_t write(const uint8_t *, size_t);
      virtual int available(void);
      virtual int read(void);
      virtual int peek(void);
      virtual void flush(void);
      void onReceive( void (*)(int));
      void onRequest( void (*)(void));

      inline size_t write(unsigned long n) { return write((uint8_t)n); }
      inline size_t write(long n) { return write((uint8_t)n); }
      inline size_t write(unsigned int n) { return write((uint8_t)n); }
      inline size_t write(int n) { return write((uint8_t)n); }
      using Print::write;
  };

  extern TwoWire Wire;
  #endif
#elif defined(USIDR)
  #ifndef USIWire_h
  #define USIWire_h
  #include <inttypes.h>
  #include <stdio.h> // for size_t
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
      static uint8_t fastmode;
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
      void onReceive( void (*)(int));
      void onRequest( void (*)(void));
      uint8_t isActive(void);

      inline size_t write(unsigned long n) { return write((uint8_t)n); }
      inline size_t write(long n) { return write((uint8_t)n); }
      inline size_t write(unsigned int n) { return write((uint8_t)n); }
      inline size_t write(int n) { return write((uint8_t)n); }
  };
  extern TwoWire Wire;
  #endif
#elif defined(__AVR_ATtiny841__) || defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny828__)
  #ifndef TWSWire_h
  #define TWSWire_h
  #include <stdint.h>
  #include <stddef.h>

  // WIRE_HAS_END means Wire has end()
  #define WIRE_HAS_END 1

  class TwoWire {
  private:
    bool slaveMode;
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
    void onReceive(void(*)(int));
    void onReceive( void (*)(size_t));
    void onRequest(void(*)(void));

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
  };
  extern TwoWire Wire;
  #endif // TWSWire_h
#else
  #error No Wire support on unknown board
#endif
