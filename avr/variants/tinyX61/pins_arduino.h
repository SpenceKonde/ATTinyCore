/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny861A, ATtiny461A, and ATtiny261A and non-A versions
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros | Compatibility mapping
 *-----------------------------------------------------------------
 * This is an AWFUL legacy pin mapping, a work of inspired evil!
 * Use the new one
 *-----------------------------------------------------------------*/

#define ATTINYX61 1  //backwards compatibility
#define __AVR_ATtinyX61__ //recommended

#define NUM_DIGITAL_PINS            (16)
#define NUM_ANALOG_INPUTS           (11)

#define PIN_PA0     ( 0)
#define PIN_PA1     ( 1)
#define PIN_PA2     ( 2)
#define PIN_PA3     (14)
#define PIN_PA4     (10)
#define PIN_PA5     (11)
#define PIN_PA6     (12)
#define PIN_PA7     (13)
#define PIN_PB0     ( 9)
#define PIN_PB1     ( 8)
#define PIN_PB2     ( 7)
#define PIN_PB3     ( 6)
#define PIN_PB4     ( 5)
#define PIN_PB5     ( 4)
#define PIN_PB6     ( 3)
#define PIN_PB7     (15) /* RESET */

#define LED_BUILTIN (PIN_PB6)

//these are like PIN_An official core, the digital pin corresponding to a given analog channel.
#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA4)
#define PIN_A4      (PIN_PA5)
#define PIN_A5      (PIN_PA6)
#define PIN_A6      (PIN_PA7)
#define PIN_A7      (PIN_PB4)
#define PIN_A8      (PIN_PB5)
#define PIN_A9      (PIN_PB6)
#define PIN_A10     (PIN_PB7)

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

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) >= 6 && (p) <=  9) ? 4 : 5)
#define digitalPinToPCMSK(p)       ((((p) >= 0 && (p) <=  2) || ((p) >= 10 && (p) <= 14)) ? (&PCMSK0) : ((((p) >= 3 && (p) <= 9) || ((p) == 15)) ? (&PCMSK1) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p)     (((p) >= 0 && (p) <=  2) ? (p) :(((p) >= 10 && (p) <= 13) ? ((p) - 6) : (((p) == 14) ? (3) : (((p) >= 3 && (p) <= 9) ? (9 - (p)) : (7)))))

#define digitalPinToInterrupt(p)     ((p) == PIN_PB6 ? 0 : ((p)==PIN_PA2? 1 : NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  ((p < 3) ? (p): (((p) >= 3 && (p) <= 6) ? ((p) + 7) : (((p) >= 7 && (p) <= 9) ? (12 - (p)) : -1)))
#define digitalPinToAnalogInput(p)  ((p < 3) ? (p): ((p) >= 10 && (p) <= 13 ) ? ((p) - 7) : (((p) < 6 ) ? (12 - (p)) : ((p == 15) ? 10: -1 )))

#define digitalPinHasPWM(p)          ((p) == PIN_PB1 || (p) == PIN_PB3 || (p) == PIN_PB5)

#define PINMAPPING_LEGACY


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
#define ANALOG_COMP_AIN0_BIT          6
#define ANALOG_COMP_AIN1_BIT          7

/*  Analog reference bit masks. */
// X 0 0 VCC used as Voltage Reference, disconnected from PB0 (AREF).
#define DEFAULT (0)
// X 0 1 External Voltage Reference at PB0 (AREF) pin, Internal Voltage Reference turned off.
#define EXTERNAL (1)
// 0 1 0 Internal 1.1V Voltage Reference.
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL
// 1 1 1 Internal 2.56V Voltage Reference with external bypass capacitor at PB0 (AREF) pin(1).
#define INTERNAL2V56 (7)
// 1 1 0 Internal 2.56V Voltage Reference without external bypass capacitor, disconnected from PB0 (AREF)(1).
#define INTERNAL2V56_NO_CAP (6)
#define INTERNAL2V56NOBP INTERNAL2V56_NO_CAP


