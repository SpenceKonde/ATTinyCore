/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny4313, ATtiny2313
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 *---------------------------------------------------------------------------*/

#define ATTINYX313 1       // backwards compatibility
#define __AVR_ATtinyX313__ // recommended

/* These cause problems later */
#undef UDR0
#undef UDR1
#undef UDR2
#undef UDR3
#undef UDR4
#undef UDR5
#undef UDR6
#undef UDR7

#define NUM_DIGITAL_PINS            (18)
#define NUM_ANALOG_INPUTS           ( 0)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
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


#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PB4)
#endif

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)        (((p) >= 9 && (p) <= 16) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (5)
#define digitalPinToPCMSK(p)        (((p) >= 9 && (p) <= 16) ? (&PCMSK) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)     ((p) - 9)

#define digitalPinToInterrupt(p)    ((p) == 5 ? 1 : ((p) == 4 ? 0: NOT_AN_INTERRUPT))

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (NOT_A_PIN)
#define digitalPinToAnalogInput(p)  (NOT_A_PIN)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == 7 || (p) == 11 || (p) == 12 || (p) == 13)

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// We have no ADC
#define DEFAULT_INITIALIZE_ADC                 0
// We have hardware serial, so don't use soft serial.
// #define USE_SOFTWARE_SERIAL                 0
// Note that there is a menu option to override this
#ifndef DEFAULT_INITIALIZE_SECONDARY_TIMERS
  #define DEFAULT_INITIALIZE_SECONDARY_TIMERS  0
#endif
#define USE_SOFTWARE_SERIAL                    0
// We have a hardware serial port... and not much else.

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
#define PIN_TIMER_OC0A              (PIN_PD5)
#define PIN_TIMER_OC0B              (PIN_PB2)
#define PIN_TIMER_T0                (PIN_PD4)

/* Timer 1 - 16-bit timer with PWM */
#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PB3)
#define PIN_TIMER_OC1B              (PIN_PB4)
#define PIN_TIMER_T1                (PIN_PD5)
#define PIN_TIMER_ICP1              (PIN_PD6)

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------*/

/* Part has NO ADC */

/* Analog Comparator - not used by core */
#define ANALOG_COMP_DDR             DDRB
#define ANALOG_COMP_PORT            PORTB
#define ANALOG_COMP_PIN             PINB
#define ANALOG_COMP_AIN0_BIT        0
#define ANALOG_COMP_AIN1_BIT        1

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

#define USE_SOFTWARE_SPI      1

/* USI */
#define USI_DI                PIN_PB5
#define USI_DO                PIN_PB6
#define USI_SCK               PIN_PB7
#define SS                    PIN_PB1

#define USI_DDR               DDRB
#define USI_PORT              PORTB
#define USI_PIN               PINB

#define USI_CLOCK_BIT         PINB7
#define USI_DO_BIT            PINB6
#define USI_DI_BIT            PINB5

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

/* One hardware serial port */
#define PIN_HWSERIAL0_TX      PIN_PD1
#define PIN_HWSERIAL0_RX      PIN_PD0

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY2313 ATTinyCore Standard Pin Mapping
 *
 *                  +-\/-+
 * RESET (17) PA2  1|    |20  VCC
 *    RX ( 0) PD0  2|    |19  PB7 (16) SCK  SCL
 *    TX ( 1) PD1  3|    |18  PB6 (15) DO
 * XTAL1 ( 2) PA1  4|    |17  PB5 (14) DI   SDA
 * XTAL2 ( 3) PA0  5|    |16  PB4 (13) PWM
 *  INT0 ( 4) PD2  6|    |15  PB3 (12) PWM
 *  INT1 ( 5) PD3  7|    |14  PB2 (11) PWM
 *       ( 6) PD4  8|    |13  PB1 (10)
 *   PWM ( 7) PD5  9|    |12  PB0 ( 9)
 *            GND 10|    |11  PD6 ( 8)
 *                  +----+
 *---------------------------------------------------------------------------*/


const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRA,
  (uint8_t)(uint16_t) &DDRB,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTA,
  (uint8_t)(uint16_t) &PORTB,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINA,
  (uint8_t)(uint16_t) &PINB,
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
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

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
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

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
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

//The 2313 2313a and 4313 use different names for a few things in the headers. This makes sure all of them are there for this family if lousy, obsolete garbage parts.
#ifndef DDA0
  #define DDA0 DDRA0
  #define DDA1 DDRA1
  #define DDA2 DDRA2
#endif
#ifdef PCMSK1
  #ifndef PCMSK0
    #define PCMSK0 PCMSK
  #endif
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
