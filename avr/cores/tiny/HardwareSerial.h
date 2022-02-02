/*
  HardwareSerial.h - Hardware serial library for Wiring
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

*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(LINBRRH)) && !USE_SOFTWARE_SERIAL

  #include <inttypes.h>

  #include "Stream.h"
  #if (RAMEND < 1000)
    #define SERIAL_BUFFER_SIZE 16
  #else
    #define SERIAL_BUFFER_SIZE 64
  #endif
  /* WARNING
   * BUFFER SIZES MUST BE POWERS OF TWO - The compiler misses some of the
   * optimization possible with the % opperator. only a small portion of it, but as there's no compelling reason NOT
   * to use a power of two size, and there are some extrenely flash-constrained parts that have a USART (I'm thinking of the 2313 in particular
   * I couldn't justify not explicitly optimizing the % SERIAL_BUFFER_SIZE to a & (SERIAL_BUFFER_SIZE -1))
   */

  struct ring_buffer;

  struct ring_buffer
    {
      unsigned char buffer[SERIAL_BUFFER_SIZE];
      byte head;
      byte tail;
    };
  inline void store_char(unsigned char c, ring_buffer *buffer) {
    byte i = (buffer->head + 1) % SERIAL_BUFFER_SIZE;

    // if we should be storing the received character into the location
    // just before the tail (meaning that the head would advance to the
    // current location of the tail), we're about to overflow the buffer
    // and so we don't write the character or advance the head.
    if (i != buffer->tail) {
      buffer->buffer[buffer->head] = c;
      buffer->head = i;
    }
  }
  class HardwareSerial : public Stream
  {
    private:
      volatile ring_buffer *_rx_buffer;
      volatile ring_buffer *_tx_buffer;
      volatile uint8_t *_ubrrh;
      volatile uint8_t *_ubrrl;
      volatile uint8_t *_ucsra;
      volatile uint8_t *_ucsrb;
      volatile uint8_t *_udr;
      uint8_t _rxen;
      uint8_t _txen;
      uint8_t _rxcie;
      uint8_t _udrie;
      uint8_t _u2x;
    public:
      HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer
      #if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
        ,
        volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
        volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
        volatile uint8_t *udr,
        uint8_t BV_rxen, uint8_t BV_txen, uint8_t BV_rxcie, uint8_t BV_udrie, uint8_t BV_u2x);
      #else
        );
      #endif
      void begin(long);
      void end();
          // Basic printHex() forms for 8, 16, and 32-bit values
      void                printHex(const     uint8_t              b);
      void                printHex(const    uint16_t  w, bool s = 0);
      void                printHex(const    uint32_t  l, bool s = 0);
      // printHex(signed) and printHexln() - trivial implementation;
      void                printHex(const      int8_t  b)              {printHex((uint8_t )   b);           }
      void                printHex(const        char  b)              {printHex((uint8_t )   b);           }
      void              printHexln(const      int8_t  b)              {printHex((uint8_t )   b); println();}
      void              printHexln(const        char  b)              {printHex((uint8_t )   b); println();}
      void              printHexln(const     uint8_t  b)              {printHex(             b); println();}
      void              printHexln(const    uint16_t  w, bool s = 0)  {printHex(          w, s); println();}
      void              printHexln(const    uint32_t  l, bool s = 0)  {printHex(          l, s); println();}
      void              printHexln(const     int16_t  w, bool s = 0)  {printHex((uint16_t)w, s); println();}
      void              printHexln(const     int32_t  l, bool s = 0)  {printHex((uint16_t)l, s); println();}
      // The pointer-versions for mass printing uint8_t and uint16_t arrays.
      uint8_t *           printHex(          uint8_t* p, uint8_t len, char sep = 0            );
      uint16_t *          printHex(         uint16_t* p, uint8_t len, char sep = 0, bool s = 0);
      virtual int available(void);
      virtual int peek(void);
      virtual int read(void);
      virtual void flush(void);
      virtual size_t write(uint8_t);
      using Print::write; // pull in write(str) and write(buf, size) from Print
      operator bool();
  };

  #endif

  #if (defined(UBRRH) || defined(UBRR0H) || defined(LINBRRH)) && !USE_SOFTWARE_SERIAL
    extern HardwareSerial Serial;
  #endif
  #if defined(UBRR1H)
    extern HardwareSerial Serial1;
  #endif

#endif
