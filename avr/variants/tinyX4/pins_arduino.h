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

#define ATTINYX4 1
#define __AVR_ATtinyX4__
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

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

#define USI_DDR_PORT DDRA
#define USI_SCK_PORT DDRA
#define USCK_DD_PIN DDA4
#define DO_DD_PIN DDA5
#define DI_DD_PIN DDA6
#  define DDR_USI DDRA
#  define PORT_USI PORTA
#  define PIN_USI PINA
#  define PORT_USI_SDA PORTA6
#  define PORT_USI_SCL PORTA4
#  define PIN_USI_SDA PINA6
#  define PIN_USI_SCL PINA4
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
static const uint8_t A4 = 0x80 | 4;
static const uint8_t A5 = 0x80 | 5;
static const uint8_t A6 = 0x80 | 6;
static const uint8_t A7 = 0x80 | 7;

#define PIN_A0  (10)
#define PIN_A1  ( 9)
#define PIN_A2  ( 8)
#define PIN_A3  ( 7)
#define PIN_A4  ( 6)
#define PIN_A5  ( 5)
#define PIN_A6  ( 4)
#define PIN_A7  ( 3)
#define PIN_B0  ( 0)
#define PIN_B1  ( 1)
#define PIN_B2  ( 2)
#define PIN_B3  (11)  /* RESET */
#define LED_BUILTIN (2)

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
#define ANALOG_COMP_AIN0_BIT          1
#define ANALOG_COMP_AIN1_BIT          2


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
#warning "This is the COUNTERCLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in counter clockwise order"

// ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D  0)  PB0  2|    |13  PA0  (D 10)        AREF
//             (D  1)  PB1  3|    |12  PA1  (D  9)
//             (D 11)  PB3  4|    |11  PA2  (D  8)
//  PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7)
//  PWM        (D  3)  PA7  6|    |9   PA4  (D  6)
//  PWM        (D  4)  PA6  7|    |8   PA5  (D  5)        PWM
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
  PA,
  PA,
  PA,
  PA,
  PA,
  PA, /* 8 */
  PA,
  PA,
  PB, /* 11 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(7), /* 3 port B */
  _BV(6),
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2),
  _BV(1),
  _BV(0),
  _BV(3),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A, /* OC0A */
  TIMER0B, /* OC0B */
  TIMER1A, /* OC1A */
  TIMER1B, /* OC1B */
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
/*
//WARNING, if using software, RX must be on a pin which has a Pin change interrupt <= 7 (e.g. PCINT6, or PCINT1, but not PCINT8)
#define USE_SOFTWARE_SERIAL             1
//These are set to match Optiboot pins.
#define SOFTWARE_SERIAL_PORT            PORTA
#define SOFTWARE_SERIAL_TX              9
#define SOFTWARE_SERIAL_PIN             PINA
#define SOFTWARE_SERIAL_RX              8*/
