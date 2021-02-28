/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny43
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros |
 *-----------------------------------------------------------------*/
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#define ATTINY43 1
#define __AVR_ATtiny43__
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            (12)
#define NUM_ANALOG_INPUTS           (4)


#define PIN_PA0  ( 8)
#define PIN_PA1  ( 9)
#define PIN_PA2  (10)
#define PIN_PA3  (11)
#define PIN_PA4  (12)
#define PIN_PA5  (13)
#define PIN_PA6  (14)
#define PIN_PA7  (15) /* RESET */
#define PIN_PB0  ( 0)
#define PIN_PB1  ( 1)
#define PIN_PB2  ( 2)
#define PIN_PB3  ( 3)
#define PIN_PB4  ( 4)
#define PIN_PB5  ( 5)
#define PIN_PB6  ( 6)
#define PIN_PB7  ( 7)

#define LED_BUILTIN (PIN_PA5)

#define PIN_A0  (PIN_PA0)
#define PIN_A1  (PIN_PA1)
#define PIN_A2  (PIN_PA2)
#define PIN_A3  (PIN_PA3)

static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);


#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) >= 0 && (p) <= 7) ? 5 : 4)
#define digitalPinToPCMSK(p)    (((p) >= 0 && (p) <= 7) ? (&PCMSK1) : ((p) <= 15) ? (&PCMSK0) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p) ((p)&0x07)

#define digitalPinToInterrupt(p)  ((p) == 7 ? 0 : NOT_AN_INTERRUPT)


#define analogInputToDigitalPin(p)  (((p) < 4) ? (P) + 8 : -1)
#define digitalPinToAnalogInput(p)  (((p) > 7) & (p) < 12 ? (p) - 8: -1)

#define digitalPinHasPWM(p)         ((p) == PIN_PB1 || (p) == PIN_PB2 || (p) == PIN_PB4 || (p) == PIN_PB5)


//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

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
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          4
#define ANALOG_COMP_AIN1_BIT          5


/* Analog reference bit masks.*/
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// Internal 1.1V voltage reference
#define INTERNAL (1)
#define INTERNAL1V1 INTERNAL

/* Special Analog Channels */
#define ADC_GROUND        ADC_CH(0x04)
#define ADC_INTERNAL1V1   ADC_CH(0x05)
#define ADC_VBATDIV2      ADC_CH(0x06)
#define ADC_TEMPERATURE   ADC_CH(0x07)

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

#define USE_SOFTWARE_SPI 1

#define PIN_USI_DI      PIN_PB4
#define PIN_USI_DO      PIN_PB5
#define PIN_USI_SCK     PIN_PB6
#define SS              PIN_PB7

#define USI_DATA_DDR       DDRB
#define USI_DATA_PORT     PORTB
#define USI_DATA_PIN       PINB

#define USI_CLOCK_BIT     PINB6
#define USI_DO_BIT        PINB5
#define USI_DI_BIT        PINB4

#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif

#ifdef ARDUINO_MAIN

// ATMEL ATTINY43 / ARDUINO
//
//                    +-\/-+
//        ( 0)  PB0  1|    |20  PA7  (15)  RESET
//  PWM   ( 1)  PB1  2|    |19  PA6  (14)
//  PWM   ( 2)  PB3  3|    |18  PA5  (13)
//        ( 3)  PB2  4|    |17  PA4  (12)
//  PWM   ( 4)  PB7  5|    |16  PA3  (11)
//  PWM   ( 5)  PB7  6|    |15  PA2  (10)
//        ( 6)  PB7  7|    |14  PA1  ( 9)
//  INT0  ( 7)  PB7  8|    |13  PA0  ( 8)
//              Vcc  9|    |12  VBat
//             Gnd  10|    |11  LSW
//                    +----+

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
