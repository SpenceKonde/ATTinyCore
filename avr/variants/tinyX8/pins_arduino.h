/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny88, ATtiny48
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 * Similar to Uno et. al. pin mapping, use this unless working with an
 * MH-ET/MH-Tiny board, which have a different numbering scheme.
 *---------------------------------------------------------------------------*/

#define ATTINYX8 1       // backwards compatibility
#define __AVR_ATtinyX8__ // recommended

#define NUM_DIGITAL_PINS  (28)
#define NUM_ANALOG_INPUTS (8)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
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
#define PIN_PB6  (14)
#define PIN_PB7  (15)
#define PIN_PC7  (16)
#define PIN_PC0  (17)
#define PIN_PC1  (18)
#define PIN_PC2  (19)
#define PIN_PC3  (20)
#define PIN_PC4  (21)
#define PIN_PC5  (22)
#define PIN_PA0  (23)
#define PIN_PA1  (24)
#define PIN_PA2  (25)
#define PIN_PA3  (26)
#define PIN_PC6  (27)

#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PB5)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0        (PIN_PC0)
#define PIN_A1        (PIN_PC1)
#define PIN_A2        (PIN_PC2)
#define PIN_A3        (PIN_PC3)
#define PIN_A4        (PIN_PC4)
#define PIN_A5        (PIN_PC5)
#define PIN_A6        (PIN_PA0)
#define PIN_A7        (PIN_PA1)

/* The "analog pins" of the form An, where n is a number map directly to
 * analog channels of the same number. Except on Digispark Pro pin mapping.
 *---------------------------------------------------------------------------*/
static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);
static const uint8_t A4 = ADC_CH(4);
static const uint8_t A5 = ADC_CH(5);
static const uint8_t A6 = ADC_CH(6);
static const uint8_t A7 = ADC_CH(7);

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)    (((p) >= 0  && (p) <= 26) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7)  ? 2 : (((p) <= 15) ? 0 : (((p) <= 22) ? 1 : 3)))
#define digitalPinToPCMSK(p)    (((p) <= 7)  ? (&PCMSK2) : (((p) <= 15) ? (&PCMSK0) : (((p) <= 22) ? (&PCMSK1) : (((p) <= 26) ? (&PCMSK3) : ((uint8_t *)0)))))
#define digitalPinToPCMSKbit(p) (((p) <= 15) ? ((p)& 0x7) : (((p) == 16) ? (7) : (((p) <= 22) ? ((p) - 17) : ((p) - 23))))

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p)==3?1: NOT_AN_INTERRUPT))

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) < 8) ? (p) + 17 : -1);
#define digitalPinToAnalogInput(p)  ((p) >= 16 && (p) <= 22) ? ((p)-17):NOT_A_PIN;

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10)


#define PINMAPPING_NORMAL

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.      1 = initialise.
// #define DEFAULT_INITIALIZE_ADC               1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS  1

/* Builtin Software Serial "Serial"
 * TX is on AIN0, RX is on AIN1. Comparator interrupt used so PCINTs remain
 * available for other uses. Comparator pins in Analog section below.        */
#define USE_SOFTWARE_SERIAL                     1

/*---------------------------------------------------------------------------
 * Chip Features - Timers amnd PWM
 *---------------------------------------------------------------------------
 * Basic PWM is covered elsewhere, but this lets you look up what pin is on
 * a given compare channel easily. Used to generate some pinmapping independent
 * defines for TimerOne library back in Arduino.h
 *
 * Functions of timers associated with pins have pins specified by macros of
 * the form PIN_TIMER_ followed by the function.
 *
 * PWM_CHANNEL_REMAPPING is defined and true where the PWM channels from timers
 * has additional non-standard behavior allowing the remapping of output from
 * otherwise normal pins (and interfering with naive code that enables them,
 * though if the code acts only on the timer registers, it will often work if
 * user code calls analogWrite() on the pin before letting the library use it.
 * Where this is not the case, it is not defined.
 *
 * TIMER0_TYPICAL is 1 if that timer is present, and is an 8-bit timer with or
 * without two output compare channels. PIN_TIMER_OC0A/OC0B will be defined if
 * it has them.
 *
 * TIMER1_TYPICAL is 1 if that timer is present, and is a 16-bit timer with PWM
 * as opposed to some bizarro one like the 85 and 861 have.
 *
 * TIMER2_TYPICAL is 1 if that timer is present, and is an 8-bit asynch timer,
 * like on classic ATmega parts. There is only one ATTinyCore part with a
 * Timer2, and this is false there, because that timer is instead like Timer1.
 *
 * We do not provide further macros to characterize the type of a timer in more
 * detail but the sheer variety of atypical timers on classic AVRs made it hard
 * to derive a quick test of whether the normal stuff will work.
 *---------------------------------------------------------------------------*/

/* Timer 0 - 8-bit timer without PWM */
#define TIMER0_TYPICAL              (1)
#define PIN_TIMER_T0                (PIN_PD4)

/* Timer 1 - 16-bit timer with PWM */
#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PB1)
#define PIN_TIMER_OC1B              (PIN_PB2)
#define PIN_TIMER_T1                (PIN_PD5)
#define PIN_TIMER_ICP1              (PIN_PB0)

