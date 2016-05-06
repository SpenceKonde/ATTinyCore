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

#define ATTINYX61 1  //backwards compatibility
#define __AVR_ATtinyX61__ //recommended
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           64
#define analogInputToDigitalPin(p)  ((p < 3) ? (p): (((p) >= 3 && (p) <= 6) ? ((p) + 7) : (((p) >= 7 && (p) <= 9) ? (12 - (p)) : -1)))

#define digitalPinHasPWM(p)          ((p) == 4 || (p) == 6 || (p) == 8)

#define SS   6
#define MOSI 9
#define MISO 8
#define SCK  7

static const uint8_t SDA = 0;
static const uint8_t SCL = 2;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().
static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;
static const uint8_t A8 = 8;
static const uint8_t A9 = 9;
static const uint8_t A10 = 10;

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
  Timer 0, otherwise there will be no PWM pins.
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
// X 0 0 VCC used as Voltage Reference, disconnected from PB0 (AREF).
#define DEFAULT (0)
// X 0 1 External Voltage Reference at PB0 (AREF) pin, Internal Voltage Reference turned off.
#define EXTERNAL (1)
// 0 1 0 Internal 1.1V Voltage Reference.
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL
// 1 1 1 Internal 2.56V Voltage Reference with external bypass capacitor at PB0 (AREF) pin(1).
#define INTERNAL2V56 (7)
// An alternative for INTERNAL2V56 is (6) ...
// 1 1 0 Internal 2.56V Voltage Reference without external bypass capacitor, disconnected from PB0 (AREF)(1).


//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------



#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) ((((p) >= 0 && (p) <= 2) || ((p) >= 10 && (p) <= 14)) ? 4 : 5)
#define digitalPinToPCMSK(p)    ((((p) >= 0 && (p) <= 2) || ((p) >= 10 && (p) <= 14)) ? (&PCMSK0) : ((((p) >= 3 && (p) <= 9) || ((p) == 15)) ? (&PCMSK1) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) >= 0 && (p) <= 2) ? (p) :(((p) >= 10 && (p) <= 13) ? ((p) - 6) : (((p) == 14) ? (3) : (((p) >= 3 && (p) <= 9) ? (9 - (p)) : (7)))))


#define digitalPinToInterrupt(p)  ((p) == 3 ? 0 : ((p)==2?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY861
//
//                   +-\/-+
//      (D  9) PB0  1|    |20  PA0 (D  0)
//     *(D  8) PB1  2|    |19  PA1 (D  1)
//      (D  7) PB2  3|    |18  PA2 (D  2) INT1
//     *(D  6) PB3  4|    |17  PA3 (D 14)
//             VCC  5|    |16  AGND
//             GND  6|    |15  AVCC
//      (D  5) PB4  7|    |14  PA4 (D 10)
//     *(D  4) PB5  8|    |13  PA5 (D 11)
// INT0 (D  3) PB6  9|    |12  PA6 (D 12)
//      (D 15) PB7 10|    |11  PA7 (D 13)
//                   +----+
//

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&DDRA,
	(uint16_t)&DDRB,
};

const uint16_t PROGMEM port_to_output_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&PORTA,
	(uint16_t)&PORTB,
};

const uint16_t PROGMEM port_to_input_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&PINA,
	(uint16_t)&PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
	PA, /* 0 */
	PA,
	PA,
	PB, /* 3 */
	PB,
	PB,
	PB,
	PB,
	PB,
	PB,
	PA, /* 10 */
	PA,
	PA,
	PA,
	PA,
	PB, /* 15 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
	_BV(0), /* 0 */
	_BV(1),
	_BV(2), 
	_BV(6), /* 3 */
	_BV(5), 
	_BV(4),
	_BV(3),
	_BV(2),
	_BV(1),
	_BV(0),
	_BV(4), /* 10 */
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(3),
	_BV(7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
{
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER1D,
	NOT_ON_TIMER,
	TIMER1B,
	NOT_ON_TIMER,
	TIMER1A,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
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
