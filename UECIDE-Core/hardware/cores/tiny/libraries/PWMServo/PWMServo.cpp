#include <avr/interrupt.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif
#include "PWMServo.h"

/*
  PWMServo.cpp - Hardware Servo Timer Library
  http://arduiniana.org/libraries/pwmservo/
  Author: Jim Studt, jim@federated.com
  Copyright (c) 2007 David A. Mellis.  All right reserved.
  renamed to PWMServo by Mikal Hart
  ported to other chips by Paul Stoffregen

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

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

uint8_t PWMServo::attachedA = 0;
uint8_t PWMServo::attachedB = 0;
#ifdef SERVO_PIN_C
uint8_t PWMServo::attachedC = 0;
#endif

void PWMServo::seizeTimer1()
{
  uint8_t oldSREG = SREG;

  cli();
  TCCR1A = _BV(WGM11); /* Fast PWM, ICR1 is top */
  TCCR1B = _BV(WGM13) | _BV(WGM12) /* Fast PWM, ICR1 is top */
  | _BV(CS11) /* div 8 clock prescaler */
  ;
  OCR1A = 3000;
  OCR1B = 3000;
  ICR1 = clockCyclesPerMicrosecond()*(20000L/8);  // 20000 uS is a bit fast for the refresh, 20ms, but 
                                                  // it keeps us from overflowing ICR1 at 20MHz clocks
                                                  // That "/8" at the end is the prescaler.
#if defined(__AVR_ATmega8__)
  TIMSK &= ~(_BV(TICIE1) | _BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
#else
  TIMSK1 &=  ~(_BV(OCIE1A) | _BV(OCIE1B) | _BV(TOIE1) );
#endif

  SREG = oldSREG;  // undo cli()    
}

void PWMServo::releaseTimer1() {
#if defined(TCCR1B) && defined(CS11) && defined(CS10)
	TCCR1A = 0;
	TCCR1B = 0;

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1B, CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
	sbi(TCCR1, CS11);
#if F_CPU >= 8000000L
	sbi(TCCR1, CS10);
#endif
#endif
	// put timer 1 in 8-bit phase correct pwm mode
#if defined(TCCR1A) && defined(WGM10)
	sbi(TCCR1A, WGM10);
#elif defined(TCCR1)
	#warning this needs to be finished
#endif
}

#define NO_ANGLE (0xff)

PWMServo::PWMServo() : pin(0), angle(NO_ANGLE) {}

uint8_t PWMServo::attach(int pinArg)
{
  return attach(pinArg, 544, 2400);
}

uint8_t PWMServo::attach(int pinArg, int min, int max)
{
  #ifdef SERVO_PIN_C
  if (pinArg != SERVO_PIN_A && pinArg != SERVO_PIN_B && pinArg != SERVO_PIN_C) return 0;
  #else
  if (pinArg != SERVO_PIN_A && pinArg != SERVO_PIN_B) return 0;
  #endif

  min16 = min / 16;
  max16 = max / 16;

  pin = pinArg;
  angle = NO_ANGLE;
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);

  #ifdef SERVO_PIN_C
  if (!attachedA && !attachedB && !attachedC) seizeTimer1();
  #else
  if (!attachedA && !attachedB) seizeTimer1();
  #endif

  if (pin == SERVO_PIN_A) {
    attachedA = 1;
    TCCR1A = (TCCR1A & ~_BV(COM1A0)) | _BV(COM1A1);
  }

  if (pin == SERVO_PIN_B) {
    attachedB = 1;
    TCCR1A = (TCCR1A & ~_BV(COM1B0)) | _BV(COM1B1);
  }

  #ifdef SERVO_PIN_C
  if (pin == SERVO_PIN_C) {
    attachedC = 1;
    TCCR1A = (TCCR1A & ~_BV(COM1C0)) | _BV(COM1C1);
  }
  #endif
  return 1;
}

void PWMServo::detach()
{
  // muck with timer flags
  if (pin == SERVO_PIN_A) {
    attachedA = 0;
    TCCR1A = TCCR1A & ~_BV(COM1A0) & ~_BV(COM1A1);
    //pinMode(pin, INPUT);
  }

  if (pin == SERVO_PIN_B) {
    attachedB = 0;
    TCCR1A = TCCR1A & ~_BV(COM1B0) & ~_BV(COM1B1);
    //pinMode(pin, INPUT);
  }

  #ifdef SERVO_PIN_C
  if (pin == SERVO_PIN_C) {
    attachedC = 0;
    TCCR1A = TCCR1A & ~_BV(COM1C0) & ~_BV(COM1C1);
    //pinMode(pin, INPUT);
  }
  #endif

  #ifdef SERVO_PIN_C
  if (!attachedA && !attachedB && !attachedC) releaseTimer1();
  #else
  if (!attachedA && !attachedB) releaseTimer1();
  #endif
}

void PWMServo::write(int angleArg)
{
  uint16_t p;

  if (angleArg < 0) angleArg = 0;
  if (angleArg > 180) angleArg = 180;
  angle = angleArg;

  // bleh, have to use longs to prevent overflow, could be tricky if always a 16MHz clock, but not true
  // That 8L on the end is the TCNT1 prescaler, it will need to change if the clock's prescaler changes,
  // but then there will likely be an overflow problem, so it will have to be handled by a human.
  p = (min16*16L*clockCyclesPerMicrosecond() + (max16-min16)*(16L*clockCyclesPerMicrosecond())*angle/180L)/8L;
  if (pin == SERVO_PIN_A) OCR1A = p;
  if (pin == SERVO_PIN_B) OCR1B = p;
  #ifdef SERVO_PIN_C
  if (pin == SERVO_PIN_C) OCR1C = p;
  #endif
}

uint8_t PWMServo::read()
{
  return angle;
}

uint8_t PWMServo::attached()
{
  if (pin == SERVO_PIN_A && attachedA) return 1;
  if (pin == SERVO_PIN_B && attachedB) return 1;
  #ifdef SERVO_PIN_C
  if (pin == SERVO_PIN_C && attachedC) return 1;
  #endif
  return 0;
}
