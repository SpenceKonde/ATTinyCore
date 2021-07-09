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

#define ATTINYX61 1  //backwards compatibility
#define __AVR_ATtinyX61__ //recommended
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           11
#define analogInputToDigitalPin(p)  ((p < 3) ? (p): (((p)<7)?((p)+1):((p)<11)?((p)+5):NOT_A_PIN))

#define digitalPinHasPWM(p)          ((p) == 9 || (p) == 11 || (p) == 13)

#define ADC_TEMPERATURE 63

//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)


#define MOSI 9
#define MISO 8
#define SCK  10
#define SS   11

#define USI_DDR_PORT DDRB
#define USI_SCK_PORT DDRB
#define USCK_DD_PIN DDB2
#define DO_DD_PIN DDB1
#define DI_DD_PIN DDB0
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PB0
#  define PORT_USI_SCL PB2
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

static const uint8_t SDA = 8;
static const uint8_t SCL = 10;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().
static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;
static const uint8_t A4 = 0x80 | 4;
static const uint8_t A5 = 0x80 | 5;
static const uint8_t A6 = 0x80 | 6;
static const uint8_t A7 = 0x80 | 7;
static const uint8_t A8 = 0x80 | 8;
static const uint8_t A9 = 0x80 | 9;
static const uint8_t A10 = 0x80 | 10;


#define PIN_PA0  ( 0)
#define PIN_PA1  ( 1)
#define PIN_PA2  ( 2)
#define PIN_PA3  ( 3)
#define PIN_PA4  ( 4)
#define PIN_PA5  ( 5)
#define PIN_PA6  ( 6)
#define PIN_PA7  ( 7)
#define PIN_PB0  ( 8)
#define PIN_PB1  ( 9)
#define PIN_PB2  ( 10)
#define PIN_PB3  ( 11)
#define PIN_PB4  ( 12)
#define PIN_PB5  ( 13)
#define PIN_PB6  ( 14)
#define PIN_PB7  ( 15) /* RESET */


#define LED_BUILTIN (PIN_PB6)

//----------------------------------------------------------
//----------------------------------------------------------
//Core Configuration (used to be in core_build_options.h)



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
#define USE_SOFTWARE_SERIAL           1
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          6
#define ANALOG_COMP_AIN1_BIT          7

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
// 1 1 0 Internal 2.56V Voltage Reference without external bypass capacitor, disconnected from PB0 (AREF)(1).
#define INTERNAL2V56_NO_CAP (6)
#define INTERNAL2V56NOBP INTERNAL2V56_NO_CAP


//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------



#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) >= 8 && (p) <= 11) ? (PCIE0) : (PCIE1))
#define digitalPinToPCMSK(p)    (((p) >= 0 && (p) <= 16) ? ((p<8)?(&PCMSK0) : (&PCMSK1)) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p) ((p) & 0x07)


#define digitalPinToInterrupt(p)  ((p) == 14 ? 0 : ((p)==2?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY861
//
//                   +-\/-+
//      (D  8) PB0  1|    |20  PA0 (D  0)
//     *(D  9) PB1  2|    |19  PA1 (D  1)
//      (D 10) PB2  3|    |18  PA2 (D  2) INT1
//     *(D 11) PB3  4|    |17  PA3 (D  3)
//             VCC  5|    |16  AGND
//             GND  6|    |15  AVCC
//      (D 12) PB4  7|    |14  PA4 (D  4)
//     *(D 13) PB5  8|    |13  PA5 (D  5)
// INT0 (D 14) PB6  9|    |12  PA6 (D  6)
//      (D 15) PB7 10|    |11  PA7 (D  7)
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
  PA,
  PA,
  PA,
  PA,
  PA,
  PB, /* 8 */
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PB, /* 15 */
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
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7), /* 15 */
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
  NOT_ON_TIMER,
  TIMER1B,
  NOT_ON_TIMER,
  TIMER1D,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};

#endif

#endif
