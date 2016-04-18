/* -*- mode: jde; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the Wiring project - http://wiring.uniandes.edu.co

  Copyright (c) 2004-05 Hernando Barragan

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
  
  Modified 24 November 2006 by David A. Mellis

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 09-10-2009 for attiny45 A.Saporetti
  Modified 20-11-2010 - B.Cook - Correct a minor bug in attachInterrupt
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "WConstants.h"
#include "wiring_private.h"
#include "core_atomic.h"

volatile static voidFuncPtr intFunc[NUMBER_EXTERNAL_INTERRUPTS];

#if defined( MCUCR ) && ! defined( EICRA )
  #define EICRA MCUCR
#endif

#if defined( GIMSK ) && ! defined( EIMSK )
  #define EIMSK GIMSK
#endif

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) 
{
  if ( interruptNum < NUMBER_EXTERNAL_INTERRUPTS ) 
  {
    /* 
      If attachInterrupt is called in succession for the same 
      interruptNum but a different userFunc then the following line 
      is not safe.  Changing intFunc is not atomic.
    intFunc[interruptNum] = userFunc;
    */
    MAKE_ATOMIC( intFunc[interruptNum] = userFunc; )
    
    // Configure the interrupt mode (trigger on low input, any change, rising
    // edge, or falling edge).  The mode constants were chosen to correspond
    // to the configuration bits in the hardware register, so we simply shift
    // the mode into place.
      
    // Enable the interrupt.

    switch ( interruptNum )
    {
      #if NUMBER_EXTERNAL_INTERRUPTS >= 1
        case EXTERNAL_INTERRUPT_0:
          EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
          EIMSK |= (1 << INT0);
          break;
      #endif

      #if NUMBER_EXTERNAL_INTERRUPTS >= 2
        case EXTERNAL_INTERRUPT_1:
          EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
          EIMSK |= (1 << INT1);
          break;
      #endif

      #if NUMBER_EXTERNAL_INTERRUPTS > 2
      #error Add handlers for the additional interrupts.
      #endif
    }
  }
}

void detachInterrupt(uint8_t interruptNum) 
{
  if ( interruptNum < NUMBER_EXTERNAL_INTERRUPTS ) 
  {
    // Disable the interrupt.  (We can't assume that interruptNum is equal
    // to the number of the EIMSK bit to clear, as this isn't true on the 
    // ATmega8.  There, INT0 is 6 and INT1 is 7.)

    switch (interruptNum) 
    {
      #if NUMBER_EXTERNAL_INTERRUPTS >= 1
        case EXTERNAL_INTERRUPT_0:
          EIMSK &= ~(1 << INT0);
          break;;
      #endif

      #if NUMBER_EXTERNAL_INTERRUPTS >= 2
        case EXTERNAL_INTERRUPT_1:
          EIMSK &= ~(1 << INT1);
          break;;
      #endif

      #if NUMBER_EXTERNAL_INTERRUPTS > 2
      #error Add handlers for the additional interrupts.
      #endif
    }
    intFunc[interruptNum] = 0;
  }
}

#if NUMBER_EXTERNAL_INTERRUPTS >= 1
ISR(EXTERNAL_INTERRUPT_0_vect)
{
  if(intFunc[EXTERNAL_INTERRUPT_0])
    intFunc[EXTERNAL_INTERRUPT_0]();
}
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 2
ISR(EXTERNAL_INTERRUPT_1_vect)
{
  if(intFunc[EXTERNAL_INTERRUPT_1])
    intFunc[EXTERNAL_INTERRUPT_1]();
}
#endif

#if NUMBER_EXTERNAL_INTERRUPTS > 2
#error Add handlers for the additional interrupts.
#endif
