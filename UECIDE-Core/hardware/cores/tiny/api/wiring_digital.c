/*
  wiring_digital.c - digital input and output functions
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

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 14-10-2009 for attiny45 Saposoft
*/

#define ARDUINO_MAIN
#include "wiring_private.h"
#include "pins_arduino.h"

void pinMode(uint8_t pin, uint8_t mode)
{
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t *reg, *out;

	if (port == NOT_A_PIN) return;

	reg = portModeRegister(port);
	out = portOutputRegister(port);

	if (mode == INPUT) { 
		uint8_t oldSREG = SREG;
                cli();
		*reg &= ~bit;
		*out &= ~bit;
		SREG = oldSREG;
	} else if (mode == INPUT_PULLUP) {
		uint8_t oldSREG = SREG;
                cli();
		*reg &= ~bit;
		*out |= bit;
		SREG = oldSREG;
	} else {
		uint8_t oldSREG = SREG;
                cli();
		*reg |= bit;
		SREG = oldSREG;
	}
}

static void turnOffPWM(uint8_t timer)
{
	#if defined(TCCR0A) && defined(COM0A1)
	if( timer == TIMER0A){
		cbi(TCCR0A, COM0A1);
		cbi(TCCR0A, COM0A0);
	} else
	#endif

	#if defined(TCCR0A) && defined(COM0B1)
	if( timer == TIMER0B){
		cbi(TCCR0A, COM0B1);
		cbi(TCCR0A, COM0B0);
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1A1)
	if( timer == TIMER1A){
		cbi(TCCR1A, COM1A1);
		cbi(TCCR1A, COM1A0);
	} else
	#endif

	#if defined(TCCR1) && defined(COM1A1)
	if(timer == TIMER1A){
		cbi(TCCR1, COM1A1);
		cbi(TCCR1, COM1A0);
	#ifdef OC1AX
		cbi(TCCR1D, OC1AX);
	#endif
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1B1)
	if( timer == TIMER1B){
		cbi(TCCR1A, COM1B1);
		cbi(TCCR1A, COM1B0);
	#ifdef OC1BV
		cbi(TCCR1D, OC1BV);
	#endif
	} else
	#endif

	#if defined(TCCR1) && defined(COM1B1)
	if( timer == TIMER1B){
		cbi(GTCCR, COM1B1);
		cbi(GTCCR, COM1B0);
	} else
	#endif
	
    {
    }

}

void digitalWrite(uint8_t pin, uint8_t val)
{
	uint8_t timer = digitalPinToTimer(pin);
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t *out;

	if (port == NOT_A_PIN) return;

	// If the pin that support PWM output, we need to turn it off
	// before doing a digital write.
	if (timer != NOT_ON_TIMER) turnOffPWM(timer);

	out = portOutputRegister(port);

	if (val == LOW) {
		uint8_t oldSREG = SREG;
    cli();
		*out &= ~bit;
		SREG = oldSREG;
	} else {
		uint8_t oldSREG = SREG;
    cli();
		*out |= bit;
		SREG = oldSREG;
	}
}

int digitalRead(uint8_t pin)
{
	uint8_t timer = digitalPinToTimer(pin);
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);

	if (port == NOT_A_PIN) return LOW;

	// If the pin that support PWM output, we need to turn it off
	// before getting a digital reading.
	if (timer != NOT_ON_TIMER) turnOffPWM(timer);

	if (*portInputRegister(port) & bit) return HIGH;
	return LOW;
}
