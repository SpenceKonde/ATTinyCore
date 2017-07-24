/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

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

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#define ATTINYX8 1       //backwards compat
#define __AVR_ATtinyX8__ //recommended
#define USE_SOFTWARE_SPI 0

#include <avr/pgmspace.h>

#define TUNED_OSCCAL_VALUE                        OSCCAL

#define NUM_DIGITAL_PINS            17
#define NUM_ANALOG_INPUTS           8
#define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 17 : -1)

#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10)


//Choosing not to initialise saves power and flash. 1 = initialise.
#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#define INITIALIZE_SECONDARY_TIMERS               1
/*
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0 //Must be timer 0!

/*
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL						  1
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR						 	  DDRD
#define ANALOG_COMP_PORT						  PORTD
#define ANALOG_COMP_PIN						 	  PIND
#define ANALOG_COMP_AIN0_BIT					  6
#define ANALOG_COMP_AIN1_BIT					  7


/*
  Analog reference bit masks.
*/
// AVCC used as analog reference
#define DEFAULT (1)
// Internal 1.1V voltage reference
#define INTERNAL (0)
#define INTERNAL1V1 INTERNAL

//#define ANALOG_PINS_SEPARATE

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 21;
static const uint8_t SCL = 22;
static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;
static const uint8_t A4 = 0x80 | 4;
static const uint8_t A5 = 0x80 | 5;
static const uint8_t A6 = 0x80 | 6;
static const uint8_t A7 = 0x80 | 7;

#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 26) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7) ? 2 : (((p) <= 15) ? 0 : (((p) <= 22) ? 1 : 3)))
#define digitalPinToPCMSK(p)    (((p) <= 7) ? (&PCMSK2) : (((p) <= 15) ? (&PCMSK0) : (((p) <= 22) ? (&PCMSK1) : (((p) <= 26) ? (&PCMSK3) : ((uint8_t *)0)))))
#define digitalPinToPCMSKbit(p) (((p) <= 15) ? ((p) & 0x7) : (((p) == 16) ? (7) : (((p) <= 22) ? ((p) - 17) : ((p) - 23))))


#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p)==3?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY88
//
//                   +-\/-+
//      (D27*) PC6  1|    |28  PC5 (A5/D22)
//      (D  0) PD0  2|    |27  PC4 (A4/D21)
//      (D  1) PD1  3|    |26  PC3 (A3/D20)
//      (D  2) PD2  4|    |25  PC2 (A2/D19)
//      (D  3) PD3  5|    |24  PC1 (A1/D18)
//      (D  4) PD4  6|    |23  PC0 (A0/D17)
//             VCC  7|    |22  GND
//             GND  8|    |21  PC7 (D 16)
//      (D 14) PB6  9|    |20  AVCC
//      (D 15) PB7 10|    |19  PB5 (D 13)
//      (D  5) PD5 11|    |18  PB4 (D 12)
//      (D  6) PD6 12|    |17  PB3 (D 11)
//      (D  7) PD7 13|    |16  PB2 (D 10) PWM
//      (D  8) PB0 14|    |15  PB1 (D  9) PWM
//                  +----+
//  Note: For 32pin Packages, PORTA exists. PA0 = A6/D23, PA1 = A7/D24, PA2 = D25, PA3 = D26
// * Only available if RSTDSBL fuse programmed, which makes further ISP programming impossible.


// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PD, /* 0 */
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PB, /* 8 */
	PB,
	PB,
	PB,
	PB,
	PB,
	PB,
	PB,
	PC, /* 16 */
	PC,
	PC,
	PC,
	PC,
	PC,
	PC,
	PA, /* 23 */
	PA,
	PA,
	PA,
	PC
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), /* 0, port D */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 8, port B */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(7), /* 16, port C */
	_BV(0),
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(0), /* 23, port A */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(6)
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	TIMER1A,
	TIMER1B,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 16 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 23 - port A */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER
};

#endif

#endif
