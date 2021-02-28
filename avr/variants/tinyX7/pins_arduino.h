/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny87, ATtiny167
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros | Combatibility mapping
 *-----------------------------------------------------------------
 * This is an horrifying legacy pin mapping, it is so bad that there
 * are significant effects on code size and performance.
 * Do not use this unless you have a gun to your head, and even then
 * you might come to regret using it!
 *-----------------------------------------------------------------*/

#define ATTINYX7 1       //backwards compat
#define __AVR_ATtinyX7__ //recommended

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           11


#define PIN_PA0  ( 0)
#define PIN_PA1  ( 1)
#define PIN_PB6  ( 2)
#define PIN_PA3  ( 3)
#define PIN_PB0  ( 4)
#define PIN_PB1  ( 5)
#define PIN_PB2  ( 6)
#define PIN_PB3  ( 7)
#define PIN_PB4  ( 8)
#define PIN_PB5  ( 9)
#define PIN_PA6  (10)
#define PIN_PA4  (11)
#define PIN_PA2  (12)
#define PIN_PA5  (13)
#define PIN_PA7  (14)
#define PIN_PB7  (15)

#define LED_BUILTIN (PIN_PB0)

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

#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (digitalPinToPort(p) == PA ? 4 : 5)
#define digitalPinToPCMSK(p)   ((((p) >= 0 && (p) <= 15) ? (digitalPinToPort(p) == PA ? &PCMSK0 : &PCMSK1) : (uint8_t *)NULL))
#define digitalPinToPCMSKbit(p) (((p) >= 0 && (p) <= 15) ? (const_array_or_pgm_(pgm_read_byte, digital_pin_to_bit_pos_PGM, (p))) : -1)

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p)==11?1: NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  (((p) >= NUM_DIGITAL_PINS)  ? NOT_A_PIN : (const_array_or_pgm_(pgm_read_byte, analog_input_to_digital_pin_PGM, (p))))
#define digitalPinToAnalogInput(p)  (((p) >= NUM_ANALOG_INPUTS) ? NOT_A_PIN : (const_array_or_pgm_(pgm_read_byte, digital_pin_to_analog_input_PGM, (p))))

#define digitalPinHasPWM(p)         ((p) == 2 || ((p) > 3 && (p) < 10) || (p) == 15 )

#define PINMAPPING_OLD


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
//     RX ( 0) PA0  1|    |20  PB0 ( 4)*
//     TX ( 1) PA1  2|    |19  PB1 ( 5)*
//       *(12) PA2  3|    |18  PB2 ( 6)*
//        ( 3) PA3  4|    |17  PB3 ( 7)*
//            AVCC  5|    |16  GND
//            AGND  6|    |15  VCC
//   INT1 (11) PA4  7|    |14  PB4 ( 8)*
//        (13) PA5  8|    |13  PB5 ( 9)*
//        (10) PA6  9|    |12  PB6 ( 2)* INT0
//        (14) PA7 10|    |11  PB7 (15)* Reset
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

const uint8_t PROGMEM digital_pin_to_analog_input_PGM[] =
{
  0, /* 0 */
  1,
  9, /* 2 */
  3, /* 3 */
  NOT_A_PIN, /* 4 */
  NOT_A_PIN,
  NOT_A_PIN,
  NOT_A_PIN,
  NOT_A_PIN,
  8,
  6, /* 10 */
  4,
  2,
  5,
  7,
  10 /* 15 */
};

// Wow, a core finally did such a perverse job of distributing pins that I needed to make a progmem array to sort it out...
const uint8_t PROGMEM analog_input_to_digital_pin_PGM [] =
{
  0,  /* 0 */
  1,
  12, /* 2 */
  3,  /* 3 */
  11, /* 4 */
  13,
  10,
  14,
  9,
  2,
  15, /* 10 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(6), /* 2 */
  _BV(3), /* 3 */
  _BV(0), /* 4 */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6), /* 10 */
  _BV(4),
  _BV(2),
  _BV(5),
  _BV(7),
  _BV(7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_bit_pos_PGM[] =
{
  (0), /* 0 */
  (1),
  (6), /* 2 */
  (3), /* 3 */
  (0), /* 4 */
  (1),
  (2),
  (3),
  (4),
  (5),
  (6), /* 10 */
  (4),
  (2),
  (5),
  (7),
  (7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIM1AX,
  NOT_ON_TIMER,
  TIM1AU,
  TIM1BU,
  TIM1AV,
  TIM1BV,
  TIM1AW,
  TIM1BW,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIM1BX,
};


#endif

#endif