/* Special Analog Channels */
#define ADC_INTERNAL1V1   ADC_CH(0x1E)
#define ADC_GROUND        ADC_CH(0x1F)
#define ADC_TEMPERATURE   ADC_CH(0x3F)

/* Differential Analog Channels */
#define DIFF_A0_A1_20X    ADC_CH(0x0B)
#define DIFF_A0_A1_1X     ADC_CH(0x0C)
#define DIFF_A1_A1_20X    ADC_CH(0x0D)
#define DIFF_A2_A1_20X    ADC_CH(0x0E)
#define DIFF_A2_A1_1X     ADC_CH(0x0F)
#define DIFF_A2_A3_1X     ADC_CH(0x10)
#define DIFF_A3_A3_20X    ADC_CH(0x11)
#define DIFF_A4_A3_20X    ADC_CH(0x12)
#define DIFF_A4_A3_1X     ADC_CH(0x13)
#define DIFF_A4_A5_20X    ADC_CH(0x14)
#define DIFF_A4_A5_1X     ADC_CH(0x15)
#define DIFF_A5_A5_20X    ADC_CH(0x16)
#define DIFF_A6_A5_20X    ADC_CH(0x17)
#define DIFF_A6_A5_1X     ADC_CH(0x18)
#define DIFF_A8_A9_20X    ADC_CH(0x19)
#define DIFF_A8_A9_1X     ADC_CH(0x1A)
#define DIFF_A9_A9_20X    ADC_CH(0x1B)
#define DIFF_A10_A9_20X   ADC_CH(0x1C)
#define DIFF_A10_A9_1X    ADC_CH(0x1D)
/* These support gain selection GSEL
 * is passed as high bit of the channel
 * so that analogRead still works  */
#define DIFF_A0_A1_20X    ADC_CH(0x20)
#define DIFF_A0_A1_32X    ADC_CH(0x60)
#define DIFF_A0_A1_1X     ADC_CH(0x21)
#define DIFF_A0_A1_8X     ADC_CH(0x61)
#define DIFF_A1_A0_20X    ADC_CH(0x22)
#define DIFF_A1_A0_32X    ADC_CH(0x62)
#define DIFF_A1_A0_1X     ADC_CH(0x23)
#define DIFF_A1_A0_8X     ADC_CH(0x63)
#define DIFF_A1_A2_20X    ADC_CH(0x24)
#define DIFF_A1_A2_32X    ADC_CH(0x64)
#define DIFF_A1_A2_1X     ADC_CH(0x25)
#define DIFF_A1_A2_8X     ADC_CH(0x65)
#define DIFF_A2_A1_20X    ADC_CH(0x26)
#define DIFF_A2_A1_32X    ADC_CH(0x66)
#define DIFF_A2_A1_1X     ADC_CH(0x27)
#define DIFF_A2_A1_8X     ADC_CH(0x67)
#define DIFF_A2_A0_20X    ADC_CH(0x28)
#define DIFF_A2_A0_32X    ADC_CH(0x68)
#define DIFF_A2_A0_1X     ADC_CH(0x29)
#define DIFF_A2_A0_8X     ADC_CH(0x69)
#define DIFF_A0_A2_20X    ADC_CH(0x2A)
#define DIFF_A0_A2_32X    ADC_CH(0x6A)
#define DIFF_A0_A2_1X     ADC_CH(0x2B)
#define DIFF_A0_A2_8X     ADC_CH(0x6B)
#define DIFF_A4_A5_20X    ADC_CH(0x2C)
#define DIFF_A4_A5_32X    ADC_CH(0x6C)
#define DIFF_A4_A5_1X     ADC_CH(0x2D)
#define DIFF_A4_A5_8X     ADC_CH(0x6D)
#define DIFF_A5_A4_20X    ADC_CH(0x2E)
#define DIFF_A5_A4_32X    ADC_CH(0x6E)
#define DIFF_A5_A4_1X     ADC_CH(0x2F)
#define DIFF_A5_A4_8X     ADC_CH(0x6F)
#define DIFF_A5_A6_20X    ADC_CH(0x30)
#define DIFF_A5_A6_32X    ADC_CH(0x70)
#define DIFF_A5_A6_1X     ADC_CH(0x31)
#define DIFF_A5_A6_8X     ADC_CH(0x71)
#define DIFF_A6_A5_20X    ADC_CH(0x32)
#define DIFF_A6_A5_32X    ADC_CH(0x72)
#define DIFF_A6_A5_1X     ADC_CH(0x33)
#define DIFF_A6_A5_8X     ADC_CH(0x73)
#define DIFF_A6_A4_20X    ADC_CH(0x34)
#define DIFF_A6_A4_32X    ADC_CH(0x74)
#define DIFF_A6_A4_1X     ADC_CH(0x35)
#define DIFF_A6_A4_8X     ADC_CH(0x75)
#define DIFF_A4_A6_20X    ADC_CH(0x36)
#define DIFF_A4_A6_32X    ADC_CH(0x76)
#define DIFF_A4_A6_1X     ADC_CH(0x37)
#define DIFF_A4_A6_8X     ADC_CH(0x77)
#define DIFF_A0_A0_20X    ADC_CH(0x38)
#define DIFF_A0_A0_32X    ADC_CH(0x78)
#define DIFF_A0_A0_1X     ADC_CH(0x39)
#define DIFF_A0_A0_8X     ADC_CH(0x79)
#define DIFF_A1_A1_20X    ADC_CH(0x3A)
#define DIFF_A1_A1_32X    ADC_CH(0x7A)
#define DIFF_A2_A2_20X    ADC_CH(0x3B)
#define DIFF_A2_A2_32X    ADC_CH(0x7B)
#define DIFF_A4_A4_20X    ADC_CH(0x3C)
#define DIFF_A4_A4_32X    ADC_CH(0x7C)
#define DIFF_A5_A5_20X    ADC_CH(0x3D)
#define DIFF_A5_A5_32X    ADC_CH(0x7D)
#define DIFF_A6_A6_20X    ADC_CH(0x3E)
#define DIFF_A6_A6_32X    ADC_CH(0x7E)

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

