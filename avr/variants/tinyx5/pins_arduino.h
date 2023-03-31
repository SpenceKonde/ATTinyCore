/* pins_arduino.h - Pin definition functions for ATTinyCore
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny25, 45, 85
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 * Pin numbers the same as bit positions within PORTB.
 *
 *---------------------------------------------------------------------------*/

#define ATTINYX5 1       // backwards compatibility
#define __AVR_ATtinyX5__ // recommended

#define NUM_DIGITAL_PINS            (6)
#define NUM_ANALOG_INPUTS           (4)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PB0  ( 0)
#define PIN_PB1  ( 1)
#define PIN_PB2  ( 2)
#define PIN_PB3  ( 3) /* XTAL1 */
#define PIN_PB4  ( 4) /* XTAL2 */
#define PIN_PB5  ( 5) /* RESET */

#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PB1)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0        (PIN_PB0)
#define PIN_A1        (PIN_PB2)
#define PIN_A2        (PIN_PB4)
#define PIN_A3        (PIN_PB3)

/* An "analog pins" these map directly to analog channels */
static const uint8_t  A0 =  ADC_CH(0);
static const uint8_t  A1 =  ADC_CH(1);
static const uint8_t  A2 =  ADC_CH(2);
static const uint8_t  A3 =  ADC_CH(3);

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/
#define digitalPinToPCICR(p)        ((uint8_t)(p) < 6 ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (5)
#define digitalPinToPCMSK(p)        ((uint8_t)(p) < 6 ? (&PCMSK) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)     (p)

#define digitalPinToInterrupt(p)    ((p)==PIN_PB2 ? 0 : NOT_AN_INTERRUPT)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) == 0) ? 5 : (((p) == 1) ? 2 : (((p) == 2) ? 4 :(((p) == 3) ? 3 : -1))))
#define digitalPinToAnalogInput(p)  (((p) == 5) ? 0 : (((p) == 4) ? 2 : (((p) == 2) ? 1 :(((p) == 3) ? 3 : -1))))

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == 0 || (p) == 1 || (p)==4)

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

#define PWM_CHANNEL_REMAPPING       (0)

#define TIMER0_TYPICAL              (1)
#define PIN_TIMER_OC0A              (PIN_PB1)
#define PIN_TIMER_OC0B              (PIN_PB0)

/* Timer 1 - 8-bit high speed timer */
#define TIMER1_TYPICAL              (0)
#define PIN_TIMER_T1                (NOT_A_PIN)
#define PIN_TIMER_ICP1              (NOT_A_PIN)

#if !defined(PWM_PB1_TIM0)
  #define PWM_TIMER1
#endif

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash, while still starting from sane values instead of doing
 * this manually. ADMUX mapping looks like:
 *
 * REFS1 : REVS0 : ADLAR : REFS2 : MUX3 : MUX2 : MUX1 : MUX0
 *
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)       ((((x) & 0x03) << 6) | (((x) & 0x04) << 2))

/* Analog Reference bit masks */
#define DEFAULT            ADC_REF(0x00)
#define EXTERNAL           ADC_REF(0x01) /* Apply external reference voltage to AREF pin */
#define INTERNAL1V1        ADC_REF(0x02) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL             INTERNAL1V1 /* deprecated */
#define INTERNAL2V56       ADC_REF(0x06) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL2V56_CAP   ADC_REF(0x07) /* Interna1 2.56V reference is output on AREF, and may be used by other devices */
#define INTERNAL2V56_NO_CAP INTERNAL2V56
#define INTERNAL2V56NOBP    INTERNAL2V56  /* deprecated */
#define INTERNAL             INTERNAL1V1   /* deprecated */
/* Special Analog Channels */
#define ADC_TEMPERATURE     ADC_CH(0x0F)
#define ADC_INTERNAL1V1     ADC_CH(0x0D)
#define ADC_GROUND          ADC_CH(0x0C)

/* Differential Analog Channels */
#define DIFF_A2_A2_1X       ADC_CH(0x04)
#define DIFF_A2_A2_20X      ADC_CH(0x05)
#define DIFF_A2_A3_1X       ADC_CH(0x06)
#define DIFF_A2_A3_20X      ADC_CH(0x07)
#define DIFF_A0_A0_1X       ADC_CH(0x08)
#define DIFF_A0_A0_20X      ADC_CH(0x09)
#define DIFF_A0_A1_1X       ADC_CH(0x0A)
#define DIFF_A0_A1_20X      ADC_CH(0x0B)
// Let's call a spade a spade here - the options for differential ADC on the tiny x5 series are pretty bad. Without disabling reset, you have only one pair of pins you can measure between.
// And why did they put one of their 4 ADC channels onto reset, instead of using one of the two other pins that don't have ADC? Not their best work.


/* Analog Comparator - not used by core */
#define ANALOG_COMP_DDR               DDRB
#define ANALOG_COMP_PORT              PORTB
#define ANALOG_COMP_PIN               PINB
#define ANALOG_COMP_AIN0_BIT          (0)
#define ANALOG_COMP_AIN1_BIT          (1)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/*  This part has a USI, not a TWI module - but it DOES have an SPI module!
 *  We provide USI defines so that Arduino.h will sort out SCL, SDA pin assignment.
 *  The included version of Wire.h will use the USI for TWI if requested.
 *---------------------------------------------------------------------------*/

/* Hardware SPI */
#define MISO                  PIN_PB0
#define MOSI                  PIN_PB1
#define SCK                   PIN_PB2
#define SS                    PIN_PB3

/* USI */
#define USI_DI                PIN_PB0
#define USI_DO                PIN_PB1
#define USI_SCK               PIN_PB2

/* USI pins as ports and bits */
#define USI_DDR               DDRB
#define USI_PORT              PORTB
#define USI_PIN               PINB
#define USI_CLOCK_BIT         PINB2
#define USI_DO_BIT            PINB1
#define USI_DI_BIT            PINB0

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

#ifdef ARDUINO_MAIN

/*---------------------------------------------------------------------------

 *---------------------------------------------------------------------------*/

const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
  PB, /* 0 */
  PB,
  PB,
  PB,
  PB,
  PB, /* 5 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3), /* 3 port B */
  _BV(4),
  _BV(5),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
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
