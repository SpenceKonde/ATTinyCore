/*
  wiring_private.h - Internal header file.
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

  $Id: wiring.h 239 2007-01-12 17:58:39Z mellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
*/

#ifndef WiringPrivate_h
#define WiringPrivate_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdarg.h>

#include "wiring.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#if defined( EXT_INT0_vect )
  #define EXTERNAL_INTERRUPT_0_vect EXT_INT0_vect
#elif defined( INT0_vect )
  #define EXTERNAL_INTERRUPT_0_vect INT0_vect
#endif

#if defined( EXT_INT1_vect )
  #define EXTERNAL_INTERRUPT_1_vect EXT_INT1_vect
#elif defined( INT1_vect )
  #define EXTERNAL_INTERRUPT_1_vect INT1_vect
#endif

#if defined( EXT_INT2_vect )
  #define EXTERNAL_INTERRUPT_2_vect EXT_INT2_vect
#elif defined( INT2_vect )
  #define EXTERNAL_INTERRUPT_2_vect INT2_vect
#endif

#if defined( EXT_INT3_vect )
  #define EXTERNAL_INTERRUPT_3_vect EXT_INT3_vect
#elif defined( INT3_vect )
  #define EXTERNAL_INTERRUPT_3_vect INT3_vect
#endif

#if defined( EXT_INT4_vect )
  #define EXTERNAL_INTERRUPT_4_vect EXT_INT4_vect
#elif defined( INT4_vect )
  #define EXTERNAL_INTERRUPT_4_vect INT4_vect
#endif

#if defined( EXT_INT5_vect )
  #define EXTERNAL_INTERRUPT_5_vect EXT_INT5_vect
#elif defined( INT5_vect )
  #define EXTERNAL_INTERRUPT_5_vect INT5_vect
#endif

#if defined( EXT_INT6_vect )
  #define EXTERNAL_INTERRUPT_6_vect EXT_INT6_vect
#elif defined( INT6_vect )
  #define EXTERNAL_INTERRUPT_6_vect INT6_vect
#endif

#if defined( EXT_INT7_vect )
  #define EXTERNAL_INTERRUPT_7_vect EXT_INT7_vect
#elif defined( INT7_vect )
  #define EXTERNAL_INTERRUPT_7_vect INT7_vect
#endif

#if defined( EXT_INT8_vect )
  #define EXTERNAL_INTERRUPT_8_vect EXT_INT8_vect
#elif defined( INT8_vect )
  #define EXTERNAL_INTERRUPT_8_vect INT8_vect
#endif

#if defined( EXT_INT9_vect )
  #define EXTERNAL_INTERRUPT_9_vect EXT_INT9_vect
#elif defined( INT9_vect )
  #define EXTERNAL_INTERRUPT_9_vect INT9_vect
#endif

#if defined( EXTERNAL_INTERRUPT_9_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (10)
#elif defined( EXTERNAL_INTERRUPT_8_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (9)
#elif defined( EXTERNAL_INTERRUPT_7_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (8)
#elif defined( EXTERNAL_INTERRUPT_6_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (7)
#elif defined( EXTERNAL_INTERRUPT_5_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (6)
#elif defined( EXTERNAL_INTERRUPT_4_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (5)
#elif defined( EXTERNAL_INTERRUPT_3_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (4)
#elif defined( EXTERNAL_INTERRUPT_2_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (3)
#elif defined( EXTERNAL_INTERRUPT_1_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (2)
#elif defined( EXTERNAL_INTERRUPT_0_vect )
  #define NUMBER_EXTERNAL_INTERRUPTS (1)
#else
  #define NUMBER_EXTERNAL_INTERRUPTS (0)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 1
  #define EXTERNAL_INTERRUPT_0 (0)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 2
  #define EXTERNAL_INTERRUPT_1 (1)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 3
  #define EXTERNAL_INTERRUPT_2 (2)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 4
  #define EXTERNAL_INTERRUPT_3 (3)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 5
  #define EXTERNAL_INTERRUPT_4 (4)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 6
  #define EXTERNAL_INTERRUPT_5 (5)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 7
  #define EXTERNAL_INTERRUPT_6 (6)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 8
  #define EXTERNAL_INTERRUPT_7 (7)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 9
  #define EXTERNAL_INTERRUPT_8 (8)
#endif

#if NUMBER_EXTERNAL_INTERRUPTS >= 10
  #define EXTERNAL_INTERRUPT_9 (9)
#endif

typedef void (*voidFuncPtr)(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
