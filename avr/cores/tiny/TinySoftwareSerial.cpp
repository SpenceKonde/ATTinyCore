/*
  TinySoftwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.All right reserved.

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

#if USE_SOFTWARE_SERIAL
#include "TinySoftwareSerial.h"

extern "C"{

  #ifndef SOFT_TX_ONLY
    /* Manually inlining because throws a warnign that might not be able to inline this, and we really want to make sure this is always inlined,
    because it's in an ISR, and calling C functions within an ISR is warned against in the datasheet, it's so inefficient and bloat-inducing.
    __attribute__((always_inline)) uint8_t getch() {
      uint8_t ch = 0x80;
        __asm__ __volatile__ (
        "clc"                      "\n\t"   // clear carry, we may then set it again. uartDelay DOES NOT CHANGE CARRY because they use DEC decrement the number...
        "rcall uartDelay"          "\n\t"   // Get to 0.25 of start bit (our baud is too fast, so give room to correct). Spence: No, you get 0.25 bits further than you were when the start of the ISR finished.
       "_rxstart:"                          //  That depends on baud rate / clock speed and the size of the interrupt vectors.... and halfway in is what you want.
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "sbic %[pin], %[rxbit]"    "\n\t"   // 1 This is how it's done!
        "sec"                      "\n\t"   // 2 Set the carry flag so we can shift it in
        "ror %[rxch]"              "\n\t"   // 3 This clears the carry bit UNTIL it shifts out the bit from the 0x80, which indicates that we got the whole byte and are done.
        "brcc _rxstart"            "\n\t"   // 5 clock total. Assuming carry is clear, we go back to the delay.
        //"rcall uartDelay"        "\n\t"   // Wait 0.25 bit period
        //"rcall uartDelay"        "\n\t"   // Wait 0.25 bit period These two lines were to ensure that this ended only once we were into the stop bit. Old code did NOT guarantee that due to clock error.
        "sbis %[pin], %[rxbit]"    "\n\t"   // but SBIS provides us with an easy solution - checking the port input bit itself, with an rjmp back to the test after it
        "rjmp .-4"                 "\n\t"   // it will only skip the rjmp if the pin is high, and thus the last 0 bit is over, and we've recorded the last bit, and it's into the stop bit and we can
                                            // let app code run until the next byte starts.
        :
          [rxch]"=r" (ch)
        : [pin] "I" (_SFR_IO_ADDR(SOFTSERIAL_PIN)),
          [rxbit] "I" (SOFTSERIAL_RXBIT)
        : "r0"
        );
      return ch;
    }*/

    soft_ring_buffer rx_buffer = {{ 0 }, 0, 0};
    ISR(SOFTSERIAL_vect) {
      uint8_t ch = 0x80;
        __asm__ __volatile__ (
        "clc"                      "\n\t"   // clear carry, we may then set it again. uartDelay DOES NOT CHANGE CARRY because they use DEC decrement the number...
        "rcall uartDelay"          "\n\t"   // Get to 0.25 of start bit (our baud is too fast, so give room to correct). Spence: No, you get 0.25 bits further than you were when the start of the ISR finished.
       "_rxstart:"                          //  That depends on baud rate / clock speed and the size of the interrupt vectors.... and halfway in is what you want.
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "rcall uartDelay"          "\n\t"   // Wait 0.25 bit period
        "sbic %[pin], %[rxbit]"    "\n\t"   // 1 This is how it's done!
        "sec"                      "\n\t"   // 2 Set the carry flag so we can shift it in
        "ror %[rxch]"              "\n\t"   // 3 This clears the carry bit UNTIL it shifts out the bit from the 0x80, which indicates that we got the whole byte and are done.
        "brcc _rxstart"            "\n\t"   // 5 clock total. Assuming carry is clear, we go back to the delay.
        //"rcall uartDelay"        "\n\t"   // Wait 0.25 bit period
        //"rcall uartDelay"        "\n\t"   // Wait 0.25 bit period These two lines were to ensure that this ended only once we were into the stop bit. Old code did NOT guarantee that due to clock error.
        "sbis %[pin], %[rxbit]"    "\n\t"   // but SBIS provides us with an easy solution - checking the port input bit itself, with an rjmp back to the test after it
        "rjmp .-4"                 "\n\t"   // it will only skip the rjmp if the pin is high, and thus the last 0 bit is over, and we've recorded the last bit, and it's into the stop bit and we can
                                            // let app code run until the next byte starts.
        :
          [rxch]"=r" (ch)
        : [pin] "I" (_SFR_IO_ADDR(SOFTSERIAL_PIN)),
          [rxbit] "I" (SOFTSERIAL_RXBIT)
        : "r0"
      );
      uint8_t i = (uint8_t)(rx_buffer.head + 1) & (SERIAL_BUFFER_SIZE-1);

      // if we should be storing the received character into the location
      // just before the tail (meaning that the head would advance to the
      // current location of the tail), we're about to overflow the buffer
      // and so we don't write the character or advance the head.
      if (i != rx_buffer.tail) {
        rx_buffer.buffer[rx_buffer.head] = ch;
        rx_buffer.head = i;
      }
      sbi(ACSR, ACI); //clear the flag.
    }

  #endif


  void uartDelay() {
    __asm__ __volatile__ (
      "mov  r0, %[count]"     "\n\t"
      "dec  r0"               "\n\t"
      "brne .-4"              "\n\t"
      "ret"                   "\n\t"
      ::[count] "r" ((uint8_t)Serial._delayCount)
    );
  }
}
TinySoftwareSerial::TinySoftwareSerial(soft_ring_buffer *rx_buffer) {
  _rx_buffer = rx_buffer;

  _txmask   = _BV(SOFTSERIAL_TXBIT);
  _txunmask = ~_txmask;

  _delayCount = 0;
}