#define PIN_USI_DI      PIN_PB0
#define PIN_USI_DO      PIN_PB1
#define PIN_USI_SCK     PIN_PB2
#define SS              PIN_PB3

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



#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY861
//
//                   +-\/-+
//       *( 9) PB0  1|    |20  PA0 ( 0)
//       *( 8) PB1  2|    |19  PA1 ( 1)
//       *( 7) PB2  3|    |18  PA2 ( 2) INT1
//       *( 6) PB3  4|    |17  PA3 (14)
//             VCC  5|    |16  AGND
//             GND  6|    |15  AVCC
//       *( 5) PB4  7|    |14  PA4 (10)
//       *( 4) PB5  8|    |13  PA5 (11)
//   INT0 ( 3) PB6  9|    |12  PA6 (12)
//        (15) PB7 10|    |11  PA7 (13)
//                   +----+
//

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
  PA,
  PB, /* 3 */
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PA, /* 10 */
  PA,
  PA,
  PA,
  PA,
  PB, /* 15 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(2),
  _BV(6), /* 3 */
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2),
  _BV(1),
  _BV(0),
  _BV(4), /* 10 */
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(3),
  _BV(7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1D,
  NOT_ON_TIMER,
  TIMER1B,
  NOT_ON_TIMER,
  TIMER1A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};

#endif

#endif




//Old code, just here for temporary backup until I decide it is not needed.
//WARNING, if using software, RX must be on a pin which has a Pin change interrupt <= 7 (e.g. PCINT6, or PCINT1, but not PCINT8)
/*#define USE_SOFTWARE_SERIAL           1
//These are set to match Optiboot pins.

#define SOFTWARE_SERIAL_PORT            PORTB
#define SOFTWARE_SERIAL_TX              0
#define SOFTWARE_SERIAL_PIN             PINB
#define SOFTWARE_SERIAL_RX              1*/
