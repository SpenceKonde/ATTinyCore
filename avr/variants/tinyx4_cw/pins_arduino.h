/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny84A, ATtiny44A, and ATtiny24A and non-A versions
 *===========================================================================
 * Clockwise mapping
 * If you have a choice, this is the pin mapping to use.
 *---------------------------------------------------------------------------*/


#define ATTINYX4 1
#define __AVR_ATtinyX4__

#define NUM_DIGITAL_PINS            (12)
#define NUM_ANALOG_INPUTS           ( 8)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PA0     ( 0)
#define PIN_PA1     ( 1)
#define PIN_PA2     ( 2)
#define PIN_PA3     ( 3)
#define PIN_PA4     ( 4)
#define PIN_PA5     ( 5)
#define PIN_PA6     ( 6)
#define PIN_PA7     ( 7)
#define PIN_PB2     ( 8)
#define PIN_PB1     ( 9)  /* XTAL1 */
#define PIN_PB0     (10)  /* XTAL2 */
#define PIN_PB3     (11)  /* RESET */

#ifndef LED_BUILTIN
  #ifndef USB_TWOPLUS
    #define LED_BUILTIN (PIN_PB2)
  #else
    #define LED_BUILTIN (PIN_PB0)
  #endif
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA3)
#define PIN_A4      (PIN_PA4)
#define PIN_A5      (PIN_PA5)
#define PIN_A6      (PIN_PA6)
#define PIN_A7      (PIN_PA7)

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

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 11) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) <= 7) ? PCIE0 : PCIE1 )
#define digitalPinToPCMSK(p)        (((p) <= 7) ? (&PCMSK0) : (((p) <= 11) ? (&PCMSK1) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p)     (((p) <= 7) ? (p) : (10 - (p)))

#define digitalPinToInterrupt(p)    ((p) == PIN_PB2 ? 0 : NOT_AN_INTERRUPT)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  ((p) < 8 ? (p) : -1)
#define digitalPinToAnalogInput(p)  ((p) < 8 ? (p) : -1)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) >= 5 && (p) <= 8)

/* We have multiple pin mappings on this part; all have a #define, where
 * multiple are present, these are for compatibility with versions that
 * used less-clear names. The first #define is recommended, all others are
 * deprecated. */
#define PINMAPPING_CW

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

/* Timer 0 - 8-bit timer with PWM */
#define TIMER0_TYPICAL              (1)
#define PIN_TIMER_OC0A              (PIN_PB2)
#define PIN_TIMER_OC0B              (PIN_PA7)
#define PIN_TIMER_T0                (PIN_PA3)

/* Timer 1 - 16-bit timer with PWM */
#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PA6)
#define PIN_TIMER_OC1B              (PIN_PA5)
#define PIN_TIMER_T1                (PIN_PA4)
#define PIN_TIMER_ICP1              (PIN_PA7)

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)            (x << 6)

/*  Analog reference bit masks. */
#define DEFAULT          ADC_REF(0x00)
#define EXTERNAL         ADC_REF(0x01)
#define INTERNAL1V1      ADC_REF(0x02)
#define INTERNAL           INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_GROUND        ADC_CH(0x20)
#define ADC_INTERNAL1V1   ADC_CH(0x21)
#define ADC_TEMPERATURE   ADC_CH(0x22)

