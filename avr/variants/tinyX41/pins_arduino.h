/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny841, ATtiny441
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros | Counterclockwise mapping
 * CCW/Legacy pin mapping is inferior - it turns several macros into
 * a no-op instead of requiring math. Use the other one if you don't
 * have code or hardware marked with the CCW numbers.
 *-----------------------------------------------------------------*/

#define ATTINYX4 1
#define __AVR_ATtinyX4__
#define USE_SOFTWARE_SPI 1

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            12
#define NUM_ANALOG_INPUTS           12

// Recommended way to reference pins
// PIN_Pxn is bit n of PORTx.
#define PIN_PA0  (10)
#define PIN_PA1  ( 9)
#define PIN_PA2  ( 8)
#define PIN_PA3  ( 7)
#define PIN_PA4  ( 6)
#define PIN_PA5  ( 5)
#define PIN_PA6  ( 4)
#define PIN_PA7  ( 3)
#define PIN_PB0  ( 0)
#define PIN_PB1  ( 1)
#define PIN_PB2  ( 2)
#define PIN_PB3  (11)  /* RESET */

#define LED_BUILTIN (PIN_PB2)

// PIN_An is the digital pin with analog channel An on it.
#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA3)
#define PIN_A4      (PIN_PA4)
#define PIN_A5      (PIN_PA5)
#define PIN_A6      (PIN_PA6)
#define PIN_A7      (PIN_PA7)
#define PIN_A8      (PIN_PB2)
#define PIN_A9      (PIN_PB3) /* RESET */
#define PIN_A10     (PIN_PB0)
#define PIN_A11     (PIN_PB1)

// An "analog pins" these map directly to analog channels
static const uint8_t A0  = ADC_CH(0);
static const uint8_t A1  = ADC_CH(1);
static const uint8_t A2  = ADC_CH(2);
static const uint8_t A3  = ADC_CH(3);
static const uint8_t A4  = ADC_CH(4);
static const uint8_t A5  = ADC_CH(5);
static const uint8_t A6  = ADC_CH(6);
static const uint8_t A7  = ADC_CH(7);
static const uint8_t A8  = ADC_CH(8);
static const uint8_t A9  = ADC_CH(9);
static const uint8_t A10 = ADC_CH(10);
static const uint8_t A11 = ADC_CH(11);


#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 10) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) <= 2) ? PCIE1 : PCIE0)
#define digitalPinToPCMSK(p)    (((p) <= 2) ? (&PCMSK1) : (((p) <= 10) ? (&PCMSK0) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) <= 2) ? (p) : (10 - (p)))

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : NOT_AN_INTERRUPT)

#define analogInputToDigitalPin(p)  (TODO)
#define digitalPinToAnalogInput(p)  (TODO)

#define digitalPinHasPWM(p)         ((p) > TODO && (p) < TODO)

#define PINMAPPING_CCW

//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

// Choosing not to initialise saves power and flash. 1 = initialise.
//#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
//#define INITIALIZE_SECONDARY_TIMERS               1

//#define TIMER_TO_USE_FOR_MILLIS                   0

#define USE_SOFTWARE_SERIAL           0


/*----------------------------------------------------------
 * Chip Features - Analog stuff
 *----------------------------------------------------------*/

/* Analog Comparator not listed because not used for software serial */

/* Analog reference bit masks. */
#define DEFAULT               (0x00)
#define INTERNAL1V1_NO_CAP    (0x01)
#define INTERNAL2V2_NO_CAP    (0x02)
#define INTERNAL4V096_NO_CAP  (0x03)
#define EXTERNAL              (0x04)
#define INTERNAL1V1_CAP       (0x05)
#define INTERNAL2V2_CAP       (0x06)
#define INTERNAL4V096_CAP     (0x07)
#define INTERNAL              INTERNAL1V1_NO_CAP
#define INTERNAL1V1           INTERNAL1V1_NO_CAP
#define INTERNAL2V2           INTERNAL2V2_NO_CAP
#define INTERNAL4V096         INTERNAL4V096_NO_CAP
#define INTERNAL4V1           INTERNAL4V096_NO_CAP
#define INTERNAL4V            INTERNAL4V096_NO_CAP   /* deprecated */

/* Gain Options */
#define GAIN_1X               (0x00)
#define GAIN_20X              (0x01)
#define GAIN_100X             (0x02)
// 0x03 is not a valid option for the GSEL bits.

/* Special Single-Ended Channels */
#define ADC_GROUND      ADC_CH(0x20)
#define ADC_INTERNAL1V1 ADC_CH(0x21)
#define ADC_TEMPERATURE ADC_CH(0x22)

