/*
  wiring.h - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 1073 2010-08-17 21:50:41Z dmellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 14-108-2009 for attiny45 Saposoft
*/

#ifndef Wiring_h
#define Wiring_h

#include <avr/io.h>
#include <stdlib.h>

#include "binary.h"
#include "core_build_options.h"

#ifdef __cplusplus
extern "C"{
#endif

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define true 0x1
#define false 0x0

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

/* rmv or fix
#if defined(__AVR_ATmega1280__)
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#else
#define INTERNAL 3
#endif
#define DEFAULT 1
#define EXTERNAL 0
*/

/* rmv
analogReference constants for ATmega168.  These are NOT correct for the ATtiny84 nor for the ATtiny85.  The correct values are below.

// Internal 1.1V Voltage Reference with external capacitor at AREF pin
#define INTERNAL 3

// AVCC with external capacitor at AREF pin
#define DEFAULT 1

// AREF, Internal Vref turned off
#define EXTERNAL 0
*/


#if defined( __AVR_ATtiny828__ )

// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)

#define INTERNAL (1)

#elif  defined( __AVR_ATtiny1634__ )

// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)

// External voltage reference at PA0 (AREF) pin, internal reference turned off
#define EXTERNAL (1)

// Internal 1.1V voltage reference
#define INTERNAL (2)

#elif defined( __AVR_ATtinyX41__ )

// 0 0 0 VCC used as Voltage Reference, disconnected from PB0 (AREF).
#define DEFAULT (0)

// 1 0 0 External Voltage Reference at PA0 (AREF) pin, Internal Voltage Reference turned off.
#define EXTERNAL (%100)

// 0 0 1 Internal 1.1V Voltage Reference. AREF disconnected
#define INTERNAL (%001)
#define INTERNAL1V1 INTERNAL

// 1 0 1 Internal 1.1V Voltage Reference with external bypass capacitor at PA0 (AREF) pin(1).
#define INTERNAL_AREF (%101)
#define INTERNAL1V1_AREF INTERNAL_AREF


// 0 1 0 Internal 2.2V Voltage Reference. AREF disconnected
#define INTERNAL2V2 (%010)

// 1 1 0 Internal 2.2V Voltage Reference with external bypass capacitor at PA0 (AREF) pin(1).
#define INTERNAL2V2_AREF (%110)

// 0 1 1 Internal 4.096V Voltage Reference. AREF disconnected
#define INTERNAL4V096 (%011)

// 1 1 1 Internal 4.096 Voltage Reference with external bypass capacitor at PA0 (AREF) pin(1).
#define INTERNAL4V096_AREF (%111)

#endif


// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define interrupts() sei()
#define noInterrupts() cli()

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )
#define microsecondsToClockCycles(a) ( ((a) * (F_CPU / 1000L)) / 1000L )

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

typedef unsigned int word;

#define bit(b) (1UL << (b))

typedef uint8_t boolean;
typedef uint8_t byte;

void initToneTimer(void);
void init(void);

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, int);

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long);
void delayMicroseconds(unsigned int us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

void setup(void);
void loop(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
