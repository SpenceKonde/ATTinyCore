/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny85, ATtiny45, and ATtiny25
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros |
 *-----------------------------------------------------------------*/

#define ATTINYX5 1       //backwards compatibility
#define __AVR_ATtinyX5__ //this is recommended way

#define NUM_DIGITAL_PINS            6
#define NUM_ANALOG_INPUTS           6

#define PIN_PB0       (0)
#define PIN_PB1       (1)
#define PIN_PB2       (2)
#define PIN_PB3       (3)
#define PIN_PB4       (4)
#define PIN_PB5       (5)

#define LED_BUILTIN   (PIN_PB1)

#define PIN_A0        (PIN_PB5)
#define PIN_A1        (PIN_PB2)
#define PIN_A2        (PIN_PB4)
#define PIN_A3        (PIN_PB3)

static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;

#define digitalPinToInterrupt(p)    ((p) == 2 ? 0 : NOT_AN_INTERRUPT)

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 5) ? (&GIMSK) : ((uint8_t *) NULL))
#define digitalPinToPCICRbit(p)     5
#define digitalPinToPCMSK(p)        (((p) >= 0 && (p) <= 5) ? (&PCMSK) : ((uint8_t *) NULL))
#define digitalPinToPCMSKbit(p)     (p)

#define analogInputToDigitalPin(p)  (((p) == 0) ? 5 : (((p) == 1) ? 2 : (((p) == 2) ? 4 :(((p) == 3) ? 3 : -1))))
#define digitalPinHasPWM(p)         ((p) == 0 || (p) == 1 || (p)==4)


//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

//Why not? It's one of the special features of these parts, why not use it?
#define TIMER1_PWM



// Choosing not to initialise saves power and flash. 1 = initialise.
// #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
// #define INITIALIZE_SECONDARY_TIMERS               1
// #define TIMER_TO_USE_FOR_MILLIS                   0

/*
 * Where to put the software serial? (Arduino Digital pin numbers)
 * TX is on AIN0, RX is on AIN1. Comparator is favoured
 * to use its interrupt for the RX pin.
 * Comparator pins in Analog section below.
 */

#define USE_SOFTWARE_SERIAL           1

/*----------------------------------------------------------
 * Chip Features - Analog stuff
 *----------------------------------------------------------*/

// Analog Comparator will be listed ONLY where it is used for software serial
// Core does not have built-in comparator library, nor will it ever.
#define ANALOG_COMP_DDR               DDRB
#define ANALOG_COMP_PORT              PORTB
#define ANALOG_COMP_PIN               PINB
#define ANALOG_COMP_AIN0_BIT          0
#define ANALOG_COMP_AIN1_BIT          1

// X 0 0 VCC used as Voltage Reference, disconnected from PB0 (AREF).
#define DEFAULT             (0)
// X 0 1 External Voltage Reference at PB0 (AREF) pin, Internal Voltage Reference turned off.
#define EXTERNAL            (1)
// 0 1 0 Internal 1.1V Voltage Reference.
#define INTERNAL            (2)
#define INTERNAL1V1         INTERNAL
// 1 1 1 Internal 2.56V Voltage Reference with external bypass capacitor at PB0 (AREF) pin(1).
#define INTERNAL2V56        (7)
// 1 1 0 Internal 2.56V Voltage Reference without external bypass capacitor, disconnected from PB0 (AREF)(1).
#define INTERNAL2V56_NO_CAP (6)
#define INTERNAL2V56NOBP    INTERNAL2V56_NO_CAP

#define ADC_TEMPERATURE 15

/*----------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *----------------------------------------------------------*/

#define USE_SOFTWARE_SPI 1

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

#define PIN_USI_SCK     PIN_PB2
#define PIN_USI_DO      PIN_PB1
#define PIN_USI_DI      PIN_PB0

#define SS    PIN_PB3

#define USI_DATA_DDR    DDRB
#define USI_DATA_PORT   PORTB
#define USI_DATA_PIN    PINB

#define USI_CLOCK_BIT   PINB2
#define USI_DO_BIT      PINB1
#define USI_DI_BIT      PINB0

#define USI_START_VECTOR USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif


/*----------------------------------------------------------
 * PROGMEM Pin Mapping Arrays
 *----------------------------------------------------------*/

#ifdef ARDUINO_MAIN

// ATMEL ATTINY85/45/25
//
//                      +-\/-+
//       A0  (5)  PB5  1|    |8   VCC
//       A3  (3)  PB3  2|    |7   PB2  (2)  A1  INT0
//  PWM  A2  (4)  PB4  3|    |6   PB1  (1)      PWM (OC0B or OC1A)
//  OC1B          GND  4|    |5   PB0  (0)      PWM (OC0A)
//                      +----+
// Currently we default to using the superior timer 1 for PW pin 1, which could also use the boring timer 0.

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing) tiny45 only port B

const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PIN,
  NOT_A_PIN,
  (uint8_t)(uint16_t)&PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PB, /* 0 */
  PB,
  PB,
  PB,
  PB,
  PB, /* 5 */

};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3), /* 3 port B */
  _BV(4),
  _BV(5),

};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  TIMER0A, /* OC0A */
  #ifdef TIMER1_PWM
  TIMER1A, /* OC1A */
  #else
  TIMER0B, /* OC0B */
  #endif
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1B, /*OC1B*/
  NOT_ON_TIMER,
};

#endif

#endif