void TinySoftwareSerial::setTxBit(uint8_t txbit) {
  _txmask   = _BV(txbit);
  _txunmask =    ~_txmask;
}

void TinySoftwareSerial::begin(long baud) {
  long tempDelay = (((F_CPU/baud) - 39) / 12);
  if ((tempDelay > 255) || (tempDelay <= 0)) {
    return; //Cannot start - baud rate out of range.
  }
  _delayCount = (uint8_t)tempDelay;
  #ifndef SOFT_TX_ONLY
    //Straight assignment, we need to configire all bits
    ACSR = (1 << ACBG) | (1 << ACIS1) | (1 << ACIS0) | (1 << ACI);
    // These should compile to cbi and sbi - everything is compile time known.
    SOFTSERIAL_DDR    &=  ~(1 << SOFTSERIAL_RXBIT);  // set RX to an input
    SOFTSERIAL_PORT   |=   (1 << SOFTSERIAL_RXBIT);  // enable pullup on RX pin - to prevent accidental interrupt triggers.
    #if !defined(__ATtinyx8__) // on most classic tinyies ACSR is in the low IO space, so we can use sbi, cbi instructions.
      ACSR            |=   (1 <<  ACI);              // clear the flag - above configuration may cause it to be set.
      ACSR            |=   (1 << ACIE);              // turn on the comparator interrupt
    #else
      ACSR = (1 << ACBG) | (1 << ACIS1) | (1 << ACIS0) | (1 << ACI) | (1 << ACIE); // do this all with an LDI/OUT on the x8 series.
    #endif
    #ifdef ACSRB
      /* This is only the case on an ATtiny x61, out of the parts that support this builtin Software serial. The 1634 and 828 have it
       * but they aren't supported because they have hardware serial. The 841 has multiple AC's, which would give the same sort of choices, except
       * that t has hardware serial ports too. so we don't provide the builtin softSeral.
       * On the 861, this is located in the low I/O space. We assume ACSRB is is POR state, hence 0, so it's better to use |=, which compiles to SBI instead of = x which compiles to ldi r__, x out ACSRB,x
       */
      #if defined(SOFTSERIAL_RXAIN0)
        ACSRB |= 2; // Use PA6 (AIN0)
      #elif defined(SOFTSERIAL_RXAIN2)
        ACSRB |= 1; // Use PA5 (AIN2)
      #endif
      // Otherwise we leave it at 0;
    #endif
  #endif
  uint8_t oldsreg      =   SREG;    //  These are NOT going to get compiled to cbi/sbi as _txmask is not compile time known.
  cli();                            //  so we need to protect this.
  SOFTSERIAL_DDR      |=   _txmask; //  set TX to an output.
  SOFTSERIAL_PORT     |=   _txmask; //  set TX pin high
  SREG                 =   oldsreg; //  restore SREG.
}

