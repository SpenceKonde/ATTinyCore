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
 * MH-ET/MH-Tiny Pin Mapping
 * The MH-Tiny VUSB board based on the ATtiny88 uses an external 16 MHz clock
 * (so PB6 is out of the picture as far as I/O goes), numbers the pins up to
 * that the same way as an Uno or the standard Tiny88 mapping. After that
 * though, it's very different. Use this pinout if you're using one of those
 * boards, otherwise use standard.
 *---------------------------------------------------------------------------*/

#define ATTINYX8 1       // backwards compatibility
#define __AVR_ATtinyX8__ // recommended

#define NUM_DIGITAL_PINS  (27)
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
//      PIN_PB6 is the CLKI pin, which is not available on the MH-Tiny
//      because they use an external 16 MHz clock.
#define PIN_PB7  (14)
#define PIN_PA2  (15) /* Wait what? There's a reason            */
#define PIN_PA3  (16) /* PA0 and PA1 have ADC, and they         */
#define PIN_PA0  (17) /* wanted to keep the ADC pins together   */
#define PIN_PA1  (18) /* but also keep the ports together, so   */
#define PIN_PC0  (19) /* they didn't want to put PORTA after    */
#define PIN_PC1  (20) /* the first 6 PORTC pins, since PC7      */
#define PIN_PC2  (21) /* would then be on the other side of it  */
#define PIN_PC3  (22)
#define PIN_PC4  (23)
#define PIN_PC5  (24)
#define PIN_PC7  (25)
#define PIN_PC6  (26) /* PC6 is reset; convention is to number reset last. */

/* The t88 removed a Vcc/Gnd pair and the dedicated AREF pin, as compaared to
 * the mega x8-series, and made the last two ADC channels into real pins
 * on a new port. The pins that replaced the Vcc/Gnd pair joined those two
 * in the newly created half-size PORTA, while the AREF pin became the
 * final bit of PORTC.
 */

#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PD0)
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

/* An "analog pins" these map directly to analog channels */
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

#define digitalPinToPCICR(p)        (&PCICR)
/*                                  PORTD:                    PORTB (PB6 is osc, skipped in pin #'s         PORTA - numbered PA2, PA3, PA0, PA1 see above note      PORTC   */
#define digitalPinToPCICRbit(p)     (((p) <= 7) ? PCIE2     : (((p) <= 14) ? PCIE0                        : (((p) <= 18) ? PCIE3                                  : PCIE1)))
#define digitalPinToPCMSK(p)        (((p) <= 7) ? (&PCMSK2) : (((p) <= 14) ? (&PCMSK0)                    : (((p) <= 18) ? (&PCMSK3)                              : (&PCMSK1))))
#define digitalPinToPCMSKbit(p)     (((p) <= 7) ? (p)       : (((p) <= 13) ? ((p) - 8) : (((p) == 14) ? 7 : (((p) <= 16) ? ((p) - 14) : (((p) <= 18) ? ((p) - 17) : (((p) == 25) ? 7 : ((p) - 19)))))))

#define digitalPinToInterrupt(p)    ((p) == 2 ? 0 : ((p) == 3 ? 1: NOT_AN_INTERRUPT))

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) < 8) ? (((p) < 6) ? (p) + 11 :(p) + 19 ): NOT_A_PIN)
#define digitalPinToAnalogInput(p)  ((p) < 25 ? (((p) > 18) ? ((p) - 19) : (((p) > 16) ? ((p) - 11) : NOT_A_PIN)) : NOT_A_PIN)
/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10)

#define PINMAPPING_MHTINY

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
#define DEFAULT             ADC_REF(1)
#define INTERNAL1V1         ADC_REF(0)
#define INTERNAL           INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_TEMPERATURE   ADC_CH(0x08)
#define ADC_INTERNAL1V1   ADC_CH(0x0E)
#define ADC_GROUND        ADC_CH(0x0F)

/* Not a differential ADC */

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR          DDRD
#define ANALOG_COMP_PORT         PORTD
#define ANALOG_COMP_PIN          PIND
#define ANALOG_COMP_AIN0_BIT     (6)
#define ANALOG_COMP_AIN1_BIT     (7)

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
#define SS              PIN_PB2
#define MOSI            PIN_PB3
#define MISO            PIN_PB4
#define SCK             PIN_PB5

/* Hardware TWI */
#define SDA             PIN_PC4
#define SCL             PIN_PC5

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY88 MH-ET / MH-Tiny Pin Mapping
 *
 *                     ( 2)  ( 0)  (24/  (22/
 *                                 /A5)  /A3)
 *                        ( 1)  (26)  (23/  (21/
 *                              /RST  /A4)  /A2)
 *                     PD2   PD0   PC5   PC3
 *                        PD1   PC6   PC4   PC2
 *                     32    30    28    26
 *                        31    29    27    25
 *                    ┌ ─  ─  ─  ─  ─  ─  ─  ─ ┐
 *     ( 3)  PD3   1  |°                       |  24  PC1   (20/A1)
 *     ( 4)  PD4   2  |                        |  23  PC0   (19/A0)
 *     (17)  PA2   3  |        ATtiny88        |  22  PA1   (16/A7)
 *           VCC   4  |           on           |  21  GND
 *           GND   5  |   "MH-ET" or "MH-tiny" |  20  PC7   (25)
 *     (18)  PA3   6  |          board         |  19  PA0   (15/A6)
 *    CLKIN  PB6   7  |                        |  18  AVCC
 *     (14)  PB7   8  |                        |  17  PB5   (13)
 *                    └ ─  ─  ─  ─  ─  ─  ─  ─ ┘
 *                      9    11    13    15
 *                        10    12    14    16
 *                     PD5   PD7   PB1   PB3
 *                        PD6   PB0   PB2   PB4
 *                     ( 5)  ( 7)  ( 9)  (11)
 *                        ( 6)  ( 8)  (10)  (12)
 *
 * PWM on pins 9 and 10 only.
 *---------------------------------------------------------------------------*/


const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &DDRA,
  (uint16_t) &DDRB,
  (uint16_t) &DDRC,
  (uint16_t) &DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &PORTA,
  (uint16_t) &PORTB,
  (uint16_t) &PORTC,
  (uint16_t) &PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint16_t) &PINA,
  (uint16_t) &PINB,
  (uint16_t) &PINC,
  (uint16_t) &PIND,
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
  PB,   /* 8 */
  PB,
  PB,
  PB,
  PB,
  PB,
  //    /* no PB6 */
  PB,
  PA,   /* 15 */
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

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
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
  //       /* no PB6 */
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

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
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
