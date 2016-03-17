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
  Modified 26-02-2015 for attiny841 and other parts that use the PUEx registers. 
*/

#include "wiring_private.h"
#include "pins_arduino.h"
#include "core_pins.h"
#include "core_timers.h"
#include "PwmTimer.h"

#if defined(__AVR_ATtinyX41__ ) || defined(__AVR_ATtiny1634__) || defined(__AVR_ATtiny828__)
void pinMode(uint8_t pin, uint8_t mode)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *reg, *out, *pue;

  if (port == NOT_A_PIN) return;

  // JWS: can I let the optimizer do this?
  reg = portModeRegister(port);
  out = portOutputRegister(port);
  pue = portPullupRegister(port);

  if (mode == INPUT) { 
    uint8_t oldSREG = SREG;
                cli();
    *reg &= ~bit;
    *out &= ~bit;
    *pue &= ~bit;
    SREG = oldSREG;
  } else if (mode == INPUT_PULLUP) {
    uint8_t oldSREG = SREG;
                cli();
    *reg &= ~bit;
    *out |= bit;
    *pue |= bit;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
                cli();
    *pue &= ~bit;
    *reg |= bit;
    SREG = oldSREG;
  }
}
#else 

void pinMode(uint8_t pin, uint8_t mode)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *reg, *out;

  if (port == NOT_A_PIN) return;

  // JWS: can I let the optimizer do this?
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

#endif

// Forcing this inline keeps the callers from having to push their own stuff
// on the stack. It is a good performance win and only takes 1 more byte per
// user than calling. (It will take more bytes on the 168.)
//
// But shouldn't this be moved into pinMode? Seems silly to check and do on
// each digitalread or write.
//
__attribute__((always_inline)) static inline void turnOffPWM( uint8_t pin )
{
  #if CORE_PWM_COUNT >= 1
    if ( pin == CORE_PWM0_PIN )
    {
      Pwm0_SetCompareOutputMode( Pwm0_Disconnected );
    }
    else
  #endif

  #if CORE_PWM_COUNT >= 2
    if ( pin == CORE_PWM1_PIN )
    {
      Pwm1_SetCompareOutputMode( Pwm1_Disconnected );
    }
    else
  #endif

  #if CORE_PWM_COUNT >= 3
    if ( pin == CORE_PWM2_PIN )
    {
      Pwm2_SetCompareOutputMode( Pwm2_Disconnected );
    }
    else
  #endif

  #if CORE_PWM_COUNT >= 4
    if ( pin == CORE_PWM3_PIN )
    {
      Pwm3_SetCompareOutputMode( Pwm3_Disconnected );
    }
    else
  #endif

  #if CORE_PWM_COUNT >= 5
    if ( pin == CORE_PWM4_PIN )
    {
      Pwm4_SetCompareOutputMode( Pwm4_Disconnected );
    }
    else
  #endif

  #if CORE_PWM_COUNT >= 6

    if ( pin == CORE_PWM5_PIN )
    {
      Pwm5_SetCompareOutputMode( Pwm5_Disconnected );
    }
    //else
  #endif

  #if CORE_PWM_COUNT >= 7
  #error Only 6 PWM pins are supported.  Add more conditions.
  #endif
{
    }
}

#if defined(__AVR_ATtinyX41__ ) || defined(__AVR_ATtiny1634__) || defined(__AVR_ATtiny828__)

void digitalWrite(uint8_t pin, uint8_t val)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out, *pue;

  if (port == NOT_A_PIN) return;

  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  turnOffPWM( pin );

  out = portOutputRegister(port);
  pue = portPullupRegister(port);

  if (val == LOW) {
    uint8_t oldSREG = SREG;
    cli();
    *out &= ~bit;
    *pue &= ~bit; //Turn off the pullups
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *out |= bit;
    *pue &= ~bit;
    SREG = oldSREG;
  }
}
#else 

void digitalWrite(uint8_t pin, uint8_t val)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;

  if (port == NOT_A_PIN) return;

  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  turnOffPWM( pin );

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
#endif

int digitalRead(uint8_t pin)
{
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);

	if (port == NOT_A_PIN) return LOW;

	// If the pin that support PWM output, we need to turn it off
	// before getting a digital reading.
  turnOffPWM( pin );

	if (*portInputRegister(port) & bit) return HIGH;
	return LOW;
}
