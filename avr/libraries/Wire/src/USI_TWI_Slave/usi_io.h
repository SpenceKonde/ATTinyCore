/*
  usi_io.h - definitions for TWI/I2C via USI
  Copyright (c) 2017 Puuu.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef USI_IO_h
#define USI_IO_h

#include <avr/io.h>

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) \
    || defined(__AVR_ATtiny84__)
#  define DDR_USI DDRA
#  define PORT_USI PORTA
#  define PIN_USI PINA
#  define PORT_USI_SDA PORTA6
#  define PORT_USI_SCL PORTA4
#  define PIN_USI_SDA PINA6
#  define PIN_USI_SCL PINA4
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) \
    || defined(__AVR_ATtiny85__) \
    || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB0
#  define PORT_USI_SCL PORTB2
#  define PIN_USI_SDA PINB0
#  define PIN_USI_SCL PINB2
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATtiny26__)
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB0
#  define PORT_USI_SCL PORTB2
#  define PIN_USI_SDA PINB0
#  define PIN_USI_SCL PINB2
#  define USI_START_VECTOR USI_STRT_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATtiny2313__) || defined(__AVR_ATtiny4313__)
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB5
#  define PORT_USI_SCL PORTB7
#  define PIN_USI_SDA PINB5
#  define PIN_USI_SCL PINB7
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#endif
#if defined( __AVR_ATtiny261__ ) || defined( __AVR_ATtiny461__ ) \
    || defined( __AVR_ATtiny861__ )
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PB0
#  define PORT_USI_SCL PB2
#  define PIN_USI_SDA PINB0
#  define PIN_USI_SCL PINB2
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATtiny43U__)
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB4
#  define PORT_USI_SCL PORTB6
#  define PIN_USI_SDA PINB4
#  define PIN_USI_SCL PINB6
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATtiny1634__)
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define DDR_USI_CL DDRC
#  define PORT_USI_CL PORTC
#  define PIN_USI_CL PINC
#  define PORT_USI_SDA PORTB1
#  define PORT_USI_SCL PORTC1
#  define PIN_USI_SDA PINB1
#  define PIN_USI_SCL PINC1
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
#if defined(__AVR_ATmega165__) || defined(__AVR_ATmega165P__) \
    || defined(__AVR_ATmega165PA__) \
    || defined(__AVR_ATmega169__) || defined(__AVR_ATmega169P__) \
    || defined(__AVR_ATmega169PA__) \
    || defined(__AVR_ATmega645__) || defined(__AVR_ATmega6450__) \
    || defined(__AVR_ATmega649__) || defined(__AVR_ATmega6490__) \
    || defined(__AVR_ATmega649P__) \
    || defined(__AVR_ATmega325__) || defined(__AVR_ATmega325PA__) \
    || defined(__AVR_ATmega3250__) || defined(__AVR_ATmega3250PA__) \
    || defined(__AVR_ATmega329__) || defined(__AVR_ATmega329P__) \
    || defined(__AVR_ATmega3290PA__) || defined(__AVR_ATmega3290__)
#  define DDR_USI DDRE
#  define PORT_USI PORTE
#  define PIN_USI PINE
#  define PORT_USI_SDA PORTE5
#  define PORT_USI_SCL PORTE4
#  define PIN_USI_SDA PINE5
#  define PIN_USI_SCL PINE4
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#endif

#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif

#ifndef DDR_USI_CL
#  define DDR_USI_CL DDR_USI
#  define PORT_USI_CL PORT_USI
#  define PIN_USI_CL PIN_USI
#endif

#endif
