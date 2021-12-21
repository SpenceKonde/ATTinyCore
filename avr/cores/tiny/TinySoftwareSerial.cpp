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

// Define constants and variables for buffering incoming serial data. We're
// using a ring buffer, in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.

extern "C"{
#ifndef SOFT_TX_ONLY
  uint8_t getch() {
    uint8_t ch = 0;
      __asm__ __volatile__ (
      "rcall uartDelay"        "\n\t"           // Get to 0.25 of start bit (our baud is too fast, so give room to correct)
     "_rxstart: "
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      "clc"                    "\n\t"
      "in    r23, %[pin]"      "\n\t"           // Could be changed to use SBIS
      "and   r23, %[mask]"     "\n\t"
      "breq  .+2"              "\n\t"
      "sec"                    "\n\t"
      "ror   %0"               "\n\t"
      "dec   %[count]"         "\n\t"
      "brne _rxstart"          "\n\t"
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      "rcall uartDelay"        "\n\t"           // Wait 0.25 bit period
      :
        "=r" (ch)
      :
        "0" ((uint8_t)0),
        [count] "r" ((uint8_t)8),
        [pin] "I" (_SFR_IO_ADDR(ANALOG_COMP_PIN)),
        [mask] "r" (Serial._rxmask)
      :
        "r23",
        "r24",
        "r25"
      );
    return ch;
  }

  #if !defined(ANALOG_COMP_vect) && defined(ANA_COMP_vect)
  //rename the vector so we can use it.
    #define ANALOG_COMP_vect ANA_COMP_vect
  #elif !defined (ANALOG_COMP_vect)
    #error Tiny Software Serial cannot find the Analog comparator interrupt vector!
  #endif
  ISR(ANALOG_COMP_vect) {
    char ch = getch(); //read in the character softwarily - I know its not a word, but it sounded cool, so you know what: #define softwarily 1
    store_char(ch, Serial._rx_buffer);
    sbi(ACSR,ACI); //clear the flag.
  }

  }
  soft_ring_buffer rx_buffer = {{ 0 }, 0, 0};
#endif


void uartDelay() {
  __asm__ __volatile__ (
    "mov  r25, %[count]"    "\n\t"
    "dec  r25"              "\n\t"
    "brne .-4"              "\n\t"
    "ret"                   "\n\t"
    ::[count] "r" ((uint8_t)Serial._delayCount)
  );
}

TinySoftwareSerial::TinySoftwareSerial(soft_ring_buffer *rx_buffer, uint8_t txBit, uint8_t rxBit) {
  _rx_buffer = rx_buffer;

  _rxmask = _BV(rxBit);
  _txmask = _BV(txBit);
  _txunmask = ~_txmask;

  _delayCount = 0;
}

void TinySoftwareSerial::setTxBit(uint8_t txbit) {
  _txmask=_BV(txbit);
  _txunmask=~txbit;
}

void TinySoftwareSerial::begin(long baud) {
  long tempDelay = (((F_CPU/baud)-39)/12);
  if ((tempDelay > 255) || (tempDelay <= 0)) {
    end(); //Cannot start as it would screw up uartDelay().
  }
  _delayCount = (uint8_t)tempDelay;
  #ifndef SOFT_TX_ONLY
    // cbi(ACSR,ACIE);   //turn off the comparator interrupt to allow change of ACD
    // We will assume that the user has not reconfigured the AC first.
    ACSR = (1 << ACBG) | (1 << ACIS1) | (1 << ACIS0) | (1 << ACI);
    /*
    #ifdef ACBG
      sbi(ACSR,ACBG);   //enable the internal bandgap reference - used instead of AIN0 to allow it to be used for TX
    #endif
    cbi(ACSR,ACD);    //turn on the comparator for RX
    #ifdef ACIC
      cbi(ACSR,ACIC);   //prevent the comparator from affecting timer1 - just to be safe.
    #endif
    sbi(ACSR,ACIS1);  //interrupt on rising edge (this means RX has gone from Mark state to Start bit state).
    sbi(ACSR,ACIS0);
    */
    //Setup the pins in case someone messed with them.
    ANALOG_COMP_DDR   &= ~_rxmask; //set RX to an input
    ANALOG_COMP_PORT  |=  _rxmask; //enable pullup on RX pin - to prevent accidental interrupt triggers.
    ACSR              |= 1 << ACI;   // clear the flag - above configuration may cause it to be set.
    ACSR              |= 1 << ACIE;  // turn on the comparator interrup
    #ifdef ACSRB
      ACSRB = 0; //Use AIN0 as +, AIN1 as -, no hysteresis - just like ones without this register.
    #endif
  #endif
  ANALOG_COMP_DDR   |=  _txmask; //set TX to an output.
  ANALOG_COMP_PORT  |=  _txmask; //set TX pin high
}

