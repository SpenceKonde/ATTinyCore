/*
  pins_arduino.c - pin definitions for the Arduino board
  Part of Arduino / Wiring Lite

  Copyright (c) 2005 David A. Mellis

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

  $Id: pins_arduino.c 565 2009-03-25 10:50:00Z dmellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 09-10-2009 for attiny45 A.Saporetti
  Modified for Atmel ATTiny2313 mcu by René Bohne
  Corrected 17-05-2010 for ATtiny84 B.Cook ...

    The default analog_reference leaves chip pin 13 (digital pin 10; PA0) 
    unconnected.  So the pin can be set to a non-floating state and so the 
    pin can be used as another digital pin, support for digital pin 10 was 
    added.
 Added Tiny841 28-02-2015 Spence Konde
*/

#include <avr/io.h>
#include "pins_arduino.h"
#include "wiring_private.h"


#if defined( __AVR_ATtiny1634__ )
// ATMEL ATTINY1634
//
//                   +-\/-+
// TX   (D  0) PB0  1|    |20  PB1 (D  16)
// RX   (D  1) PA7  2|    |19  PB2 (D  15)
//    * (D  2) PA6  3|    |18  PB3 (D  14) *
//    * (D  3) PA5  4|    |17  PC0 (D  13) *
//      (D  4) PA4  5|    |16  PC1 (D  12)
//      (D  5) PA4  6|    |15  PC2 (D  11)
//      (D  6) PA3  7|    |14  PC3/RESET (D 17)
//      (D  7) PA2  8|    |13  PC4 (D  10)
//      (D  8) PA0  9|    |12  PC5 (D  9)
//             GND 10|    |11  VCC
//                   +----+
//
// Reminder: Pins 6 - 1 & 20 - 15 are ADC pins
// * indicates PWM port

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] = 
{
	NOT_A_PORT,
	&DDRA,
	&DDRB,
	&DDRC,
};

const uint8_t PROGMEM port_to_output_PGM[] = 
{
	NOT_A_PORT,
	&PORTA,
	&PORTB,
	&PORTC,
};

const uint8_t PROGMEM port_to_pullup_PGM[] = 
{
	NOT_A_PORT,
	&PUEA,
	&PUEB,
	&PUEC,
};

const uint8_t PROGMEM port_to_input_PGM[] = 
{
	NOT_A_PORT,
	&PINA,
	&PINB,
	&PINC,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
	PORT_B_ID, /* 0 */
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID, /* 8 */
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_B_ID, /* 14 */
	PORT_B_ID,
	PORT_B_ID,
    PORT_C_ID, /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
	_BV(0), /* 0 */
	_BV(7),
	_BV(6),
	_BV(5),
	_BV(4),
	_BV(3),
	_BV(2),
	_BV(1),
	_BV(0), /* 8 */
	_BV(5),
	_BV(4),
	_BV(2),
	_BV(1),
	_BV(0),
	_BV(3), /* 14 */
	_BV(2),
	_BV(1),
    _BV(3), /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
{
	NOT_ON_TIMER, /* 0 */
	NOT_ON_TIMER,
	TIMER1B,
    TIMER0B,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER0A,
	TIMER1A,      /* 14 */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 17 = RESET */
};



#endif

#if defined( __AVR_ATtinyX41__ )
// ATMEL ATTINY841 / ARDUINO
//
//                                 +-\/-+
//                           VCC  1|    |14  GND
//  ADC11            (D  0)  PB0  2|    |13  AREF (D 10)              ADC0
//  ADC10      _____ (D  1)  PB1  3|    |12  PA1  (D  9)  TX0         ADC1
//  ADC9       RESET (D 11)  PB3  4|    |11  PA2  (D  8)  RX0         ADC2
//  ADC8  PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7)        PWM   ADC3
//  ADC7  PWM        (D  3)  PA7  6|    |9   PA4  (D  6)  RX1   PWM   ADC4
//  ADC6  PWM        (D  4)  PA6  7|    |8   PA5  (D  5)  TX1   PWM   ADC5
//                                 +----+
//
// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)

const uint8_t PROGMEM port_to_mode_PGM[] = 
{
  NOT_A_PORT,
  &DDRA,
  &DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] = 
{
  NOT_A_PORT,
  &PORTA,
  &PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] = 
{
  NOT_A_PORT,
  &PINA,
  &PINB,
};

const uint8_t PROGMEM port_to_pullup_PGM[] = 
{
 NOT_A_PORT,
 &PUEA,
 &PUEB,
};

const uint8_t PROGMEM port_to_pcmask_PGM[] = 
{
  NOT_A_PORT,
  &PCMSK0,
  &PCMSK1,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
  PORT_B_ID, /* 0 */
  PORT_B_ID,
  PORT_B_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID, /* 8 */
  PORT_A_ID,
  PORT_A_ID,
  PORT_B_ID,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(7), /* 3 port A */
  _BV(6),
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2), 
  _BV(1),
  _BV(0),
  _BV(3), //reset
};

//const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
//{
//  NOT_ON_TIMER,
//  NOT_ON_TIMER,
//  TIMER2A, /* TOCC7 */
//  TIMER2B, /* TOCC6 */
//  TIMER1A, /* TOCC5 */
//  TIMER0B, /* TOCC4 - this is shared with serial 1*/
//  TIMER0A, /* TOCC3 - this is shared with serial 1 so let's give it the least desirable timer */
//  TIMER1B, /* TOCC2 */
//  NOT_ON_TIMER, //This could have pwm, but it's serial 0, and we only get PWM on 6 of the 8 pins at once.
//  NOT_ON_TIMER, //see above.
//  NOT_ON_TIMER,
//  NOT_ON_TIMER,
//};

#endif



#if defined( __AVR_ATtiny828__ )
// ATMEL ATTINY828
// 
//             16*   26   24   14
//          17    27   25   15
//             PC0  PD2  PD0  PB6
//          PC1  PD3  PD1   PB7
//             _________________
// 18 RX  PC2 | *               | PB5   13
// 19 TX  PC3 |                 | PB4   12
// 20 *   PC4 |                 | PB3   11
//        VCC |                 | GND
//        GND |                 | PB2   10
// 21 *   PC5 |                 | PB1    9
// 22 *   PC6 |                 | AVCC
// 23     PC7 |_________________| PB0    8
//           PA0  PA2  PA4  PA6 
//              PA1  PA3  PA5  PA7
//            0     2    4    6
//               1     3    5    7

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] = 
{
	NOT_A_PORT,
	&DDRA,
	&DDRB,
	&DDRC,
	&DDRD
};

const uint8_t PROGMEM port_to_output_PGM[] = 
{
	NOT_A_PORT,
	&PORTA,
	&PORTB,
	&PORTC,
	&PORTD
};

const uint8_t PROGMEM port_to_pullup_PGM[] = 
{
	NOT_A_PORT,
	&PUEA,
	&PUEB,
	&PUEC,
	&PUED
};

const uint8_t PROGMEM port_to_input_PGM[] = 
{
	NOT_A_PORT,
	&PINA,
	&PINB,
	&PINC,
	&PIND
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_C_ID,
	PORT_D_ID,
	PORT_D_ID,
	PORT_D_ID,
	PORT_D_ID
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
	_BV(0), 
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), 
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), 
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), 
	_BV(1),
	_BV(2),
	_BV(3),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
{
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER0A,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	TIMER0B,
	TIMER1A,
	TIMER1B,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	NOT_ON_TIMER
};



#endif
