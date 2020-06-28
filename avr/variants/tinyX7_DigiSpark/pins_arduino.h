/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Copyright 2015~2018 Spence Konde

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

#define ATTINYX7 1       //backwards compat
#define __AVR_ATtinyX7__ //recommended

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            14
#define NUM_ANALOG_INPUTS           10
#define analogInputToDigitalPin(p)  (((p == 9) ? 3 : (p == 7) ? 5 : (p < 13 && p > 5) ? (p-6) : (p ==13) ? 13 : -1))
#define ADC_TEMPERATURE 11

#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 1 || (p)==8)


#define SS   12
#define MOSI 10
#define MISO 8
#define SCK  11
#define USI_DDR_PORT DDRB
#define USI_SCK_PORT DDRB
#define USCK_DD_PIN DDA2
#define DO_DD_PIN DDA1
#define DI_DD_PIN DDA0
#define SDA 0
#define SCL 2


#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB0
#  define PORT_USI_SCL PORTB2
#  define PIN_USI_SDA PINB0
#  define PIN_USI_SCL PINB2
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#  define DDR_USI_CL DDR_USI
#  define PORT_USI_CL PORT_USI
#  define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif

static const uint8_t A3 = 0x80|9;
static const uint8_t A5 = 0x80|7;
static const uint8_t A6 = 0x80|0;
static const uint8_t A7 = 0x80|1;
static const uint8_t A8 = 0x80|2;
static const uint8_t A9 = 0x80|3;
static const uint8_t A10 = 0x80|4;
static const uint8_t A11 = 0x80|5;
static const uint8_t A12 = 0x80|6;
static const uint8_t A13 = 0x80|10;



#define PIN_PB0  ( 0 )
#define PIN_PB1  ( 1 )
#define PIN_PB2  ( 2 )
#define PIN_PB6  ( 3 )
#define PIN_PB3  ( 4 )
#define PIN_PA7  ( 5 )
#define PIN_PA0  ( 6 )
#define PIN_PA1  ( 7 )
#define PIN_PA2  ( 8 )
#define PIN_PA3  ( 9 )
#define PIN_PA4  ( 10 )
#define PIN_PA5  ( 11 )
#define PIN_PA6  ( 12 )
#define PIN_PB7  ( 13 )
#define PIN_PB4  ( 14 )
#define PIN_PB5  ( 15 )

#define LED_BUILTIN (PIN_PB1)

#define Serial1 Serial

#define PINMAPPING_DIGI

//----------------------------------------------------------
//----------------------------------------------------------
//Core Configuration (used to be in core_build_options.h)

//If Software Serial communications doesn't work, run the TinyTuner sketch provided with the core to give you a calibrated OSCCAL value.
//Change the value here with the tuned value.
//e.g
//#define TUNED_OSCCAL_VALUE                        0x57

//Choosing not to initialise saves power and flash. 1 = initialise.
#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#define INITIALIZE_SECONDARY_TIMERS               1

#define TIMER_TO_USE_FOR_MILLIS                   0

// This is commented out. The only place where HAVE_BOOTLOADER is checked is in wiring.c, where it wastes precious bytes of flash resetting timer-related registers out of fear that the bootloader has scribbled on them.
// However, Optiboot does a WDR before jumping straight to app to start after running.
// This means that optiboot leaves all the registers clean. Meanwhile, Micronucleus doesn't even USE any of the timers, and that's all the wiring.c code checks on (to make sure millis will work)
// commenting out instead of setting to 0, as that would allow a hypothetical badly behaved bootloader to be supported in the future by having it add -DHAVE_BOOTLOADER from boards.txt
// #define HAVE_BOOTLOADER                           1

/*
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL           0
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          6
#define ANALOG_COMP_AIN1_BIT          7

/*
  Analog reference bit masks.
*/
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// External voltage reference at PA0 (AREF) pin, internal reference turned off
#define EXTERNAL (1)
// Internal 1.1V voltage reference
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL
#define INTERNAL2V56 (7)


//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------




#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ( ((p) >= 5 && (p) <= 12) ? PCIE0 : PCIE1 )
#define digitalPinToPCMSK(p)    ( ((p) >= 5 && (p) <= 12) ? (&PCMSK0) : (&PCMSK1) )
#define digitalPinToPCMSKbit(p) ( (((p) >= 0) && ((p) <= 2))  ? (p) :       \
                                ( (((p) >= 6) && ((p) <= 13)) ? ((p) - 6) : \
                                ( ((p) == 3) ? 6 :                          \
                                ( ((p) == 4) ? 3 :                          \
                                ( 7) ) ) ) ) /* pin 5 */


#define digitalPinToInterrupt(p)  ((p) == PIN_PB6 ? 0 : ((p)==PIN_PA3?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the DigiSpark board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY167
//
//                   +-\/-+
// RX   (D  6) PA0  1|    |20  PB0 (D  0)
// TX   (D  7) PA1  2|    |19  PB1 (D  1)
//     *(D  8) PA2  3|    |18  PB2 (D  2)
//      (D  9) PA3  4|    |17  PB3 (D  4)*
//            AVCC  5|    |16  GND
//            AGND  6|    |15  VCC
// INT1 (D 10) PA4  7|    |14  PB4 (D 14) XTAL1
//      (D 11) PA5  8|    |13  PB5 (D 15) XTAL2
//      (D 12) PA6  9|    |12  PB6 (D  3)* INT0
//      (D  5) PA7 10|    |11  PB7 (D 13)
//                   +----+
//
// * indicates PWM pin.

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
  PB, /* 0 */
  PB,
  PB, /* 2 */
  PB, /* 3 */
  PB, /* 4 */
  PA,
  PA,
  PA,
  PA,
  PA,
  PA, /* 10 */
  PA,
  PA,
  PB, /* RESET */
  PB,
  PB,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(2), /* 2 */
  _BV(6), /* 3 */
  _BV(3), /* 4 */
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4), /* 10 */
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(4),
  _BV(5),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  TIM1AU,
  TIM1BU,
  TIM1AV,
  TIM1AX,
  TIM1BV,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIM1BX,
  TIM1AW,
  TIM1BW,
};

#endif

#endif