void TinySoftwareSerial::end() {
  #ifndef SOFT_TX_ONLY
    ACSR = (1 << ACD) | (1 << ACI) // turn off the analog comparator, clearing the flag while we're at it
    /*
    sbi(ACSR,ACI);   //clear the flag.
    cbi(ACSR,ACIE);  //turn off the comparator interrupt to allow change of ACD, and because it needs to be turned off now too!
    #ifdef ACBG
      cbi(ACSR,ACBG); // disable the bandgap reference
    #endif
    sbi(ACSR,ACD);  // turn off the comparator to save power
    */
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

inline void store_char(unsigned char c, soft_ring_buffer *buffer) {
  int i = (uint8_t)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
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
  __asm__ __volatile__ (
    "com %[ch]"                 "\n\t" // ones complement, carry set
    "sec"                       "\n\t"
  "_txstart:"
    "brcc _txpart2"             "\n\t"
    "in r23,%[uartPort]"        "\n\t"
    "and r23,%[uartUnmask]"     "\n\t"
    "out %[uartPort], r23"      "\n\t"
    "rjmp 3f"                   "\n\t"
  "_txpart2:"
    "in r23, %[uartPort]"       "\n\t"
    "or r23, %[uartMask]"       "\n\t"
    "out %[uartPort], r23"      "\n\t"
    "nop"     "\n\t"
  "_txdelay:"
    "rcall uartDelay"           "\n\t"
    "rcall uartDelay"           "\n\t"
    "rcall uartDelay"           "\n\t"
    "rcall uartDelay"           "\n\t"
    "lsr %[ch]"                 "\n\t"
    "dec %[count]"              "\n\t"
    "brne _txstart"             "\n\t"
    :
    :
      [ch] "r" (ch),
      [count] "r" ((uint8_t)10),
      [uartPort] "I" (_SFR_IO_ADDR(ANALOG_COMP_PORT)),
      [uartMask] "r" (_txmask),
      [uartUnmask] "r" (_txunmask)
    : "r23",
      "r24",
      "r25"
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

#ifndef ANALOG_COMP_DDR
  #error Please define ANALOG_COMP_DDR in the pins_arduino.h file!
#endif

#ifndef ANALOG_COMP_PORT
  #error Please define ANALOG_COMP_PORT in the pins_arduino.h file!
#endif

#ifndef ANALOG_COMP_PIN
  #error Please define ANALOG_COMP_PIN in the pins_arduino.h file!
#endif

#ifndef ANALOG_COMP_AIN0_BIT
  #error Please define ANALOG_COMP_AIN0_BIT in the pins_arduino.h file!
#endif

#ifndef ANALOG_COMP_AIN1_BIT
  #error Please define ANALOG_COMP_AIN1_BIT in the pins_arduino.h file!
#endif

TinySoftwareSerial Serial(&rx_buffer, ANALOG_COMP_AIN0_BIT, ANALOG_COMP_AIN1_BIT);

#endif // whole file
