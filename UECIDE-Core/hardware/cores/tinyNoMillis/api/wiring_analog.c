/*
  wiring_analog.c - analog input and output
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

  Modified  28-08-2009 for attiny84 R.Wiersma
  Modified  14-10-2009 for attiny45 Saposoft
  Corrected 17-05-2010 for ATtiny84 B.Cook
*/

#include "wiring_private.h"
#include "pins_arduino.h"

#ifndef DEFAULT
//For those with no ADC, need to define default.
#define DEFAULT (0)
#endif

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
  // can't actually set the register here because the default setting
  // will connect AVCC and the AREF pin, which would cause a short if
  // there's something connected to AREF.
  // fix? Validate the mode?
  analog_reference = mode;
}

int analogRead(uint8_t pin)
{
  #if defined( NUM_DIGITAL_PINS )
  if ( pin >= NUM_DIGITAL_PINS ) pin -= NUM_DIGITAL_PINS; // allow for channel or pin numbers
  #endif
  
  // fix? Validate pin?
  if(pin >= NUM_ANALOG_INPUTS) return 0; //Not a valid pin.
  #ifndef ADCSRA
  return digitalRead(analogInputToDigitalPin(pin)) ? 1023 : 0; //No ADC, so read as a digital pin instead.
  #endif
  
  #if defined(ADMUX)
  #if defined(MUX4)
  ADMUX = ((analog_reference & 0x03) << REFS0) | ((pin & 0x1F) << MUX0); //select the channel and reference
  #elif defined(MUX3)
  ADMUX = ((analog_reference & 0x03) << REFS0) | ((pin & 0x0F) << MUX0); //select the channel and reference
  #else
  ADMUX = ((analog_reference & 0x03) << REFS0) | ((pin & 0x07) << MUX0); //select the channel and reference
  #endif
  #endif
  #if defined(REFS2)
  ADMUX |= (((analog_reference & 0x04) >> 2) << REFS2); //some have an extra reference bit in a weird position.
  #endif
  
  #if defined(HAVE_ADC) && HAVE_ADC
  sbi(ADCSRA, ADSC); //Start conversion

  while(ADCSRA & (1<<ADSC)); //Wait for conversion to complete.

  uint8_t low = ADCL;
  uint8_t high = ADCH;
  return (high << 8) | low;
  #else
  return LOW;
  #endif
}
// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint8_t pin, int val)
{
  // We need to make sure the PWM output is enabled for those pins
  // that support it, as we turn it off when digitally reading or
  // writing with them.  Also, make sure the pin is in output mode
  // for consistenty with Wiring, which doesn't require a pinMode
  // call for the analog output pins.
  pinMode(pin, OUTPUT);

  if (val <= 0)
  {
    digitalWrite(pin, LOW);
  }
  else if (val >= 255)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    uint8_t timer = digitalPinToTimer(pin);
	#if defined(TCCR0A) && defined(COM0A1)
	if( timer == TIMER0A){
		// connect pwm to pin on timer 0, channel A
		sbi(TCCR0A, COM0A1);
		cbi(TCCR0A, COM0A0);
		OCR0A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR0A) && defined(COM0B1)
	if( timer == TIMER0B){
		// connect pwm to pin on timer 0, channel B
		sbi(TCCR0A, COM0B1);
		cbi(TCCR0A, COM0B0);
		OCR0B = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1A1) && !defined(TCCR1E)
	if( timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1A, COM1A1);
		cbi(TCCR1A, COM1A0);
		OCR1A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1E)
	if( timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1A1S);
		sbi(TCCR1C,COM1A0S);
		OCR1A = val; // set pwm duty
	} else if (timer == TIMER1B){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1B1S);
		sbi(TCCR1C,COM1B0S);
		OCR1B = val; // set pwm duty
	} else if (timer == TIMER1D){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1D1);
		sbi(TCCR1C,COM1D0);
		OCR1D = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1) && defined(COM1A1)
	if(timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1, COM1A1);
		cbi(TCCR1, COM1A0);
		OCR1A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1B1) && !defined(TCCR1E)
	if( timer == TIMER1B){
		// connect pwm to pin on timer 1, channel B
		sbi(TCCR1A, COM1B1);
		cbi(TCCR1A, COM1B0);
		OCR1B = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1) && defined(COM1B1)
	if( timer == TIMER1B){
		// connect pwm to pin on timer 1, channel B
		sbi(GTCCR, COM1B1);
		cbi(GTCCR, COM1B0);
		OCR1B = val; // set pwm duty
	} else
	#endif
	
    {
      if (val < 128)
      {
        digitalWrite(pin, LOW);
      }
      else
      {
        digitalWrite(pin, HIGH);
      }
    }

  }
}
