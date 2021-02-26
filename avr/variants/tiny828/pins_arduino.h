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

#define ATTINY828 1
#define __AVR_ATtinyX4__
//#define USE_SOFTWARE_SPI
#define USE_SOFTWARE_SERIAL   0

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            28
#define NUM_ANALOG_INPUTS           28
#define analogInputToDigitalPin(p)  ((p < 28) ? (p): -1)

#define ADC_TEMPERATURE 34

#define digitalPinHasPWM(p)         ((p) == PIN_PC0 || (p) == PIN_PC4 || (p) == PIN_PC5 || (p) == PIN_PC6)

//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)


#define SS   PIN_PC0
#define MOSI PIN_PD3
#define MISO PIN_PD1
#define SCK  PIN_PD3



static const uint8_t SDA = 4;
static const uint8_t SCL = 6;

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().

static const uint8_t  A0 = (0x80 |  0);
static const uint8_t  A1 = (0x80 |  1);
static const uint8_t  A2 = (0x80 |  2);
static const uint8_t  A3 = (0x80 |  3);
static const uint8_t  A4 = (0x80 |  4);
static const uint8_t  A5 = (0x80 |  5);
static const uint8_t  A6 = (0x80 |  6);
static const uint8_t  A7 = (0x80 |  7);
static const uint8_t  A8 = (0x80 |  8);
static const uint8_t  A9 = (0x80 |  9);
static const uint8_t A10 = (0x80 | 10);
static const uint8_t A11 = (0x80 | 11);
static const uint8_t A12 = (0x80 | 12);
static const uint8_t A13 = (0x80 | 13);
static const uint8_t A14 = (0x80 | 14);
static const uint8_t A15 = (0x80 | 15);
static const uint8_t A16 = (0x80 | 16);
static const uint8_t A17 = (0x80 | 17);
static const uint8_t A18 = (0x80 | 18);
static const uint8_t A19 = (0x80 | 19);
static const uint8_t A20 = (0x80 | 20);
static const uint8_t A21 = (0x80 | 21);
static const uint8_t A22 = (0x80 | 22);
static const uint8_t A23 = (0x80 | 23);
static const uint8_t A24 = (0x80 | 24);
static const uint8_t A25 = (0x80 | 25);
static const uint8_t A26 = (0x80 | 26);
static const uint8_t A27 = (0x80 | 27);
static const uint8_t A28 = (0x80 | 28);



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
#define PIN_PB2  (10)
#define PIN_PB3  (11)
#define PIN_PB4  (12)
#define PIN_PB5  (13)
#define PIN_PB6  (14)
#define PIN_PB7  (15)
#define PIN_PC0  (16)
#define PIN_PC1  (17)
#define PIN_PC2  (18)
#define PIN_PC3  (19)
#define PIN_PC4  (20)
#define PIN_PC5  (21)
#define PIN_PC6  (22)
#define PIN_PC7  (23)
#define PIN_PD0  (24)
#define PIN_PD1  (25)
#define PIN_PD2  (26)  /* RESET */
#define PIN_PD3  (27)



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
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL           0
//Please define the port on which the analog comparator is found.
//#define ANALOG_COMP_DDR               DDRA
//#define ANALOG_COMP_PORT              PORTA
//#define ANALOG_COMP_PIN               PINA
//#define ANALOG_COMP_AIN0_BIT          1
//#define ANALOG_COMP_AIN1_BIT          2


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



#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ((p) >> 3)
#define digitalPinToPCMSK(p)    ((p) > 23 ? (&PCMSK3) : (&PCMSK0 + (p >> 3)))
#define digitalPinToPCMSKbit(p) ((p) & 0x07)

#define digitalPinToInterrupt(p)  ((p) == PIN_PC1 ? 0 : ((p) == PIN_PC2 ? 1 : NOT_AN_INTERRUPT))
#ifdef ARDUINO_MAIN
// ATMEL/Microchip ATtiny828
//
//             16*   26   24   14
//          17    27   25   15
//             PC0  PD2  PD0  PB6
//          PC1  PD3  PD1   PB7
//             _________________
// 18 RX  PC2 | *               | PB5   13
// 19 TX  PC3 |                 | PB4   12
// 20 *   PC4 |                 | PB3   11
//        VCC |                 | GND
//        GND |                 | PB2   10
// 21 *   PC5 |                 | PB1    9
// 22 *   PC6 |                 | AVCC
// 23     PC7 |_________________| PB0    8
//           PA0  PA2  PA4  PA6
//              PA1  PA3  PA5  PA7
//            0     2    4    6
//               1     3    5    7


const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRA,
  (uint8_t)(uint16_t)&DDRB,
  (uint8_t)(uint16_t)&DDRC,
  (uint8_t)(uint16_t)&DDRD
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTA,
  (uint8_t)(uint16_t)&PORTB,
  (uint8_t)(uint16_t)&PORTC,
  (uint8_t)(uint16_t)&PORTD
};

const uint8_t PROGMEM port_to_pullup_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PUEA,
  (uint8_t)(uint16_t)&PUEB,
  (uint8_t)(uint16_t)&PUEC,
  (uint8_t)(uint16_t)&PUED
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PINA,
  (uint8_t)(uint16_t)&PINB,
  (uint8_t)(uint16_t)&PINC,
  (uint8_t)(uint16_t)&PIND
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_A_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_B_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_C_ID,
  PORT_D_ID,
  PORT_D_ID,
  PORT_D_ID,
  PORT_D_ID
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
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
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0B,
  TIMER1A,
  TIMER1B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER
};



#endif

#endif

#endif
