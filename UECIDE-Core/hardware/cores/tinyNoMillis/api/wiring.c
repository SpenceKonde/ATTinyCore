/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
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

  $Id: wiring.c 970 2010-05-25 20:16:15Z dmellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 14-10-2009 for attiny45 Saposoft
  Modified 20-11-2010 - B.Cook - Rewritten to use the various Veneers.
*/

#include "wiring_private.h"

void delay(unsigned int ms)
{
  while(ms--){
    delayMicroseconds(1000);
  }
}

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)
  
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
#if F_CPU >= 24000000L
	// for the 24 MHz clock for the aventurous ones, trying to overclock

	// zero delay fix
  if (!us) return; //  = 3 cycles, (4 when true)

	// the following loop takes a 1/6 of a microsecond (4 cycles)
	// per iteration, so execute it six times for each microsecond of
	// delay requested.
	us *= 6; // x6 us, = 7 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 22 (24) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
	us -= 5; //=2 cycles

#elif F_CPU >= 20000000L
	// for the 20 MHz clock on rare Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call takes 18 (20) cycles, which is 1us
	__asm__ __volatile__ (
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop"); //just waiting 4 cycles
  if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes a 1/5 of a microsecond (4 cycles)
	// per iteration, so execute it five times for each microsecond of
	// delay requested.
	us = (us << 2) + us; // x5 us, = 7 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 26 (28) cycles above, remove 7, (7*4=28)
  // us is at least 10 so we can substract 7
	us -= 7; // 2 cycles

#elif F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 1us
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/4 of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2; // x4 us, = 4 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
	us -= 5; // = 2 cycles, 

#elif F_CPU >= 12000000L
	// for the 12 MHz clock if somebody is working with USB

	// for a 1 microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 1.5us
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/3 of a microsecond (4 cycles)
	// per iteration, so execute it three times for each microsecond of
	// delay requested.
	us = (us << 1) + us; // x3 us, = 5 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 20 (22) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
	us -= 5; //2 cycles

#elif F_CPU >= 8000000L
	// for the 8 MHz internal clock

	// for a 1 and 2 microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 2us
	if (us <= 2) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/2 of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1; //x2 us, = 2 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 17 (19) cycles above, remove 4, (4*4=16)
  // us is at least 6 so we can substract 4
	us -= 4; // = 2 cycles

#else
	// for the 1 MHz internal clock (default settings for common AVR microcontrollers)

	// the overhead of the function calls is 14 (16) cycles
	if (us <= 16) return; //= 3 cycles, (4 when true)
	if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to substract 22)

	// compensate for the time taken by the preceeding and next commands (about 22 cycles)
	us -= 22; // = 2 cycles
	// the following loop takes 4 microseconds (4 cycles)
	// per iteration, so execute it us/4 times
  // us is at least 4, divided by 4 gives us 1 (no zero delay bug)
	us >>= 2; // us div 4, = 4 cycles
	

#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
	// return = 4 cycles
}

#if F_CPU == 20000000
  // 20 MHz / 128 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 18432000
  // 18.432 MHz / 128 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 16000000
  // 16 MHz / 128 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 12000000
  // 12 MHz / 64 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B110
#elif F_CPU == 8000000
  // 8 MHz / 64 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B110
#elif F_CPU == 1000000
  // 1 MHz / 8 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B011
#elif F_CPU == 128000
  // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
  #define ADC_ARDUINO_PRESCALER   B000
#else
  #error Add an entry for the selected processor speed.
#endif

void init(void)
{
  sei();
  // Initialize the ADC
  #if defined( INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER ) && INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #if defined(ADCSRA)
    // set a2d prescale factor
	ADCSRA = (ADCSRA & ~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))) | (ADC_ARDUINO_PRESCALER << ADPS0) | (1<<ADEN);
    // enable a2d conversions
    sbi(ADCSRA, ADEN);
  #endif
  #endif
}
