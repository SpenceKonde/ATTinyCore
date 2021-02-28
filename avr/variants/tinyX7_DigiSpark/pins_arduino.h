/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny167, ATtiny87 "Digispark Pro"
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros | Digispark mapping
 *-----------------------------------------------------------------
 * Use this pin mapping only if you are working with Digispark Pro
 * hardware (because then the numbers will match the markings on the
 * Digispark). Otherwise, use the "New" rationalized pin mapping.
 *-----------------------------------------------------------------*/

#define ATTINYX7 1       //backwards compat
#define __AVR_ATtinyX7__ //recommended

#define NUM_DIGITAL_PINS            16
#define NUM_ANALOG_INPUTS           11


#define PIN_PB0   ( 0)
#define PIN_PB1   ( 1)
#define PIN_PB2   ( 2)
#define PIN_PB6   ( 3) // A9
#define PIN_PB3   ( 4)
#define PIN_PA7   ( 5) // A7
#define PIN_PA0   ( 6) // A0
#define PIN_PA1   ( 7) // A1
#define PIN_PA2   ( 8) // A2
#define PIN_PA3   ( 9) // A3
#define PIN_PA4   (10) // A4
#define PIN_PA5   (11) // A5
#define PIN_PA6   (12) // A6
#define PIN_PB7   (13) // A10
#define PIN_PB4   (14)
#define PIN_PB5   (15) // A8

#define LED_BUILTIN (PIN_PB1)

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

#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) ( ((p) >= 5 && (p) <= 12) ? PCIE0 : PCIE1 )
#define digitalPinToPCMSK(p)    ( ((p) >= 5 && (p) <= 12) ? (&PCMSK0) : (&PCMSK1) )
#define digitalPinToPCMSKbit(p) ( (((p) >= 0) && ((p) <= 2))  ? (p) :       \
                                ( (((p) >= 6) && ((p) <= 13)) ? ((p) - 6) : \
                                ( ((p) == 3) ? 6 :                          \
                                ( ((p) == 4) ? 3 :                          \
                                ( 7) ) ) ) ) /* pin 5 */


#define digitalPinToInterrupt(p)  ((p) == PIN_PB6 ? 0 : ((p)==PIN_PA3?1: NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  (((p == 9) ? 3 : (p == 7) ? 5 : (p < 13 && p > 5) ? (p-6) : (p ==13) ? 13 : -1))
#define digitalPinToAnalogInput(p)  (((p) > 5) && ((p < 13)) ? (p) - 6 :(((p) == 5) ? 7 : (((p) == 3) ? 9 : (((p) == 13) ? 10 : -1 ))))

#define digitalPinHasPWM(p)         ((p) == PIN_PA2 || ((p) >= 0 && (p) <= 16 && ((p) < 5 || (p) > 12)))

#define Serial1 Serial

#define PINMAPPING_DIGI


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

// On the DigiSpark board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY167
//
//                   +-\/-+
// RX   (D  6) PA0  1|    |20  PB0 (D  0)
// TX   (D  7) PA1  2|    |19  PB1 (D  1)
//     *(D  8) PA2  3|    |18  PB2 (D  2)
//      (D  9) PA3  4|    |17  PB3 (D  4)*
//            AVCC  5|    |16  GND
//            AGND  6|    |15  VCC
// INT1 (D 10) PA4  7|    |14  PB4 (D 14) XTAL1
//      (D 11) PA5  8|    |13  PB5 (D 15) XTAL2
//      (D 12) PA6  9|    |12  PB6 (D  3)* INT0
//      (D  5) PA7 10|    |11  PB7 (D 13)
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

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PB, /* 0 */
  PB,
  PB, /* 2 */
  PB, /* 3 */
  PB, /* 4 */
  PA,
  PA,
  PA,
  PA,
  PA,
  PA, /* 10 */
  PA,
  PA,
  PB, /* RESET */
  PB,
  PB,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(2), /* 2 */
  _BV(6), /* 3 */
  _BV(3), /* 4 */
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4), /* 10 */
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(4),
  _BV(5),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  TIM1AU,
  TIM1BU,
  TIM1AV,
  TIM1AX,
  TIM1BV,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIM1BX,
  TIM1AW,
  TIM1BW,
};

#endif

#endif
