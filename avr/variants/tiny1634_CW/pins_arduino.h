/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Copyright (c) 2015~2020 Spence Konde

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

#define ATTINYX4 1
#define __AVR_ATtinyX4__
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            12
#define NUM_ANALOG_INPUTS           8
#define analogInputToDigitalPin(p)  ((p < 8) ? 10 -(p): -1)

#define ADC_TEMPERATURE 34

#define digitalPinHasPWM(p)         ((p) == PIN_PA5 || (p) == PIN_PA6 || (p) == PIN_PB3 || (p) == PIN_PC0)
// WACKY pins!

// This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)


#define SS   PIN_PC2  //This one doesn't really matter
#define MOSI PIN_PB2
#define MISO PIN_PB1
#define SCK  PIN_PC1

#define USI_DATA_DDR    DDRB
#define USI_DATA_PORT   PORTB
#define USI_DATA_PIN    PINB

#define USI_CLOCL_DDR   DDRC
#define USI_CLOCK_PORT  PORTC
#define USI_CLOCK_BIT   PINC1
#define USI_DO_BIT      PINB2
#define USI_DI_BIT      PINB1

#define USI_START_VECTOR USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif


static const uint8_t SDA = PIN_PB1;
static const uint8_t SCL = PIN_PC1;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().
static const uint8_t  A0 = 0x80 |  0;
static const uint8_t  A1 = 0x80 |  1;
static const uint8_t  A2 = 0x80 |  2;
static const uint8_t  A3 = 0x80 |  3;
static const uint8_t  A4 = 0x80 |  4;
static const uint8_t  A5 = 0x80 |  5;
static const uint8_t  A6 = 0x80 |  6;
static const uint8_t  A7 = 0x80 |  7;
static const uint8_t  A8 = 0x80 |  8;
static const uint8_t  A9 = 0x80 |  9;
static const uint8_t A10 = 0x80 | 10;
static const uint8_t A11 = 0x80 | 11;


#define PIN_PA0     ( 0)
#define PIN_PA1     ( 1)
#define PIN_PA2     ( 2)
#define PIN_PA3     ( 3)
#define PIN_PA4     ( 4)
#define PIN_PA5     ( 5)
#define PIN_PA6     ( 6)
#define PIN_PA7     ( 7)
#define PIN_PB0     ( 8)
#define PIN_PB1     ( 9)
#define PIN_PB2     (10)
#define PIN_PB3     (11)
#define PIN_PC0     (12)
#define PIN_PC1     (13)
#define PIN_PC2     (14)
#define PIN_PC3     (17)  /* RESET */
#define PIN_PC4     (15)  /* XTAL2 */
#define PIN_PC5     (16)  /* XTAL1 */
#define LED_BUILTIN ( 2)

//legacy
#define PIN_A0      (PIN_PA3)
#define PIN_A1      (PIN_PA4)
#define PIN_A2      (PIN_PA5)
#define PIN_A3      (PIN_PA6)
#define PIN_A4      (PIN_PA7)
#define PIN_A5      (PIN_PB0)
#define PIN_A6      (PIN_PB1)
#define PIN_A7      (PIN_PB2)
#define PIN_A8      (PIN_PB3)
#define PIN_A9      (PIN_PC0)
#define PIN_A10     (PIN_PC1)
#define PIN_A11     (PIN_PC2)

#define PINMAPPING_CW

//----------------------------------------------------------
//----------------------------------------------------------
//Core Configuration (used to be in core_build_options.h)



//Choosing not to initialise saves power and flash. 1 = initialise.
#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#define INITIALIZE_SECONDARY_TIMERS               1
/*
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

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

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------



#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 11) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) >= 3 && (p) <= 10) ? 4 : 5)
#define digitalPinToPCMSK(p)    (((p) >= 3 && (p) <= 10) ? (&PCMSK0) : ((((p) >= 0 && (p) <= 2) || ((p) == 11)) ? (&PCMSK1) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) >= 3 && (p) <= 10) ? (10 - (p)) : (((p) == 11) ? 3 : (p)))

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : NOT_AN_INTERRUPT)


#ifdef ARDUINO_MAIN
#warning "This is the CLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in clockwise order"

// ATMEL ATTINY1634
//
//                  +-\/-+
// TX0   ( 8) PB0  1|a  a|20  PB1 ( 9)   TX1
// RX0   ( 7) PA7  2|a  a|19  PB2 (10)   RX1
//     * ( 6) PA6  3|a  a|18  PB3 (11) *
//     * ( 5) PA5  4|a  a|17  PC0 (12) *
//       ( 4) PA4  5|a  a|16  PC1 (13)
//       ( 3) PA3  6|a  a|15  PC2 (14)
//       ( 2) PA2  7|    |14  PC3/RESET (17)
//       ( 1) PA1  8|   x|13  PC4 (15)
//       ( 0) PA0  9|   x|12  PC5 (16)
//            GND 10|    |11  VCC
//                  +----+
//
// * indicates PWM port
// a indicates ADC pin.
// x indicates XTAL pin

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading and writing)
const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRA,
  (uint8_t)(uint16_t)&DDRB,
  (uint8_t)(uint16_t)&DDRC
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTA,
  (uint8_t)(uint16_t)&PORTB,
  (uint8_t)(uint16_t)&PORTC
};

const uint8_t PROGMEM port_to_pullup_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PUEA,
  (uint8_t)(uint16_t)&PUEB,
  (uint8_t)(uint16_t)&PUEC
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PINA,
  (uint8_t)(uint16_t)&PINB,
  (uint8_t)(uint16_t)&PINC
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PORT_A_ID, /* 0 */
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_B_ID, /* 8 */
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_C_ID, /* 12 */
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID, /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* 8 */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(0), /* 12 */
  _BV(1),
  _BV(2),
  _BV(4), /* SKIP RESET */
  _BV(5),
  _BV(3), /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER, /* 0 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0B,      /* 5 */
  TIMER1B,      /* 6 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1A,      /* 11 */
  TIMER0A,      /* 12 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 17 = RESET */
};


#endif

#endif