/* Differential Channels */
#define DIFF_A0_A1      ADC_CH(0x10)
#define DIFF_A0_A3      ADC_CH(0x11)
#define DIFF_A1_A2      ADC_CH(0x12)
#define DIFF_A1_A3      ADC_CH(0x13)
#define DIFF_A2_A3      ADC_CH(0x14)
#define DIFF_A3_A4      ADC_CH(0x15)
#define DIFF_A3_A5      ADC_CH(0x16)
#define DIFF_A3_A6      ADC_CH(0x17)
#define DIFF_A3_A7      ADC_CH(0x18)
#define DIFF_A4_A5      ADC_CH(0x19)
#define DIFF_A4_A6      ADC_CH(0x1A)
#define DIFF_A4_A7      ADC_CH(0x1B)
#define DIFF_A5_A6      ADC_CH(0x1C)
#define DIFF_A5_A7      ADC_CH(0x1D)
#define DIFF_A6_A7      ADC_CH(0x1E)
#define DIFF_A8_A9      ADC_CH(0x1F)
#define DIFF_A0_A0      ADC_CH(0x20)
#define DIFF_A1_A1      ADC_CH(0x21)
#define DIFF_A2_A2      ADC_CH(0x22)
#define DIFF_A3_A3      ADC_CH(0x23)
#define DIFF_A4_A4      ADC_CH(0x24)
#define DIFF_A5_A5      ADC_CH(0x25)
#define DIFF_A6_A6      ADC_CH(0x26)
#define DIFF_A7_A7      ADC_CH(0x27)
#define DIFF_A8_A8      ADC_CH(0x28)
#define DIFF_A9_A9      ADC_CH(0x29)
#define DIFF_A10_A8     ADC_CH(0x2A)
#define DIFF_A10_A9     ADC_CH(0x2B)
#define DIFF_A11_A8     ADC_CH(0x2C)
#define DIFF_A11_A9     ADC_CH(0x2D)
#define DIFF_A1_A0      ADC_CH(0x30)
#define DIFF_A3_A0      ADC_CH(0x31)
#define DIFF_A2_A1      ADC_CH(0x32)
#define DIFF_A3_A1      ADC_CH(0x33)
#define DIFF_A3_A2      ADC_CH(0x34)
#define DIFF_A4_A3      ADC_CH(0x35)
#define DIFF_A5_A3      ADC_CH(0x36)
#define DIFF_A6_A3      ADC_CH(0x37)
#define DIFF_A7_A3      ADC_CH(0x38)
#define DIFF_A5_A4      ADC_CH(0x39)
#define DIFF_A6_A4      ADC_CH(0x3A)
#define DIFF_A7_A4      ADC_CH(0x3B)
#define DIFF_A6_A5      ADC_CH(0x3C)
#define DIFF_A7_A5      ADC_CH(0x3D)
#define DIFF_A7_A6      ADC_CH(0x3E)
#define DIFF_A9_A8      ADC_CH(0x3F)

/*----------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *----------------------------------------------------------*/


/*  This part has a real SPI module and a slave-only TWI module
    The included Wire.h will use the TWI hardware for TWI slave, or
    a markedly inferior software TWI master implementation if that is requested.
    TWI master is one of the few roles that the ATtiny841 does not excel in */


//#define USE_SOFTWARE_SPI 0

#define SCL  PIN_PA6
#define SDA  PIN_PA4

#define MISO PIN_PA5
#define MOSI PIN_PA6
#define SCK  PIN_PA4
#define SS   PIN_PA7


#ifdef ARDUINO_MAIN
#warning "This is the COUNTERCLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in counter clockwise order"
// ATMEL ATTINY841 /ATTinyCore Legacy/CCW
//
//                                +-\/-+
//                          VCC  1|    |14  GND
//                 (A11 0)  PB0  2|    |13  PA0  (A0 10)     *MISO   AREF
//                 (A10 1)  PB1  3|    |12  PA1  (A1  9) TX0 *MOSI   *PWM
//           RESET (A9 11)  PB3  4|    |11  PA2  (A2  8) RX0 *SS     *PWM
//  PWM INT0  *RX0    ( 2)  PB2  5|    |10  PA3  (A3  7)     *SCK     PWM
//  PWM SS    *TX0 (A7  3)  PA7  6|    |9   PA4  (A4  6) RX1 SCK  SCL PWM
//  PWM SDA   MOSI (A6  4)  PA6  7|    |8   PA5  (A5  5) TX1 MISO     PWM
//                                +----+
//
//  Pins can be remapped, optional remap options denoted by *; these are not enabled by default.

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
  PA, /* 3 */
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PB, /* 11 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 PB0 */
  _BV(1),
  _BV(2),
  _BV(7), /* 3 PA7 */
  _BV(6),
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2),
  _BV(1),
  _BV(0),
  _BV(3), /* RESET PB3 */
};


const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TOCC7 | TIMER2A,  /* OCR2A */
  TOCC6 | TIMER2B,  /* OCR2B */
  TOCC5 | TIMER1A,  /* OCR1A */
  TOCC4 | TIMER0B,  /* OCR0B this is serial 1 TX */
  TOCC3 | TIMER0A,  /* OCR0A this is serial 1 RX, too, so let's give it the least desirable timer */
  TOCC2 | TIMER1B,  /* OCR1B */
  TOCC1 | NOT_ON_TIMER,   /* this is serial 0 RX */
  TOCC0 | NOT_ON_TIMER,   /* this is serial 0 TX, lets not use it for pwm */
  NOT_ON_TIMER,
  NOT_ON_TIMER      /* RESET */
};

#endif
#endif


/*
#if defined(SUPER_PWM)
  const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
  {
    NOT_ON_TIMER,
    NOT_ON_TIMER,
    TOCC7,       / OCR2A /
    TOCC6,       / OCR2B /
    TOCC5,       / OCR1A /
    TOCC4,       / OCR0B this is serial 1 TX /
    TOCC3,       / OCR0A this is serial 1 RX, too, so it is initially given least desirable timer /
    TOCC2,       / OCR1B /
    TOCC1,       / OCR0A this is serial 0 RX /
    TOCC0,       / OCR0B this is serial 0 TX, too, so it is initially given least desirable timer /
    NOT_ON_TIMER,
    NOT_ON_TIMER / RESET /
  };
#else
*/
