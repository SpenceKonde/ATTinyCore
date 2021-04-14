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

#if defined(REFS1)
#define ADMUX_REFS_MASK (0x03)
#else
#define ADMUX_REFS_MASK (0x01)
#endif

#if defined(MUX5)
#define ADMUX_MUX_MASK (0x3f)
#elif defined(MUX4)
#define ADMUX_MUX_MASK (0x1f)
#elif defined(MUX3)
#define ADMUX_MUX_MASK (0x0f)
#else
#define ADMUX_MUX_MASK (0x07)
#endif

int analogRead(uint8_t pin)
{
  pin &=127; //strip off the high bit of the A# constants
  #ifndef ADCSRA
  return digitalRead(analogInputToDigitalPin(pin)) ? 1023 : 0; //No ADC, so read as a digital pin instead.
  #endif

  #if defined(ADMUX)
  ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0) | ((pin & ADMUX_MUX_MASK) << MUX0); //select the channel and reference
  #if defined(REFS2)
  ADMUX |= (((analog_reference & 0x04) >> 2) << REFS2); //some have an extra reference bit in a weird position.
  #endif
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
    //cbi(TCCR0A, COM0A0);
    OCR0A = val; // set pwm duty
  } else
  #endif

  #if defined(TCCR0A) && defined(COM0B1)
  if( timer == TIMER0B){
    // connect pwm to pin on timer 0, channel B
    sbi(TCCR0A, COM0B1);
    //cbi(TCCR0A, COM0B0);
    OCR0B = val; // set pwm duty
  } else
  #endif
  #if defined(__AVR_ATtinyX7__)
  if (timer&0x10) {
    //Timer 1
    TCCR1A|=(1<<COM1B1)|(1<<COM1A1);
    if (timer&0x04){
      //TCCR1D&=(0x0F); //clear all PWM on same channel
      OCR1B=val;
    } else {
      //TCCR1D&=(0xF0); //clear all PWM on same channel
      OCR1A=val;
    }
    TCCR1D|=(1<<(timer&0x07));
  } else

  #elif defined(TCCR1A) && defined(COM1A1) && !defined(TCCR1E)
    //TCCR1E is present only on tiny861, and there's no TCCR1A on Tiny85.
    //So this handles "normal" timers
  if( timer == TIMER1A){
    // connect pwm to pin on timer 1, channel A
    sbi(TCCR1A, COM1A1);
    //cbi(TCCR1A, COM1A0);
    OCR1A = val; // set pwm duty
  } else
  #endif
  // ATtiny x61
    // This can be recoded to use the OCOEn bits in TCCR1E
    // This would be much better - then we'd leave COM bits at 0, and just switch on and off the OCOEn bits
    // In this case, we would use WGM10 or WGM11 (PWM6 mode). Only one duty cycle could be output on each of the three pairs of
    // PWM pins, but it gives you more choice on which pins you use. Would implement it like we do on x7, ie, if you analogWrite()
    // both pins, and didn't turn off PWM between with digitalWrite(), you'd have identical waveform on the two pins.
  #if defined(TCCR1E) //Tiny861
    if( timer == TIMER1A){
      // connect pwm to pin on timer 1, channel A
      //cbi(TCCR1C,COM1A1S);
      sbi(TCCR1C,COM1A0S);
      //sbi(TCCR1A,PWM1A);
      OCR1A = val; // set pwm duty
    } else if (timer == TIMER1B){
      // connect pwm to pin on timer 1, channel A
      //cbi(TCCR1C,COM1B1S);
      sbi(TCCR1C,COM1B0S);
      //sbi(TCCR1A,PWM1B);
      OCR1B = val; // set pwm duty
    } else if (timer == TIMER1D){
      // connect pwm to pin on timer 1, channel A
      //cbi(TCCR1C,COM1D1);
      sbi(TCCR1C,COM1D0);
      //sbi(TCCR1A,PWM1D);
      OCR1D = val; // set pwm duty
    } else
  #endif

  #if defined(TCCR1) && defined(COM1A1) //Tiny85
    if(timer == TIMER1A){
      // connect pwm to pin on timer 1, channel A
      sbi(TCCR1, COM1A1);
      //cbi(TCCR1, COM1A0);
      OCR1A = val; // set pwm duty
    } else
  #endif

  #if defined(TCCR1A) && defined(COM1B1) && !defined(TCCR1E)
    if( timer == TIMER1B){
      // connect pwm to pin on timer 1, channel B
      sbi(TCCR1A, COM1B1);
      //cbi(TCCR1A, COM1B0);
      OCR1B = val; // set pwm duty
    } else
  #endif

  #if defined(TCCR1) && defined(COM1B1)
    if( timer == TIMER1B){
      // connect pwm to pin on timer 1, channel B
      sbi(GTCCR, COM1B1);
      //cbi(GTCCR, COM1B0);
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
