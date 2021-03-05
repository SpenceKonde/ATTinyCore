/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny84A, ATtiny44A, and ATtiny24A and non-A versions
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros | Clockwise mapping
 *-----------------------------------------------------------------
 * If you have a choice, this is the pin mapping to use.
 *-----------------------------------------------------------------*/


#define ATTINYX4 1
#define __AVR_ATtinyX4__

#define NUM_DIGITAL_PINS            (12)
#define NUM_ANALOG_INPUTS           ( 8)

#define PIN_PA0     ( 0)
#define PIN_PA1     ( 1)
#define PIN_PA2     ( 2)
#define PIN_PA3     ( 3)
#define PIN_PA4     ( 4)
#define PIN_PA5     ( 5)
#define PIN_PA6     ( 6)
#define PIN_PA7     ( 7)
#define PIN_PB0     (10)
#define PIN_PB1     ( 9)
#define PIN_PB2     ( 8)
#define PIN_PB3     (11)   /* RESET */

#define LED_BUILTIN (PIN_PB2)

//these are like PIN_An official core, the digital pin corresponding to a given analog channel.
#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA3)
#define PIN_A4      (PIN_PA4)
#define PIN_A5      (PIN_PA5)
#define PIN_A6      (PIN_PA6)
#define PIN_A7      (PIN_PA7)

static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);
static const uint8_t A4 = ADC_CH(4);
static const uint8_t A5 = ADC_CH(5);
static const uint8_t A6 = ADC_CH(6);
static const uint8_t A7 = ADC_CH(7);

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 11) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) <= 7) ? PCIE0 : PCIE1 )
#define digitalPinToPCMSK(p)        (((p) <= 7) ? (&PCMSK0) : (((p) <= 10) ? (&PCMSK1) : ((uint8_t *)NULL) )
#define digitalPinToPCMSKbit(p)     (((p) <= 7) ? (p) : (10 - (p)))

#define digitalPinToInterrupt(p)    ((p) == PIN_PB2 ? 0 : NOT_AN_INTERRUPT)

#define analogInputToDigitalPin(p)  ((p) < 8 ? (p) : -1)
#define digitalPinToAnalogInput(p)  ((p) < 8 ? (p) : -1)

#define digitalPinHasPWM(p)         ((p) >= 5 && (p) <= 8)

#define PINMAPPING_CW

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

// These are commented out because they are the default option
// That makes it easier to see when something that matters
// is being set - otherwise te exceptions get lost in the
// noise.                                            1 = initialise.
// #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
// #define INITIALIZE_SECONDARY_TIMERS               1
// #define TIMER_TO_USE_FOR_MILLIS                   0


//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT          1
#define ANALOG_COMP_AIN1_BIT          2


/*  Analog reference bit masks. */
// VCC used as analog reference, disconnected from PA0 (AREF)
#define DEFAULT (0)
// External voltage reference at PA0 (AREF) pin, internal reference turned off
#define EXTERNAL (1)
// Internal 1.1V voltage reference
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL

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

#define PIN_USI_DI      PIN_PA6
#define PIN_USI_DO      PIN_PA5
#define PIN_USI_SCK     PIN_PA4
#define SS              PIN_PA3

#define USI_DATA_DDR       DDRA
#define USI_DATA_PORT     PORTA
#define USI_DATA_PIN       PINA

#define USI_CLOCK_BIT     PINA4
#define USI_DO_BIT        PINA5
#define USI_DI_BIT        PINA6

#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif

/*----------------------------------------------------------
 * PROGMEM Pin Mapping Arrays
 *----------------------------------------------------------*/
#ifdef ARDUINO_MAIN
#warning "This is the CLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in clockwise order."

// ATMEL ATTINY84/44/24
//
//                         +-\/-+
//                   VCC  1|    |14  GND
//             (10)  PB0  2|    |13  PA0  ( 0)  AREF
//             ( 9)  PB1  3|    |12  PA1  ( 1)
//             (11)  PB3  4|    |11  PA2  ( 2)
//  PWM  INT0  ( 8)  PB2  5|    |10  PA3  ( 3)
//  PWM        ( 7)  PA7  6|    |9   PA4  ( 4)
//  PWM        ( 6)  PA6  7|    |8   PA5  ( 5)  PWM
//                         +----+

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

// ATMEL ATTINY84 / ARDUINO


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
  PB, /* 11 */
};



const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
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

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
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
