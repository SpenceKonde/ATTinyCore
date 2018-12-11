/*
  TinySoftwareSerial.cpp - Hardware serial library for Wiring
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

#if USE_SOFTWARE_SERIAL 
#include "TinySoftwareSerial.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.

extern "C"{
uint8_t getch() {
  uint8_t ch = 0;
    __asm__ __volatile__ (
		"   rcall uartDelay\n"          // Get to 0.25 of start bit (our baud is too fast, so give room to correct)
		"1: rcall uartDelay\n"              // Wait 0.25 bit period
		"   rcall uartDelay\n"              // Wait 0.25 bit period
		"   rcall uartDelay\n"              // Wait 0.25 bit period
		"   rcall uartDelay\n"              // Wait 0.25 bit period
		"   clc\n"
		"   in r23,%[pin]\n"
		"   and r23, %[mask]\n"
		"   breq 2f\n"
		"   sec\n"
		"2: ror   %0\n"                    
		"   dec   %[count]\n"
		"   breq  3f\n"
		"   rjmp  1b\n"
		"3: rcall uartDelay\n"              // Wait 0.25 bit period
		"   rcall uartDelay\n"              // Wait 0.25 bit period
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

void uartDelay() {
	__asm__ __volatile__ (
	  "mov r25,%[count]\n"
	  "1:dec r25\n"
      "brne 1b\n"
      "ret\n"
	  ::[count] "r" ((uint8_t)Serial._delayCount)
	);
}

#if !defined (ANALOG_COMP_vect) && defined(ANA_COMP_vect)
//rename the vector so we can use it.
	#define ANALOG_COMP_vect ANA_COMP_vect
#elif !defined (ANALOG_COMP_vect)
	#error Tiny Software Serial cant find the Analog comparator interrupt vector!
#endif
ISR(ANALOG_COMP_vect){
  char ch = getch(); //read in the character softwarily - I know its not a word, but it sounded cool, so you know what: #define softwarily 1
  store_char(ch, Serial._rx_buffer);
  sbi(ACSR,ACI); //clear the flag.
}

}
soft_ring_buffer rx_buffer  =  { { 0 }, 0, 0 };

// Constructor ////////////////////////////////////////////////////////////////

TinySoftwareSerial::TinySoftwareSerial(soft_ring_buffer *rx_buffer, uint8_t txBit, uint8_t rxBit)
{
  _rx_buffer = rx_buffer;
  
  _rxmask = _BV(rxBit);
  _txmask = _BV(txBit);
  _txunmask = ~_txmask;
  
  _delayCount = 0;	
}

// Public Methods //////////////////////////////////////////////////////////////

void TinySoftwareSerial::begin(long baud)
{
  long tempDelay = (((F_CPU/baud)-39)/12);
  if ((tempDelay > 255) || (tempDelay <= 0)){
	end(); //Cannot start as it would screw up uartDelay().
  }
  _delayCount = (uint8_t)tempDelay;
  cbi(ACSR,ACIE);  //turn off the comparator interrupt to allow change of ACD
#ifdef ACBG
  sbi(ACSR,ACBG); //enable the internal bandgap reference - used instead of AIN0 to allow it to be used for TX.
#endif
  cbi(ACSR,ACD);  //turn on the comparator for RX
#ifdef ACIC
  cbi(ACSR,ACIC);  //prevent the comparator from affecting timer1 - just to be safe.
#endif
  sbi(ACSR,ACIS1);  //interrupt on rising edge (this means RX has gone from Mark state to Start bit state).
  sbi(ACSR,ACIS0);  
  //Setup the pins in case someone messed with them.
  ANALOG_COMP_DDR &= ~_rxmask; //set RX to an input
  ANALOG_COMP_PORT |= _rxmask; //enable pullup on RX pin - to prevent accidental interrupt triggers.
  ANALOG_COMP_DDR |= _txmask; //set TX to an output.
  ANALOG_COMP_PORT |= _txmask; //set TX pin high
  sbi(ACSR,ACI); //clear the flag.
  sbi(ACSR,ACIE);  //turn on the comparator interrupt to allow us to use it for RX
#ifdef ACSRB
  ACSRB = 0; //Use AIN0 as +, AIN1 as -, no hysteresis - just like ones without this register.
#endif
}

void TinySoftwareSerial::end()
{
  sbi(ACSR,ACI); //clear the flag.
  cbi(ACSR,ACIE);  //turn off the comparator interrupt to allow change of ACD, and because it needs to be turned off now too!
#ifdef ACBG
  cbi(ACSR,ACBG); //disable the bandgap reference
#endif
  sbi(ACSR,ACD);  //turn off the comparator to save power
  _delayCount = 0;
  _rx_buffer->head = _rx_buffer->tail;
}

int TinySoftwareSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

void store_char(unsigned char c, soft_ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

int TinySoftwareSerial::peek(void)
{
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

int TinySoftwareSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

size_t TinySoftwareSerial::write(uint8_t ch)
{
  uint8_t oldSREG = SREG;
  cli(); //Prevent interrupts from breaking the transmission. Note: TinySoftwareSerial is half duplex.
  //it can either recieve or send, not both (because recieving requires an interrupt and would stall transmission
  __asm__ __volatile__ (
    "   com %[ch]\n" // ones complement, carry set
    "   sec\n"
    "1: brcc 2f\n"
	"   in r23,%[uartPort] \n"
    "   and r23,%[uartUnmask]\n"
	"   out %[uartPort],r23 \n"
    "   rjmp 3f\n"
	"2: in r23,%[uartPort] \n"
    "   or r23,%[uartMask]\n"
	"   out %[uartPort],r23 \n"
    "   nop\n"
    "3: rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   lsr %[ch]\n"
    "   dec %[count]\n"
    "   brne 1b\n"
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

void TinySoftwareSerial::flush()
{
  
}

TinySoftwareSerial::operator bool() {
	return true;
}

// Preinstantiate Objects //////////////////////////////////////////////////////
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