/*---------------------------------------------------------------------------
 * Chip Features (or lack thereof) - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)            (x << 6)

/*  Analog reference bit masks. */
#define DEFAULT               ADC_REF(1)
#define INTERNAL1V1           ADC_REF(0)
#define INTERNAL              INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_TEMPERATURE       ADC_CH(0x08)
#define ADC_INTERNAL1V1       ADC_CH(0x0E)
#define ADC_GROUND            ADC_CH(0x0F)

/* Not a differential ADC     *
 * single ended channels only */

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR       DDRD
#define ANALOG_COMP_PORT      PORTD
#define ANALOG_COMP_PIN       PIND
#define ANALOG_COMP_AIN0_BIT  (6)
#define ANALOG_COMP_AIN1_BIT  (7)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a real SPI module and a real master/slave TWI module
 * You may not get decent on-chip peripherals, but you can at least effectively
 * talk to off-chip ones! Seems like it's meant to be a replacement for cost
 * optimizing designs that don't need any of the expensive features, which
 * covers a lot of use cases. Reimplementing something to use a USI instead of
 * a real SPI or TWI interface is not for the faint of heart. Nor really anyone
 * who isn't a digital masochist - unlike reimplementing things in assembly,
 * using the USI is never illuminating, but is every bit as annoying.
 *---------------------------------------------------------------------------*/

/* Hardware SPI */
#define SS                    PIN_PB2
#define MOSI                  PIN_PB3
#define MISO                  PIN_PB4
#define SCK                   PIN_PB5

/* Hardware TWI */
#define SDA                   PIN_PC4
#define SCL                   PIN_PC5

#ifdef ARDUINO_MAIN

/*---------------------------------------------------------------------------
 * ATMEL ATTINY88/48  ATTinyCore Standard Pin Mapping
 *
 *                   +-\/-+
 *        (27) PC6 1 |    |28  PC5 (22)
 *        ( 0) PD0 2 |    |27  PC4 (21)
 *        ( 1) PD1 3 |    |26  PC3 (20)
 *        ( 2) PD2 4 |    |25  PC2 (19)
 *        ( 3) PD3 5 |    |24  PC1 (18)
 *        ( 4) PD4 6 |    |23  PC0 (17)
 *             VCC 7 |    |22  GND
 *             GND 8 |    |21  PC7 (16)
 *        (14) PB6 9 |    |20  AVCC
 *        (15) PB7 10|    |19  PB5 (13)
 *        ( 5) PD5 11|    |18  PB4 (12)
 *        ( 6) PD6 12|    |17  PB3 (11)
 *        ( 7) PD7 13|    |16  PB2 (10) PWM
 *        ( 8) PB0 14|    |15  PB1 ( 9) PWM
 *                   +----+
 *
 *
 *                     ( 2)  ( 0)  (22)  (20)
 *                                 (A5)  (A3)
 *                        ( 1)  (27)  (21)  (19)
 *                                    (A4)  (A2)
 *                     PD2   PD0   PC5   PC3
 *                        PD1   PC6   PC4   PC2
 *                      32    30    28    26
 *                         31    29    27    25
 *                    ┌ ─  ─  ─  ─  ─  ─  ─  ─ ┐
 *      (3 )  PD3  1  |°                       |  24  PC1  (18/A1)
 *      (4 )  PD4  2  |                        |  23  PC0  (17/A0)
 *      (25)  PA2  3  |        ATtiny88        |  22  PA1  (24/A7)
 *            VCC  4  |                        |  21  GND
 *            GND  5  |                        |  20  PC7  (16)
 *      (26)  PA3  6  |                        |  19  PA0  (23/A6)
 *      (14)  PB6  7  |                        |  18  AVCC
 *      (15)  PB7  8  |                        |  17  PB5  (13)
 *                    └ ─  ─  ─  ─  ─  ─  ─  ─ ┘
 *                       9    11    13    15
 *                         10    12    14    16
 *                     PD5   PD7   PB1   PB3
 *                        PD6   PB0   PB2   PB4
 *                     ( 5)  ( 7)  ( 9)  (11)
 *                        ( 6)  ( 8)  (10)  (12)
 *                                  PWM
 *                                     PWM
 *---------------------------------------------------------------------------*/

const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRA,
  (uint8_t)(uint16_t) &DDRB,
  (uint8_t)(uint16_t) &DDRC,
  (uint8_t)(uint16_t) &DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTA,
  (uint8_t)(uint16_t) &PORTB,
  (uint8_t)(uint16_t) &PORTC,
  (uint8_t)(uint16_t) &PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINA,
  (uint8_t)(uint16_t) &PINB,
  (uint8_t)(uint16_t) &PINC,
  (uint8_t)(uint16_t) &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
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
  PB,
  PB,
  PC, /* 16 */
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PA, /* 23 */
  PA,
  PA,
  PA,
  PC
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0, port D */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* 8, port B */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(7), /* 16, port C */
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(0), /* 23, port A */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(6)
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER, /* 0 - port D */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 8 - port B */
  TIMER1A,
  TIMER1B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 16 - port C */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 23 - port A */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER
};

#endif

#endif
