/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===================================================================
 * Microchip ATtiny4313, ATtiny2313
 *===================================================================
 * Basic Pin Definitions | Interrupt Macros |
 *-----------------------------------------------------------------*/

#define ATTINYX313 1       //backwards compatibility
#define __AVR_ATtinyX313__ //recommended
#define USE_SOFTWARE_SPI 1

#define LED_BUILTIN (13)

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            18
#define NUM_ANALOG_INPUTS           0



//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)



#define PIN_PD0   ( 0)
#define PIN_PD1   ( 1)
#define PIN_PA1   ( 2)
#define PIN_PA0   ( 3)
#define PIN_PD2   ( 4)
#define PIN_PD3   ( 5)
#define PIN_PD4   ( 6)
#define PIN_PD5   ( 7)
#define PIN_PD6   ( 8)
#define PIN_PB0   ( 9)
#define PIN_PB1   (10)
#define PIN_PB2   (11)
#define PIN_PB3   (12)
#define PIN_PB4   (13)
#define PIN_PB5   (14)
#define PIN_PB6   (15)
#define PIN_PB7   (16)
#define PIN_PA2   (17)



#define digitalPinToPCICR(p)        (((p) >= 9 && (p) <= 16) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (5)
#define digitalPinToPCMSK(p)        (((p) >= 9 && (p) <= 16) ? (&PCMSK) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)     ((p) - 9)

#define digitalPinToInterrupt(p)    ((p) == 5 ? 1 : ((p) == 4 ? 0: NOT_AN_INTERRUPT))

#define analogInputToDigitalPin(p)  (NOT_A_PIN)
#define digitalPinToAnalogInput(p)  (NOT_A_PIN)

#define digitalPinHasPWM(p)         ((p) == 7 || (p) == 11 || (p) == 12 || (p) == 13)

//----------------------------------------------------------
// Core Configuration where these are not the defaults
//----------------------------------------------------------

// Choosing not to initialise saves power and flash. 1 = initialise.
#ifndef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    0
#endif
// Secondary timers - nope, we skip setting up t
// Note that this is double-entry - there is also a menu option whivh takes precedence.
#ifndef INITIALIZE_SECONDARY_TIMERS
  #define INITIALIZE_SECONDARY_TIMERS               0
#endif
// #define TIMER_TO_USE_FOR_MILLIS                   0

#define USE_SOFTWARE_SERIAL           0

/*----------------------------------------------------------
 * Chip Features - Analog stuff
 *----------------------------------------------------------*/

// Analog Comparator will be listed ONLY where it is used for software serial
// Core does not have built-in comparator library, nor will it ever.
/* Part has NO ADC */

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

#define PIN_USI_DI      PIN_PB5
#define PIN_USI_DO      PIN_PB6
#define PIN_USI_SCK     PIN_PB7
#define SS              PIN_PB1

#define USI_DATA_DDR       DDRB
#define USI_DATA_PORT     PORTB
#define USI_DATA_PIN       PINB

#define USI_CLOCK_BIT     PINB7
#define USI_DO_BIT        PINB6
#define USI_DI_BIT        PINB5

#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif


#ifdef ARDUINO_MAIN
// ATMEL ATTINY2313
//
//                  +-\/-+
// RESET (17) PA2  1|    |20  VCC
//    RX ( 0) PD0  2|    |19  PB7 (16) SCK  SCL
//    TX ( 1) PD1  3|    |18  PB6 (15) DO
// XTAL1 ( 2) PA1  4|    |17  PB5 (14) DI   SDA
// XTAL2 ( 3) PA0  5|    |16  PB4 (13) PWM
//  INT0 ( 4) PD2  6|    |15  PB3 (12) PWM
//  INT1 ( 5) PD3  7|    |14  PB2 (11) PWM
//       ( 6) PD4  8|    |13  PB1 (10)
//   PWM ( 7) PD5  9|    |12  PB0 ( 9)
//            GND 10|    |11  PD6 ( 8)
//                  +----+
//

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRA,
  (uint8_t)(uint16_t)&DDRB,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTA,
  (uint8_t)(uint16_t)&PORTB,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PINA,
  (uint8_t)(uint16_t)&PINB,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PD, /* 0 */
  PD,
  PA,
  PA,
  PD,
  PD,
  PD,
  PD,
  PD, /* 8 */
  PB,
  PB,
  PB,
  PB,
  PB,
  PB, /* 14 */
  PB,
  PB,
  PA,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0 */
  _BV(1),
  _BV(1),
  _BV(0),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6), /* 8 */
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5), /* 14 */
  _BV(6),
  _BV(7),
  _BV(2),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  TIMER1A,
  TIMER1B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};
#endif

//The 2313 and 2313a use different names for a few things in the headers. This makes sure all of them are there for all x313 parts...
#ifndef DDA0
#define DDA0 DDRA0
#define DDA1 DDRA1
#define DDA2 DDRA2
#endif
#ifndef DDAR0
#define DDAR0 DDA0
#define DDAR1 DDA1
#define DDAR2 DDA2
#endif
#ifndef EEPROM_Ready_vect
#define EEPROM_Ready_vect EEPROM_READY_vect
#define EEPROM_Ready_vect_num EEPROM_READY_vect_num
#endif
#ifndef EEPROM_READY_vect
#define EEPROM_READY_vect EEPROM_Ready_vect
#define EEPROM_READY_vect_num EEPROM_Ready_vect_num
#endif
#ifndef EEARL
#define EEARL EEAR
#endif
#ifndef GIFR
#define GIFR EIFR
#endif
#ifndef OCR1
#define OCR1 OCR1A
#define OCR1H OCR1AH
#define OCR1L OCR1AL
#endif
#ifndef UMSEL
#define UMSEL UMSEL0
#endif
#ifndef UMSEL0
#define UMSEL0 UMSEL
#endif
#ifndef WDTCR
#define WDTCR WDTCSR
#endif
#ifndef WDTCSR
#define WDTCSR WDTCR
#endif
#ifndef TXB
#define TXB UDR
#define RXB UDR
#endif

#endif
