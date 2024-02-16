/*
  HardwareSerial.cpp - Hardware serial library for Wiring
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

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "Arduino.h"
#include "wiring_private.h"

// this next line disables the entire HardwareSerial.cpp,
// this is so I can support Attiny series and any other chip without a uart
// (If DISABLE_UART is set, HW serial is disabled, skipping this file and freeing up memory.)
#if ((defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(LINBRRH)) && !(USE_SOFTWARE_SERIAL && (!defined(UBRR1H) || DISABLE_UART1)) && !DISABLE_UART && !(DISABLE_UART0 && (DISABLE_UART1 || !defined(UBRR1H ))))

  #include "HardwareSerial.h"
  //from here on out we rely on the normalized register names!


  // Constructors ////////////////////////////////////////////////////////////////

  HardwareSerial::HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer
  #if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
    ,volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
    volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
    volatile uint8_t *udr) {
    _rx_buffer = rx_buffer;
    _tx_buffer = tx_buffer;
    _ubrrh = ubrrh;
    _ubrrl = ubrrl;
    _ucsra = ucsra;
    _ucsrb = ucsrb;
    _udr = udr;
  }
  #else
  ) {
    _rx_buffer = rx_buffer;
    _tx_buffer = tx_buffer;
  }
  #endif


  // Public Methods //////////////////////////////////////////////////////////////

  void HardwareSerial::begin(long baud) {
  #if (defined(UBRR0H) || defined(UBRR1H))
    uint16_t baud_setting;
    bool use_u2x = true;
  /*
  #if F_CPU == 16000000UL
    // hardcoded exception for compatibility with the bootloader shipped
    // with the Duemilanove and previous boards and the firmware on the 8U2
    // on the Uno and Mega 2560.
    if (baud == 57600) {
      use_u2x = false;
    }
  #endif
  */

  try_again:

    if (use_u2x) {
      *_ucsra = _u2x;
      baud_setting = (F_CPU / 4 / baud - 1) / 2;
    } else {
      baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }

    if ((baud_setting > 4095) && use_u2x) {
      use_u2x = false;
      goto try_again;
    }
    // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
    *_ubrrh = baud_setting >> 8;
    *_ubrrl = baud_setting;
    *_ucsrb = (_rxen | _txen | _rxcie);

  #else
    LINCR = (1 << LSWRES);
    LINBRR = (((F_CPU * 10L / 16L / baud) + 5L) / 10L) - 1;
    LINBTR = (1 << LDISR) | (16 << LBT0);
    LINCR = _BV(LENA) | _BV(LCMD2) | _BV(LCMD1) | _BV(LCMD0);
    LINENIR =_BV(LENRXOK);
  #endif
    // LINENIR only ever set to 0, 1, 2, or 3. Can we use our knowledge of the state if should be in to out advantage?

  }

  void HardwareSerial::end() {
    while (_tx_buffer->head != _tx_buffer->tail) {
      ; // wait for buffer to end.
    }
    #if (defined(UBRR0H) || defined(UBRR1H))
     /*
      cbi(*_ucsrb, _rxen);
      cbi(*_ucsrb, _txen);
      cbi(*_ucsrb, _rxcie);
      cbi(*_ucsrb, _udrie);
     */
      *_ucsrb = 0; //
    #else
      /*
      cbi(LINENIR,LENTXOK);
      cbi(LINENIR,LENRXOK);
      cbi(LINCR,LENA);
      cbi(LINCR,LCMD0);
      cbi(LINCR,LCMD1);
      cbi(LINCR,LCMD2);
      */
      LINENIR = 0;
      LINCR = 0;


    #endif
    _rx_buffer->head = _rx_buffer->tail;
  }

  int HardwareSerial::available(void) {
    return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) & (SERIAL_BUFFER_SIZE - 1);
  }

  int HardwareSerial::peek(void) {
    if (_rx_buffer->head == _rx_buffer->tail) {
      return -1;
    } else {
      return _rx_buffer->buffer[_rx_buffer->tail];
    }
  }

  int HardwareSerial::read(void) {
    // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buffer->head == _rx_buffer->tail) {
      return -1;
    } else {
      unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
      _rx_buffer->tail = (_rx_buffer->tail + 1)  & (SERIAL_BUFFER_SIZE - 1);
      return c;
    }
  }

  void HardwareSerial::flush() {
    while (_tx_buffer->head != _tx_buffer->tail)
      ;
  }

  size_t HardwareSerial::write(uint8_t c) {
    byte i = (_tx_buffer->head + 1)  & (SERIAL_BUFFER_SIZE - 1);

    // If the output buffer is full, there's nothing for it other than to
    // wait for the interrupt handler to empty it a bit
    // ???: return 0 here instead?
    while (i == _tx_buffer->tail)
      ;

    _tx_buffer->buffer[_tx_buffer->head] = c;
    _tx_buffer->head = i;

    #if (defined(UBRR0H) || defined(UBRR1H) )
      *_ucsrb |= _udrie;
    #else
      if(!(LINENIR & _BV(LENTXOK))){
        // The buffer was previously empty, so load the first byte, then enable 
        // the TX Complete interrupt
        unsigned char c = _tx_buffer->buffer[_tx_buffer->tail];
        _tx_buffer->tail = (_tx_buffer->tail + 1) & (SERIAL_BUFFER_SIZE - 1);
        LINDAT = c;
        LINENIR = _BV(LENTXOK) | _BV(LENRXOK);
      }
    #endif


    return 1;
  }

  HardwareSerial::operator bool() {
    return true;
  }

  void HardwareSerial::printHex(const uint8_t b) {
    char x = (b >> 4) | '0';
    if (x > '9')
      x += 7;
    write(x);
    x = (b & 0x0F) | '0';
    if (x > '9')
      x += 7;
    write(x);
  }

  void HardwareSerial::printHex(const uint16_t w, bool swaporder) {
    uint8_t *ptr = (uint8_t *) &w;
    if (swaporder) {
      printHex(*(ptr++));
      printHex(*(ptr));
    } else {
      printHex(*(ptr + 1));
      printHex(*(ptr));
    }
  }

  void HardwareSerial::printHex(const uint32_t l, bool swaporder) {
    uint8_t *ptr = (uint8_t *) &l;
    if (swaporder) {
      printHex(*(ptr++));
      printHex(*(ptr++));
      printHex(*(ptr++));
      printHex(*(ptr));
    } else {
      ptr+=3;
      printHex(*(ptr--));
      printHex(*(ptr--));
      printHex(*(ptr--));
      printHex(*(ptr));
    }
  }

  uint8_t * HardwareSerial::printHex(uint8_t* p, uint8_t len, char sep) {
    for (byte i = 0; i < len; i++) {
      if (sep && i) write(sep);
      printHex(*p++);
    }
    println();
    return p;
  }

  uint16_t * HardwareSerial::printHex(uint16_t* p, uint8_t len, char sep, bool swaporder) {
    for (byte i = 0; i < len; i++) {
      if (sep && i) write(sep);
      printHex(*p++, swaporder);
    }
    println();
    return p;
  }
  // Preinstantiate Objects //////////////////////////////////////////////////////

// Moved to separate files to save flash
#endif