void TinySoftwareSerial::end() {
  #ifndef SOFT_TX_ONLY
    ACSR = (1 << ACD) | (1 << ACI); // turn off the analog comparator, clearing the flag while we're at it. We also in the process clear all the other settings.
    _delayCount = 0;
    _rx_buffer->head = _rx_buffer->tail;
  #endif
}

int TinySoftwareSerial::available(void) {
  #ifndef SOFT_TX_ONLY
    return (uint8_t)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
  #else
    return 0;
  #endif
}


int TinySoftwareSerial::peek(void) {
  #ifndef SOFT_TX_ONLY
    if (_rx_buffer->head == _rx_buffer->tail) {
      return -1;
    } else {
      return _rx_buffer->buffer[_rx_buffer->tail];
    }
  #else
    return -1;
  #endif
}

int TinySoftwareSerial::read(void) {
  #ifndef SOFT_TX_ONLY
    // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buffer->head == _rx_buffer->tail) {
      return -1;
    } else {
      uint8_t c = _rx_buffer->buffer[_rx_buffer->tail];
      _rx_buffer->tail = (uint8_t)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
      return c;
    }
  #else
    return -1;
  #endif
}

size_t TinySoftwareSerial::write(uint8_t ch) {
  uint8_t oldSREG = SREG;
  cli(); //Prevent interrupts from breaking the transmission. Note: TinySoftwareSerial is half duplex.
  //it can either receive or send, not both (because receiving requires an interrupt and would stall transmission
  uint8_t zerobit = SOFTSERIAL_PORT & ~_txmask;
  uint8_t onebit = SOFTSERIAL_PORT | _txmask;
  __asm__ __volatile__ (
    "com %[ch]"                   "\n\t" // ones complement, carry set
    "sec"                         "\n\t"
  "_txstart:"
    "brcc _txpart2"               "\n\t" //  1  no branch first bit
    "out %[uartPort], %[zerobit]" "\n\t" //  2 output a zero bit
    "rjmp .+4"                    "\n\t" //  4 jump to the delay part.
  "_txpart2:"                            //  2 used with brcc with branch
    "out %[uartPort], %[onebit]"  "\n\t" //  3 output a 1 bit
    "nop"                         "\n\t" //  4 so the two paths have equal running time.
  "_txdelay:"                            //  4 clocks either way
    "rcall uartDelay"             "\n\t"
    "rcall uartDelay"             "\n\t"
    "rcall uartDelay"             "\n\t"
    "rcall uartDelay"             "\n\t"
    "lsr %[ch]"                   "\n\t"
    "dec %[count]"                "\n\t"
    "brne _txstart"               "\n\t"
    :
    :
      [ch] "r" (ch),
      [count] "r" ((uint8_t)10),
      [uartPort] "I" (_SFR_IO_ADDR(SOFTSERIAL_PORT)),
      [zerobit] "r" (zerobit),
      [onebit] "r" (onebit)
    : "r0"
  );
  SREG = oldSREG;
  return 1;
}

void TinySoftwareSerial::flush() {
  ;
}

TinySoftwareSerial::operator bool() {
  return true;
}

TinySoftwareSerial Serial(&rx_buffer);

#endif // whole file