/* Differential Analog Channels */
#define DIFF_A0_A0_20X    ADC_CH(0x23)
#define DIFF_A0_A1_1X     ADC_CH(0x08)
#define DIFF_A0_A1_20X    ADC_CH(0x09)
#define DIFF_A0_A3_1X     ADC_CH(0x0A)
#define DIFF_A0_A3_20X    ADC_CH(0x0B)
#define DIFF_A1_A0_1X     ADC_CH(0x28)
#define DIFF_A1_A0_20X    ADC_CH(0x29)
#define DIFF_A1_A2_1X     ADC_CH(0x0C)
#define DIFF_A1_A2_20X    ADC_CH(0x0D)
#define DIFF_A1_A3_1X     ADC_CH(0x0E)
#define DIFF_A1_A3_20X    ADC_CH(0x0F)
#define DIFF_A2_A1_1X     ADC_CH(0x2C)
#define DIFF_A2_A1_20X    ADC_CH(0x2D)
#define DIFF_A2_A3_1X     ADC_CH(0x10)
#define DIFF_A2_A3_20X    ADC_CH(0x11)
#define DIFF_A3_A0_1X     ADC_CH(0x2A)
#define DIFF_A3_A0_20X    ADC_CH(0x2B)
#define DIFF_A3_A1_1X     ADC_CH(0x2E)
#define DIFF_A3_A1_20X    ADC_CH(0x2F)
#define DIFF_A3_A2_1X     ADC_CH(0x30)
#define DIFF_A3_A2_20X    ADC_CH(0x31)
#define DIFF_A3_A3_1X     ADC_CH(0x24)
#define DIFF_A3_A3_20X    ADC_CH(0x25)
#define DIFF_A3_A4_1X     ADC_CH(0x12)
#define DIFF_A3_A4_20X    ADC_CH(0x13)
#define DIFF_A3_A5_1X     ADC_CH(0x14)
#define DIFF_A3_A5_20X    ADC_CH(0x15)
#define DIFF_A3_A6_1X     ADC_CH(0x16)
#define DIFF_A3_A6_20X    ADC_CH(0x17)
#define DIFF_A3_A7_1X     ADC_CH(0x18)
#define DIFF_A3_A7_20X    ADC_CH(0x19)
#define DIFF_A4_A3_1X     ADC_CH(0x32)
#define DIFF_A4_A3_20X    ADC_CH(0x33)
#define DIFF_A4_A5_1X     ADC_CH(0x1A)
#define DIFF_A4_A5_20X    ADC_CH(0x1B)
#define DIFF_A5_A3_1X     ADC_CH(0x34)
#define DIFF_A5_A3_20X    ADC_CH(0x35)
#define DIFF_A5_A4_1X     ADC_CH(0x3A)
#define DIFF_A5_A4_20X    ADC_CH(0x3B)
#define DIFF_A5_A6_1X     ADC_CH(0x1C)
#define DIFF_A5_A6_20X    ADC_CH(0x1D)
#define DIFF_A6_A3_1X     ADC_CH(0x36)
#define DIFF_A6_A3_20X    ADC_CH(0x37)
#define DIFF_A6_A5_1X     ADC_CH(0x3C)
#define DIFF_A6_A5_20X    ADC_CH(0x3D)
#define DIFF_A6_A7_1X     ADC_CH(0x1E)
#define DIFF_A6_A7_20X    ADC_CH(0x1F)
#define DIFF_A7_A3_1X     ADC_CH(0x38)
#define DIFF_A7_A3_20X    ADC_CH(0x39)
#define DIFF_A7_A6_1X     ADC_CH(0x3E)
#define DIFF_A7_A6_20X    ADC_CH(0x3F)
#define DIFF_A7_A7_1X     ADC_CH(0x26)
#define DIFF_A7_A7_20X    ADC_CH(0x27)

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          1
#define ANALOG_COMP_AIN1_BIT          2


/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a USI, not an SPI or TWI module. Accordingly, there is no
 * MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as
 * master, DI is used as MISO, DO is MOSI; the defines here tell it where the
 * USI pins are. These are used in Arduino.h to generate MISO/MOSI/SCK for SPI
 * (this is for master mode, as there isn't support for SPI Slave in stock
 * SPI.h) and master mode is almost always what people want. A USI SPI slave
 * library should use the USI_role defines. The MISO/MOSI/SCK defines are
 * required for compatibility anyway.
 * Also, be aware that the MISO and MOSI markings on the pinout diagram in the
 * datasheet are for ISP programming, where the chip is the slave. The pinout
 * diagram included with this core attempts to clarify this,
 * The SS pin is chosen arbitrarily - libraries acting as master often expect
 * there to be an SS pin defined, and will throw errors if there isn't one.
 * Since we provide an SPI.h that mimics the interface of the standard one
 * we also provide a dummy SS pin macro. MISO/MOSI/SCK, SDA, SCL #defines
 * are in Arduino.h and refer back to these macros (USI_* )
 *---------------------------------------------------------------------------*/

#define USE_SOFTWARE_SPI 1

/* USI */
#define USI_DI                PIN_PA6
#define USI_DO                PIN_PA5
#define USI_SCK               PIN_PA4
#define SS                    PIN_PA3

#define USI_DDR               DDRA
#define USI_PORT              PORTA
#define USI_PIN               PINA

#define USI_CLOCK_BIT         PINA4
#define USI_DO_BIT            PINA5
#define USI_DI_BIT            PINA6

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY84/44/24  ATTinyCore Standard (Clockwise) Pin Mapping
 *
 *                         +-\/-+
 *                   VCC  1|    |14  GND
 *             (10)  PB0  2|x  a|13  PA0  ( 0)  AREF
 *             ( 9)  PB1  3|x  a|12  PA1  ( 1)
 *             (11)  PB3  4|   a|11  PA2  ( 2)
 *  PWM  INT0  ( 8)  PB2  5|   a|10  PA3  ( 3)
 *  PWM        ( 7)  PA7  6|a  a|9   PA4  ( 4)
 *  PWM        ( 6)  PA6  7|a  a|8   PA5  ( 5)  PWM
 *                         +----+
 * a indicates ADC pin
 * x indicates XTAL pin
 *---------------------------------------------------------------------------*/

#warning "This is the CLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in clockwise order."

const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRA,
  (uint8_t)(uint16_t) &DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTA,
  (uint8_t)(uint16_t) &PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINA,
  (uint8_t)(uint16_t) &PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
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
  PB, /* 11 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* port A */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(2), /* port B */
  _BV(1),
  _BV(0),
  _BV(3),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1B, /* OC1B */
  TIMER1A, /* OC1A */
  TIMER0B, /* OC0B */
  TIMER0A, /* OC0A */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};

#endif

#endif
