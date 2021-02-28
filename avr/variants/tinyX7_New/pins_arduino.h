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

#define ATTINYX7 1       //backwards compat
#define __AVR_ATtinyX7__ //recommended

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           11



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

#define LED_BUILTIN (3)

//Ax constants cannot be used for digitalRead/digitalWrite/analogWrite functions, only analogRead().

#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA3)
#define PIN_A4      (PIN_PA4)
#define PIN_A5      (PIN_PA5)
#define PIN_A6      (PIN_PA6)
#define PIN_A7      (PIN_PA7)
#define PIN_A8      (PIN_PB5)
#define PIN_A9      (PIN_PB6)
#define PIN_A10     (PIN_PB7)

static const uint8_t  A0 =  ADC_CH(0);
static const uint8_t  A1 =  ADC_CH(1);
static const uint8_t  A2 =  ADC_CH(2);
static const uint8_t  A3 =  ADC_CH(3);
static const uint8_t  A4 =  ADC_CH(4);
static const uint8_t  A5 =  ADC_CH(5);
static const uint8_t  A6 =  ADC_CH(6);
static const uint8_t  A7 =  ADC_CH(7);
static const uint8_t  A8 =  ADC_CH(8);
static const uint8_t  A9 =  ADC_CH(9);
static const uint8_t A10 = ADC_CH(10);

#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ((p) >= 8 ? 1 : 0)
#define digitalPinToPCMSK(p)    ((p) >= 8 ?(&PCMSK1) : (&PCMSK0))
#define digitalPinToPCMSKbit(p) (p & 15)

#define digitalPinToInterrupt(p)  ((p) == PIN_PB6 ? 0 : ((p)==PIN_PA3?1: NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  ((p) < 8 ? (p) :((p) + 5))
#define digitalPinToAnalogInput(p)  ((p) < 8 ? (p) : ((p) > 12 ? (p - 5) : (-1)))

#define digitalPinHasPWM(p)     ((p) == 2 || (p) > 7 )

#define PINMAPPING_NEW
//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

// Choosing not to initialise saves power and flash. 1 = initialise.
//#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
//#define INITIALIZE_SECONDARY_TIMERS               1

//#define TIMER_TO_USE_FOR_MILLIS                   0

// This is commented out. The only place where HAVE_BOOTLOADER is checked is in wiring.c, where it wastes precious bytes of flash resetting timer-related registers out of fear that the bootloader has scribbled on them.
// However, Optiboot does a WDR before jumping straight to app to start after running.
// This means that optiboot leaves all the registers clean. Meanwhile, Micronucleus doesn't even USE any of the timers, and that's all the wiring.c code checks on (to make sure millis will work)
// commenting out instead of setting to 0, as that would allow a hypothetical badly behaved bootloader to be supported in the future by having it add -DHAVE_BOOTLOADER from boards.txt
// #define HAVE_BOOTLOADER                           1

#define USE_SOFTWARE_SERIAL           0


/*----------------------------------------------------------
 * Chip Features - Analog stuff
 *----------------------------------------------------------*/

// Analog Comparator will be listed ONLY where it is used for software serial
// Core does not have built-in comparator library, nor will it ever.
//#define ANALOG_COMP_DDR               DDRA
//#define ANALOG_COMP_PORT              PORTA
//#define ANALOG_COMP_PIN               PINA
//#define ANALOG_COMP_AIN0_BIT          6
//#define ANALOG_COMP_AIN1_BIT          7

/* Analog reference bit masks. */
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// External voltage reference at PA0 (AREF) pin, internal reference turned off
#define EXTERNAL (1)
// Internal 1.1V voltage reference
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL
#define INTERNAL2V56 (7)

/* Special Analog Channels */
#define ADC_TEMPERATURE    ADC_CH(0x0B)
#define ADC_INTERNAL1V1    ADC_CH(0x0B)
#define ADC_AVCCDIV4       ADC_CH(0x0B)
#define ADC_GROUND         ADC_CH(0x0B)

