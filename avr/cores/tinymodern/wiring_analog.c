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
#include "core_adc.h"
#include "core_pins.h"
#include "core_timers.h"
#include "PwmTimer.h"

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
  #if defined( CORE_ANALOG_FIRST )
    if ( pin >= CORE_ANALOG_FIRST ) pin -= CORE_ANALOG_FIRST; // allow for channel or pin numbers
  #endif

  // fix? Validate pin?

  ADC_SetVoltageReference( analog_reference );
  ADC_SetInputChannel( pin );

  ADC_StartConversion();

  while( ADC_ConversionInProgress() );

  return( ADC_GetDataRegister() );
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
    #if CORE_PWM_COUNT >= 1
      if ( pin == CORE_PWM0_PIN )
      {
        Pwm0_SetCompareOutputMode( Pwm0_Clear );
        Pwm0_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 2
      if ( pin == CORE_PWM1_PIN )
      {
        Pwm1_SetCompareOutputMode( Pwm1_Clear );
        Pwm1_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 3
      if ( pin == CORE_PWM2_PIN )
      {
        Pwm2_SetCompareOutputMode( Pwm2_Clear );
        Pwm2_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 4
      if ( pin == CORE_PWM3_PIN )
      {
        Pwm3_SetCompareOutputMode( Pwm3_Clear );
        Pwm3_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 5
      if ( pin == CORE_PWM4_PIN )
      {
        Pwm4_SetCompareOutputMode( Pwm4_Clear );
        Pwm4_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 6
      if ( pin == CORE_PWM5_PIN )
      {
        Pwm5_SetCompareOutputMode( Pwm5_Clear );
        Pwm5_SetOutputCompareMatch( val );
      }
      else
    #endif

    #if CORE_PWM_COUNT >= 7
    #error Only 6 PWM pins are supported.  Add more conditions.
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
