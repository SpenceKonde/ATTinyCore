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
#define PINMAPPING_MHTINY
#define USE_SOFTWARE_SPI 0

#include <avr/pgmspace.h>



#define ADC_TEMPERATURE 8

#define NUM_DIGITAL_PINS            27
#define NUM_ANALOG_INPUTS           8
#define analogInputToDigitalPin(p)  ((p < 8) ? ((p < 6) ? (p) + 11 :(p) + 19 ): -1)

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
#define USE_SOFTWARE_SERIAL           1
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRD
#define ANALOG_COMP_PORT              PORTD
#define ANALOG_COMP_PIN               PIND
#define ANALOG_COMP_AIN0_BIT          6
#define ANALOG_COMP_AIN1_BIT          7


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

static const uint8_t SDA = 23;
static const uint8_t SCL = 24;
static const uint8_t LED_BUILTIN = 0;

static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;
static const uint8_t A4 = 0x80 | 4;
static const uint8_t A5 = 0x80 | 5;
static const uint8_t A6 = 0x80 | 6;
static const uint8_t A7 = 0x80 | 7;

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
#define PIN_PB2  ( 10)
#define PIN_PB3  ( 11)
#define PIN_PB4  ( 12)
#define PIN_PB5  ( 13)
#define PIN_PB7  ( 14)
#define PIN_PA0  ( 15)
#define PIN_PA1  ( 16)
#define PIN_PA2  ( 17)
#define PIN_PA3  ( 18)
#define PIN_PC0  ( 19)
#define PIN_PC1  ( 20)
#define PIN_PC2  ( 21)
#define PIN_PC3  ( 22)
#define PIN_PC4  ( 23)
#define PIN_PC5  ( 24)
#define PIN_PC7  ( 25)
#define PIN_PC6  ( 26)

#define PIN_A0   (19)
#define PIN_A1   (20)
#define PIN_A2   (21)
#define PIN_A3   (22)
#define PIN_A4   (23)
#define PIN_A5   (24)
#define PIN_A6   (17)
#define PIN_A7   (18)

#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ( ((p) <= 7) ? PCIE2 : ( ((p) <= 14) ? PCIE0 : ( ((p) <= 18) ? PCIE3 : PCIE1 ) ) )
#define digitalPinToPCMSK(p)    ( ((p) <= 7) ? (&PCMSK2) : ( ((p) <= 14) ? (&PCMSK0) : ( ((p) <= 18) ? (&PCMSK3) : (&PCMSK1) ) ) )
#define digitalPinToPCMSKbit(p) ( ((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : (((p) == 14) ? 7 : (((p) <= 16) ? ((p) - 14) : (((p) <= 18) ? ((p) - 17) : (((p) == 25) ? 7 : ((p) - 19) ) ) ) ) ) )

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p)==3?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN


// ATMEL ATTINY88 - note that MH Tiny boards all use TQFP-32 parts.
//
//                   +-\/-+
//      (D26*) PC6  1|    |28  PC5 (A5/D24)
//      (D  0) PD0  2|    |27  PC4 (A4/D23)
//      (D  1) PD1  3|    |26  PC3 (A3/D22)
//      (D  2) PD2  4|    |25  PC2 (A2/D21)
//      (D  3) PD3  5|    |24  PC1 (A1/D20)
//      (D  4) PD4  6|    |23  PC0 (A0/D19)
//             VCC  7|    |22  GND
//             GND  8|    |21  PC7 (D 25)
//      (CLKI) PB6  9|    |20  AVCC
//      (D 14) PB7 10|    |19  PB5 (D 13)
//      (D  5) PD5 11|    |18  PB4 (D 12)
//      (D  6) PD6 12|    |17  PB3 (D 11)
//      (D  7) PD7 13|    |16  PB2 (D 10) PWM
//      (D  8) PB0 14|    |15  PB1 (D  9) PWM
//                   +----+
//
//                    (D 2) (D 0) (D22/ (D20/
//                                 /A5)  /A3)
//                       (D 1) (D27) (D21/ (D19/
//                                    /A4)  /A2)
//                     PD2   PD0   PC5   PC3
//                        PD1   PC6   PC4   PC2
//                     32    30    28    26
//                        31    29    27    25
//                    ┌ ─  ─  ─  ─  ─  ─  ─  ─ ┐
//    (D 3)  PD3   1  |°                       |  24  PC1  (D18/A1)
//    (D 4)  PD4   2  |                        |  23  PC0  (D17/A0)
//    (D25)  PA2   3  |        ATtiny88        |  22  PA1  (D24/A7)
//           VCC   4  |           on           |  21  GND
//           GND   5  |   "MH-ET" or "HW-tiny" |  20  PC7  (D16   )
//    (D26)  PA3   6  |          board         |  19  PA0  (D23/A6)
//    (D14)  PB6   7  |                        |  18  AVCC
//    (D15)  PB7   8  |                        |  17  PB5  (D13   )
//                    └ ─  ─  ─  ─  ─  ─  ─  ─ ┘
//                      9    11    13    15
//                        10    12    14    16
//                     PD5   PD7   PB1   PB3
//                        PD6   PB0   PB2   PB4
//                    (D 5) (D 7) (D 9) (D11)
//                       (D 6) (D 8) (D10) (D12)
//
// * Only available if RSTDSBL fuse programmed, which makes further ISP programming impossible.
//   unlike some ATtiny85-based digispark clones,
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
