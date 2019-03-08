/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/
  Copyright (c) 2007 David A. Mellis
  Copyright 2019 Spence Konde
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

#define ATTINY43 1
#define __AVR_ATtiny43__
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

//FIXED TO HERE - all below is wrong.

#define NUM_DIGITAL_PINS            12
#define NUM_ANALOG_INPUTS           8
#define analogInputToDigitalPin(p)  ((p < 8) ? 10 -(p): -1)

#define ADC_TEMPERATURE 34

#define digitalPinHasPWM(p)         ((p) == 2 || (p) == 3 || (p) == 4 || (p) == 5)

//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)


#define SS   7
#define MOSI 5
#define MISO 4
#define SCK  6

#define USI_DDR_PORT DDBA
#define USI_SCK_PORT DDBA
#define USCK_DD_PIN DDB6
#define DO_DD_PIN DDB5
#define DI_DD_PIN DDB4
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB4
#  define PORT_USI_SCL PORTB6
#  define PIN_USI_SDA PINB4
#  define PIN_USI_SCL PINB6
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#  define DDR_USI_CL DDR_USI
#  define PORT_USI_CL PORT_USI
#  define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif


static const uint8_t SDA = 4;
static const uint8_t SCL = 6;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().
static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;

#define PIN_A0  ( 8)
#define PIN_A1  ( 9)
#define PIN_A2  (10)
#define PIN_A3  (11)
#define PIN_A4  (12)
#define PIN_A5  (13)
#define PIN_A6  (14)
#define PIN_A7  (15) /* RESET */
#define PIN_B0  ( 0)
#define PIN_B1  ( 1)
#define PIN_B2  ( 2)
#define PIN_B3  ( 3)
#define PIN_B4  ( 4)
#define PIN_B5  ( 5)
#define PIN_B6  ( 6)
#define PIN_B7  ( 7)
#define LED_BUILTIN (13)

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
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

/*
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL           1
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          4
#define ANALOG_COMP_AIN1_BIT          5


/*
  Analog reference bit masks.
*/
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// Internal 1.1V voltage reference
#define INTERNAL (1)
#define INTERNAL1V1 INTERNAL

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------



#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) >= 0 && (p) <= 7) ? 5 : 4)
#define digitalPinToPCMSK(p)    (((p) >= 0 && (p) <= 7) ? (&PCMSK1) : ((p) <= 15) ? (&PCMSK0) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p) ((p)&0x07)

#define digitalPinToInterrupt(p)  ((p) == 7 ? 0 : NOT_AN_INTERRUPT)
#ifdef ARDUINO_MAIN

// ATMEL ATTINY43 / ARDUINO
//
//                           +-\/-+
//             (D  0)  PB0  1|    |13  PA7  (D 15)        RESET
//  PWM        (D  1)  PB1  2|    |12  PA6  (D 14)
//  PWM        (D  2)  PB3  3|    |11  PA5  (D 13)
//             (D  3)  PB2  4|    |10  PA4  (D 12)
//  PWM        (D  4)  PB7  5|    |9   PA3  (D 11)
//  PWM        (D  5)  PB7  6|    |9   PA2  (D 10)
//             (D  6)  PB7  7|    |9   PA1  (D  9)
//       INT0  (D  7)  PB7  8|    |9   PA0  (D  8)
//                     Vcc  9|    |9   VBat
//                    Gnd  10|    |8   LSW
//                           +----+

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
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7)
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  TIMER0A, /* OC0A */
  TIMER0B, /* OC0B */
  NOT_ON_TIMER,
  TIMER1A, /* OC1A */
  TIMER1B, /* OC1B */
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
