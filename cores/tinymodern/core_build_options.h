/*==============================================================================

  core_build_options.h - Various options for mapping functionality to hardware.

  Copyright 2010 Rowdy Dog Software.

  This file is part of Arduino-Tiny.

  Arduino-Tiny is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  Arduino-Tiny is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Arduino-Tiny.  If not, see <http://www.gnu.org/licenses/>.

==============================================================================*/

#ifndef core_build_options_h
#define core_build_options_h


/*=============================================================================
  Low power / smaller code options
=============================================================================*/

#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#define INITIALIZE_SECONDARY_TIMERS               1

/*=============================================================================
  Build options for the ATtiny841 processor
=============================================================================*/

#if defined( __AVR_ATtiny441__ ) || defined( __AVR_ATtiny841__ )
#define __AVR_ATtinyX41__
#endif

#if defined( __AVR_ATtinyX41__ )

/*
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

/*
  Tone goes on whichever timer was not used for millis.
*/
#if TIMER_TO_USE_FOR_MILLIS == 1
#define TIMER_TO_USE_FOR_TONE                     0
#else
#define TIMER_TO_USE_FOR_TONE                     1
#endif

#define HAVE_ADC                                  1

#define DEFAULT_TO_TINY_DEBUG_SERIAL              0


// missing defines (?)
#if !defined(PCINT0)
#define PCINT0 0
#define PCINT1 1
#define PCINT2 2
#define PCINT3 3
#define PCINT4 4
#define PCINT5 5
#define PCINT6 6
#define PCINT7 7
#define PCINT8 0
#define PCINT9 1
#define PCINT10 2
#define PCINT11 3
#endif
#endif

/*=============================================================================
  Build options for the ATtiny1634 processor and other boring ones
=============================================================================*/
#if defined( __AVR_ATtiny1634__ ) || defined(__AVR_ATtiny828__ )
/*
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

/*
  Tone goes on whichever timer was not used for millis.
*/
#if TIMER_TO_USE_FOR_MILLIS == 1
#define TIMER_TO_USE_FOR_TONE                     0
#else
#define TIMER_TO_USE_FOR_TONE                     1
#endif

#define HAVE_ADC                                  1

#define DEFAULT_TO_TINY_DEBUG_SERIAL              0



#endif



/*=============================================================================
  There doesn't seem to be many people using a bootloader so we'll assume
  there isn't one.  If the following is true (non-zero), the timers are
  reinitialized to their power-up state in init just in case the bootloader
  left them in a bad way.
=============================================================================*/

#define HAVE_BOOTLOADER                           0


/*=============================================================================
  Allow the ADC to be optional for low-power applications
=============================================================================*/

#if ! defined( HAVE_ADC )
  #define HAVE_ADC   0
#endif

#if ! HAVE_ADC
  #undef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER  0
#else
  #if ! defined( INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER )
    #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER   1
  #endif
#endif


/*=============================================================================
  Allow the "secondary timers" to be optional for low-power applications
=============================================================================*/

#if ! defined( INITIALIZE_SECONDARY_TIMERS )
  #define INITIALIZE_SECONDARY_TIMERS               1
#endif


#endif