/* Differential Analog Channels */
#define DIFF_A0_A1_8X      ADC_CH(0x10)
#define DIFF_A0_A1_20X     ADC_CH(0x11)
#define DIFF_A1_A2_8X      ADC_CH(0x12)
#define DIFF_A1_A2_20X     ADC_CH(0x13)
#define DIFF_A2_A3_8X      ADC_CH(0x14)
#define DIFF_A2_A3_20X     ADC_CH(0x15)
#define DIFF_A4_A5_8X      ADC_CH(0x16)
#define DIFF_A4_A5_20X     ADC_CH(0x17)
#define DIFF_A5_A6_8X      ADC_CH(0x18)
#define DIFF_A5_A6_20X     ADC_CH(0x19)
#define DIFF_A6_A7_8X      ADC_CH(0x1A)
#define DIFF_A6_A7_20X     ADC_CH(0x1B)
#define DIFF_A8_A9_8X      ADC_CH(0x1C)
#define DIFF_A8_A9_20X     ADC_CH(0x1D)
#define DIFF_A9_A10_8X     ADC_CH(0x1E)
#define DIFF_A9_A10_20X    ADC_CH(0x1F)

/*----------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *----------------------------------------------------------*/

/*  This part has a USI, not an SPI or TWI module. Accordingly, there is no MISO/MOSI in hardware.
    There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI;
    the defines here specify the pins for master mode, as SPI master is much more commonly used
    in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware
    of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI
    markings on the pinout diagram in the datasheet are for ISP programming, where the chip is
    a slave. The pinout diagram included with this core attempt to clarify this.
    The SS pin is chosen arbitrarily - we have no SPI slave library included with the core, but
    libraries acting as master often expect there to be an SS pin defined, and will throw errors
    if there isn't one. Since we provide an SPI.h that mimics the interface of the standard one
    we also provide a dummy SS pin macro. MISO/MOSI/SCK, SDA, SCL #defines are in Arduino.h and
    refer back to these macros (PIN_USI_* )*/


//#define USE_SOFTWARE_SPI 0

#define PIN_USI_DI      PIN_PB0
#define PIN_USI_DO      PIN_PB1
#define PIN_USI_SCK     PIN_PB2

#define USI_DATA_DDR       DDRB
#define USI_DATA_PORT     PORTB
#define USI_DATA_PIN       PINB

#define USI_CLOCK_BIT     PINB2
#define USI_DO_BIT        PINB1
#define USI_DI_BIT        PINB0

#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif


#define MISO PIN_PA2
#define MOSI PIN_PA4
#define SCK  PIN_PA5
#define SS   PIN_PA6

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY167
//
//                   +-\/-+
// RX   (D  0) PA0  1|    |20  PB0 (D  8)
// TX   (D  1) PA1  2|    |19  PB1 (D  9)
//     *(D  2) PA2  3|    |18  PB2 (D 10)
// INT1 (D  3) PA3  4|    |17  PB3 (D 11)*
//            AVCC  5|    |16  GND
//            AGND  6|    |15  VCC
//      (D  4) PA4  7|    |14  PB4 (D 12)
//      (D  5) PA5  8|    |13  PB5 (D 13)
//      (D  6) PA6  9|    |12  PB6 (D 14)* INT0
//      (D  7) PA7 10|    |11  PB7 (D 15)
//                   +----+
//
// * indicates PWM pin.

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRA,
  (uint8_t)(uint16_t)&DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTA,
  (uint8_t)(uint16_t)&PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PINA,
  (uint8_t)(uint16_t)&PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PA, /* 0 */
  PA,
  PA, /* 2 */
  PA, /* 3 */
  PA, /* 4 */
  PA,
  PA,
  PA,
  PB,
  PB,
  PB, /* 10 */
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
  _BV(2), /* 2 */
  _BV(3), /* 3 */
  _BV(4), /* 4 */
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2), /* 10 */
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
  TIMER0A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIM1AU,
  TIM1BU,
  TIM1AV,
  TIM1BV,
  TIM1AW,
  TIM1BW,
  TIM1AX,
  TIM1BX
};

#endif

#endif


//Old code, just here for temporary backup until I decide it is not needed.
//WARNING, if using software, RX must be on a pin which has a Pin change interrupt <= 7 (e.g. PCINT6, or PCINT1, but not PCINT8)
/*#define USE_SOFTWARE_SERIAL             1
//These are set to match Optiboot pins.

#define SOFTWARE_SERIAL_PORT            PORTB
#define SOFTWARE_SERIAL_TX              0
#define SOFTWARE_SERIAL_PIN             PINB
#define SOFTWARE_SERIAL_RX              1*/
