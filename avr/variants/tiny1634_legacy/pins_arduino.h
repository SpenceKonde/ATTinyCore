/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny1634
 *===========================================================================
 * Basic Pin Definitions | Interrupt Macros | CounterClockwise Mapping
 * The Clockwise one is better. Use it unless working with hardware
 * or existing code that uses the old mapping.
 *---------------------------------------------------------------------------*/

// __AVR_ATtiny1634__ is already defined in the io headers.

#define NUM_DIGITAL_PINS            18
#define NUM_ANALOG_INPUTS           12

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PB0     ( 0) // A5 Odd one out
#define PIN_PA7     ( 1) // A4
#define PIN_PA6     ( 2) // A3
#define PIN_PA5     ( 3) // A2
#define PIN_PA4     ( 4) // A1
#define PIN_PA3     ( 5) // A0
#define PIN_PA2     ( 6)
#define PIN_PA1     ( 7)
#define PIN_PA0     ( 8)
#define PIN_PC5     ( 9) // Port C backwards
#define PIN_PC4     (10)
/* skip the reset pin */
#define PIN_PC2     (11) // A11
#define PIN_PC1     (12) // A10
#define PIN_PC0     (13) // A9
#define PIN_PB3     (14) // A8   rest of port B
#define PIN_PB2     (15) // A7
#define PIN_PB1     (16) // A6
#define PIN_PC3     (17) // Reset
#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PC0)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0      (PIN_PA3)
#define PIN_A1      (PIN_PA4)
#define PIN_A2      (PIN_PA5)
#define PIN_A3      (PIN_PA6)
#define PIN_A4      (PIN_PA7)
#define PIN_A5      (PIN_PB0)
#define PIN_A6      (PIN_PB1)
#define PIN_A7      (PIN_PB2)
#define PIN_A8      (PIN_PB3)
#define PIN_A9      (PIN_PC0)
#define PIN_A10     (PIN_PC1)
#define PIN_A11     (PIN_PC2)

/* An "analog pins" these map directly to analog channels */
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

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *
 * Here due to how bulky the pin mapping made these macros, reset is omitted
 * If you really need a PCINT on RESET when it is fused as GPIO (dear lord
 * why), either do it manually or use other pin mapping.
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)      ((((p) >= 0) && ((p) < 17)) ? (volatile uint8_t *)(&GIFR) : ((volatile uint8_t *)NULL))
#define digitalPinToPCICRbit(p)   (((p) > 13 || (p) == 0)? 4 : (p) < 9 ? 3 : 5) /* note: checking if it's a valid pin is done by the digitalPinToPCICR macro. */
#define digitalPinToPCMSK(p)      (((p) > 0 && (p) < 9) ? (volatile uint8_t *)(&PCMSK0) : (((p) < 14 || (p) == 0) ? (volatile uint8_t *)(&PCMSK2) : ((p) < 17) ? (volatile uint8_t *)(&PCMSK1) : ((volatile uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p)   (((p) == 0) ? 0 : (((p) < 9) ? : (8 - (p)) : (((p) > 13) ? (17 - (p)) : (((p) > 11) ? (13 - (p)) : (14 - (p))))))

#define digitalPinToInterrupt(p)    ((p)==PIN_PC2 ? 0 : NOT_AN_INTERRUPT)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) < 6) ? 5-(p) : ((p) < 12) ? 22 - (p) : -1);
#define digitalPinToAnalogInput(p)  (((p) < 6) ? 5-(p) : ((p) < 17 && (p) > 10) ? 22 - (p) : -1)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == PIN_PA5 || (p) == PIN_PA6 || (p) == PIN_PB3 || (p) == PIN_PC0)

/* We have multiple pin mappings on this part; all have a #define, where
 * multiple are present, these are for compatibility with versions that
 * used less-clear names. The first #define is recommended, all others are
 * deprecated. */
#define PINMAPPING_CCW
#define PINMAPPING_LEGACY

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.      1 = initialise.
// #define DEFAULT_INITIALIZE_ADC               1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS  1

// We have hardware serial, so don't use soft serial.
#define USE_SOFTWARE_SERIAL                  0

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
#define PIN_TIMER_OC0A              (PIN_PC0)
#define PIN_TIMER_OC0B              (PIN_PA5)
#define PIN_TIMER_T0                (PIN_PA4)

/* Timer 1 - 16-bit timer with PWM */
#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PB3)
#define PIN_TIMER_OC1B              (PIN_PA6)
#define PIN_TIMER_T1                (PIN_PA3)
#define PIN_TIMER_ICP1              (PIN_PC1)

