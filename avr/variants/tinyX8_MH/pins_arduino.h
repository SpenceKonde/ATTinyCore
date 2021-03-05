/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Copyright (c) 2020 Spence Konde

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

#include <avr/pgmspace.h>


#define NUM_DIGITAL_PINS            27
#define NUM_ANALOG_INPUTS           8


#define PIN_PD0  ( 0)
#define PIN_PD1  ( 1)
#define PIN_PD2  ( 2)
#define PIN_PD3  ( 3)
#define PIN_PD4  ( 4)
#define PIN_PD5  ( 5)
#define PIN_PD6  ( 6)
#define PIN_PD7  ( 7)
#define PIN_PB0  ( 8)
#define PIN_PB1  ( 9)
#define PIN_PB2  (10)
#define PIN_PB3  (11)
#define PIN_PB4  (12)
#define PIN_PB5  (13)
#define PIN_PB7  (14)
#define PIN_PA0  (15)
#define PIN_PA1  (16)
#define PIN_PA2  (17)
#define PIN_PA3  (18)
#define PIN_PC0  (19)
#define PIN_PC1  (20)
#define PIN_PC2  (21)
#define PIN_PC3  (22)
#define PIN_PC4  (23)
#define PIN_PC5  (24)
#define PIN_PC7  (25)
#define PIN_PC6  (26)

#define LED_BUILTIN PIN_PD0

#define PIN_A0   (PIN_PC0)
#define PIN_A1   (PIN_PC1)
#define PIN_A2   (PIN_PC2)
#define PIN_A3   (PIN_PC3)
#define PIN_A4   (PIN_PC4)
#define PIN_A5   (PIN_PC5)
#define PIN_A6   (PIN_PA0)
#define PIN_A7   (PIN_PA1)

static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);
static const uint8_t A4 = ADC_CH(4);
static const uint8_t A5 = ADC_CH(5);
static const uint8_t A6 = ADC_CH(6);
static const uint8_t A7 = ADC_CH(7);

#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ( ((p) <= 7) ? PCIE2 : ( ((p) <= 14) ? PCIE0 : ( ((p) <= 18) ? PCIE3 : PCIE1 ) ) )
#define digitalPinToPCMSK(p)    ( ((p) <= 7) ? (&PCMSK2) : ( ((p) <= 14) ? (&PCMSK0) : ( ((p) <= 18) ? (&PCMSK3) : (&PCMSK1) ) ) )
#define digitalPinToPCMSKbit(p) ( ((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : (((p) == 14) ? 7 : (((p) <= 16) ? ((p) - 14) : (((p) <= 18) ? ((p) - 17) : (((p) == 25) ? 7 : ((p) - 19) ) ) ) ) ) )

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p)==3?1: NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  ((p < 8) ? ((p < 6) ? (p) + 11 :(p) + 19 ): -1)

#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10)

#define PINMAPPING_MHTINY
//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

// Choosing not to initialise saves power and flash. 1 = initialise.
// #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
// #define INITIALIZE_SECONDARY_TIMERS               1
// #define TIMER_TO_USE_FOR_MILLIS                   0

// No hardware serial port, so Serial will be software serial
// using comparator pins, TX is on AIN0, RX is on AIN1.
#define USE_SOFTWARE_SERIAL           1

/*----------------------------------------------------------
 * Chip Features - Analog stuff
 *----------------------------------------------------------*/

// These are commented out because they are the default option
// That makes it easier to see when something that matters
// is being set - otherwise te exceptions get lost in the
// noise.                                            1 = initialise.
// #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
// #define INITIALIZE_SECONDARY_TIMERS               1
// #define TIMER_TO_USE_FOR_MILLIS                   0


//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRD
#define ANALOG_COMP_PORT              PORTD
#define ANALOG_COMP_PIN               PIND
#define ANALOG_COMP_AIN0_BIT          6
#define ANALOG_COMP_AIN1_BIT          7

/*  Analog reference bit masks. */
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (1)
// Internal 1.1V voltage reference
#define INTERNAL (1)
#define INTERNAL1V1 INTERNAL

/*----------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *----------------------------------------------------------*/


/*  This part has a real SPI module and a real master/slave TWI module
 * You may not get decen on-chip peripherals, but you can at least effectively talk to off-chip ones!@"
 * Seems like it's meant to be a dropin
 */


//#define USE_SOFTWARE_SPI 0


#define SS            PIN_PB2
#define MOSI          PIN_PB3
#define MISO          PIN_PB4
#define SCK           PIN_PB5

#define SDA           PIN_PC4
#define SCL           PIN_PC5


#ifdef ARDUINO_MAIN


// ATMEL ATTINY88
//
//                     ( 2)  ( 0)  (22/  (20/
//                                 /A5)  /A3)
//                        ( 1)  (27)  (21/  (19/
//                                    /A4)  /A2)
//                     PD2   PD0   PC5   PC3
//                        PD1   PC6   PC4   PC2
//                     32    30    28    26
//                        31    29    27    25
//                    ┌ ─  ─  ─  ─  ─  ─  ─  ─ ┐
//     ( 3)  PD3   1  |°                       |  24  PC1   (18/A1)
//     ( 4)  PD4   2  |                        |  23  PC0   (17/A0)
//     (17)  PA2   3  |        ATtiny88        |  22  PA1   (16/A7)
//           VCC   4  |           on           |  21  GND
//           GND   5  |   "MH-ET" or "HW-tiny" |  20  PC7   (16)
//     (18)  PA3   6  |          board         |  19  PA0   (15/A6)
//    CLKIN  PB6   7  |                        |  18  AVCC
//     (14)  PB7   8  |                        |  17  PB5   (13)
//                    └ ─  ─  ─  ─  ─  ─  ─  ─ ┘
//                      9    11    13    15
//                        10    12    14    16
//                     PD5   PD7   PB1   PB3
//                        PD6   PB0   PB2   PB4
//                     ( 5)  ( 7)  ( 9)  (11)
//                        ( 6)  ( 8)  (10)  (12)
//
// * Only available if RSTDSBL fuse programmed, which makes further ISP programming impossible.
//   unlike some ATtiny85-based digispark clones,


const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &DDRA,
  (uint16_t) &DDRB,
  (uint16_t) &DDRC,
  (uint16_t) &DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &PORTA,
  (uint16_t) &PORTB,
  (uint16_t) &PORTC,
  (uint16_t) &PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &PINA,
  (uint16_t) &PINB,
  (uint16_t) &PINC,
  (uint16_t) &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
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
       /* no PB6 */
  PB,
  PA,  /* 15 */
  PA,
  PA,
  PA,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0),  /* 0, port D */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),  /* 8, port B */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
           /* no PB6 */
  _BV(7),
  _BV(2),
  _BV(3),
  _BV(0),
  _BV(1),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(7),
  _BV(6)
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
  TIMER1A,
  TIMER1B,
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
  NOT_ON_TIMER
};

#endif

#endif
