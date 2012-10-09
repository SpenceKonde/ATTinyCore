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

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           11
#define analogInputToDigitalPin(p)  ((p < 8) ? 10 -(p): -1)

#define digitalPinHasPWM(p)         ((p) == 2 || (p) == 3 || (p) == 4 || (p) == 5)

#define SS   7
#define MOSI 5
#define MISO 4
#define SCK  6

static const uint8_t SDA = 4;
static const uint8_t SCL = 6;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().
static const uint8_t A0 = NUM_DIGITAL_PINS;
static const uint8_t A1 = NUM_DIGITAL_PINS+1;
static const uint8_t A2 = NUM_DIGITAL_PINS+2;
static const uint8_t A3 = NUM_DIGITAL_PINS+3;
static const uint8_t A4 = NUM_DIGITAL_PINS+4;
static const uint8_t A5 = NUM_DIGITAL_PINS+5;
static const uint8_t A6 = NUM_DIGITAL_PINS+6;
static const uint8_t A7 = NUM_DIGITAL_PINS+7;

//----------------------------------------------------------
//----------------------------------------------------------
//Core Configuration (used to be in core_build_options.h)

//If Software Serial communications doesn't work, run the TinyTuner sketch provided with the core to give you a calibrated OSCCAL value.
//Change the value here with the tuned value. By default this option uses the default value which the compiler will optimise out. 
#define TUNED_OSCCAL_VALUE                        OSCCAL
//e.g
//#define TUNED_OSCCAL_VALUE                        0x57

//Choosing not to initialise saves power and flash. 1 = initialise.
#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#define INITIALIZE_SECONDARY_TIMERS               1
/*
  For various reasons, Timer 1 is a better choice for the millis timer on the
  '85 processor.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

/*
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL						  1
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR						 	  DDRA
#define ANALOG_COMP_PORT						  PORTA
#define ANALOG_COMP_PIN						 	  PINA
#define ANALOG_COMP_AIN0_BIT					  6
#define ANALOG_COMP_AIN1_BIT					  7

/*
  Analog reference bit masks.
*/
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// External voltage reference at PA0 (AREF) pin, internal reference turned off
#define EXTERNAL (1)
// Internal 1.1V voltage reference
#define INTERNAL (2)


//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------



#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 10) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) >= 3 && (p) <= 10) ? 4 : 5)
#define digitalPinToPCMSK(p)    (((p) >= 3 && (p) <= 10) ? (&PCMSK0) : (((p) >= 0 && (p) <= 2) ? (&PCMSK1) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) >= 3 && (p) <= 10) ? (10 - (p)) : (p))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY167
//
//                   +-\/-+
// RX   (D  0) PA0  1|    |20  PB0 (D  4)**
// TX   (D  1) PA1  2|    |19  PB1 (D  5)***
//     *(D 12) PA2  3|    |18  PB2 (D  6)**
//      (D  3) PA3  4|    |17  PB3 (D  7)***
//            AVCC  5|    |16  GND
//            AGND  6|    |15  VCC
// INT1 (D 11) PA4  7|    |14  PB4 (D  8)**
//      (D 13) PA5  8|    |13  PB5 (D  9)***
//      (D 10) PA6  9|    |12  PB6 (D  2)**
//      (D 14) PA7 10|    |11  PB7 (D 15)***
//                   +----+
//
// * indicates PWM0 pin. **pick one for PWM1. ***pick one for PWM2. 
// For PWM1 and PWM2, setting one as PWM will disable the others as PWM (it is possible to repeatedly swap between them)

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

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
	PORT_A_ID, /* 0 */
	PORT_A_ID,
	PORT_B_ID, /* 2 */
	PORT_A_ID, /* 3 */
	PORT_B_ID, /* 4 */
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_B_ID,
	PORT_A_ID, /* 10 */
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_A_ID,
	PORT_B_ID, /* 15 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
	_BV(0), /* 0 */
	_BV(1),
	_BV(6), /* 2 */
	_BV(3), /* 3 */
	_BV(0), /* 4 */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6), /* 10 */
	_BV(4),
	_BV(2),
	_BV(5),
	_BV(7),
	_BV(7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
{
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	TIMER1AX,
	NOT_ON_TIMER,
	TIMER1AU,
	TIMER1BU,
	TIMER1AV,
	TIMER1BV,
	TIMER1AW,
	TIMER1BW,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER0A,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER1BX,
};

#endif

#endif




//Old code, just here for temporary backup until I decide it is not needed.
//WARNING, if using software, RX must be on a pin which has a Pin change interrupt <= 7 (e.g. PCINT6, or PCINT1, but not PCINT8)
/*#define USE_SOFTWARE_SERIAL						  1
//These are set to match Optiboot pins.

#define SOFTWARE_SERIAL_PORT 					  PORTB
#define SOFTWARE_SERIAL_TX 						  0
#define SOFTWARE_SERIAL_PIN 					  PINB
#define SOFTWARE_SERIAL_RX 						  1*/