/*---------------------------------------------------------------------------
 * Chip Features (or lack thereof) - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)            (x << 6)

/* Analog reference bit masks. */
#define DEFAULT          ADC_REF(0x00) /* VCC used as analog reference, AREF pin may be used for other purposes. */
#define EXTERNAL         ADC_REF(0x01) /* External voltage applied to AREF pin. */
#define INTERNAL1V1      ADC_REF(0x02) /* Internal 1.1V voltage reference, AREF must not have external voltage applied. */
#define INTERNAL           INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_GROUND        ADC_CH(0x0C)
#define ADC_INTERNAL1V1   ADC_CH(0x0D)
#define ADC_TEMPERATURE   ADC_CH(0x0E)

/* Not a differential ADC */

/* Analog Comparator - not used by core */
#define ANALOG_COMP_DDR         (DDRA)
#define ANALOG_COMP_PORT        (PORTA)
#define ANALOG_COMP_PIN         (PINA)
#define ANALOG_COMP_AIN0_BIT    (1)
#define ANALOG_COMP_AIN1_BIT    (2)

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
#define USI_DI                PIN_PB1
#define USI_DO                PIN_PB2
#define USI_SCK               PIN_PC1
#define SS                    PIN_PC2

/* USI pins as ports and bits */
#define USI_DDR               DDRB
#define USI_PORT              PORTB
#define USI_PIN               PINB
#define USI_PUE               PUEB
#define USI_CLOCK_DDR         DDRC
#define USI_CLOCK_PORT        PORTC
#define USI_CLOCK_PIN         PINC
#define USI_CLOCK_PUE         PUEC
#define USI_CLOCK_BIT         PINC1
#define USI_DO_BIT            PINB2
#define USI_DI_BIT            PINB1

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

/* Two hardware serial ports */
#define PIN_HWSERIAL0_TX      PIN_PB0
#define PIN_HWSERIAL0_RX      PIN_PA7

#define PIN_HWSERIAL1_TX      PIN_PB1
#define PIN_HWSERIAL1_RX      PIN_PB2


#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY1634 ATTinyCore Legacy (Counterclockwise) Pin Mapping
 *
 *                 +-\/-+
 * TX   ( 0) PB0  1|a  a|20  PB1 (16)   TX
 * RX   ( 1) PA7  2|a  a|19  PB2 (15)   RX
 *    * ( 2) PA6  3|a  a|18  PB3 (14) *
 *    * ( 3) PA5  4|a  a|17  PC0 (13) *
 *      ( 4) PA4  5|a  a|16  PC1 (12)
 *      ( 5) PA3  6|a  a|15  PC2 (11)
 *      ( 6) PA2  7|    |14  PC3/RESET (17)
 *      ( 7) PA1  8|   x|13  PC4 (10)
 *      ( 8) PA0  9|   x|12  PC5 ( 9)
 *           GND 10|    |11  VCC
 *                 +----+
 *
 * * indicates PWM pin
 * a indicates ADC pin
 * x indicates XTAL pin
 *---------------------------------------------------------------------------*/

#pragma message("This is the Counterclockwise pin mapping - make sure you're using the pinout diagram or\n board numbered with the pins in counterclockwise order or by port \nand bit, or just use PIN_Pxn notation and never use a number and it'll work on both.\nThis mapping is stritly worse than clockwise, it is for compatibility, and is not recommended for new designs.")
const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRA,
  (uint8_t)(uint16_t) &DDRB,
  (uint8_t)(uint16_t) &DDRC
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTA,
  (uint8_t)(uint16_t) &PORTB,
  (uint8_t)(uint16_t) &PORTC
};

const uint8_t PROGMEM port_to_pullup_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PUEA,
  (uint8_t)(uint16_t) &PUEB,
  (uint8_t)(uint16_t) &PUEC
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINA,
  (uint8_t)(uint16_t) &PINB,
  (uint8_t)(uint16_t) &PINC
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
  PB, /* 0 */
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA, /* 8 */
  PC,
  PC,
  PC,
  PC,
  PC,
  PB, /* 14 */
  PB,
  PB,
  PC, /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0 */
  _BV(7),
  _BV(6),
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2),
  _BV(1),
  _BV(0), /* 8 */
  _BV(5),
  _BV(4),
  _BV(2),
  _BV(1),
  _BV(0),
  _BV(3), /* 14 */
  _BV(2),
  _BV(1),
  _BV(3), /* 17 = RESET */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER, /* 0 */
  NOT_ON_TIMER,
  TIMER1B,
  TIMER0B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 8 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER0A,
  TIMER1A,      /* 14 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* 17 = RESET */
};


#endif

#endif
