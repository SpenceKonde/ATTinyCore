/* pins_arduino.h - Pin definition functions for ATTinyCore
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny167, ATtiny87 "Digispark Pro"
 *===========================================================================
 * Basic Pin Definitions | Interrupt Macros | Legacy/compatibility
 *---------------------------------------------------------------------------
 * This is an horrifying legacy pin mapping, it is so bad that there are
 * significant effects on code size and performance. Do not use this unless
 * you have a gun to your head. Rven then you might come to regret it.
 *---------------------------------------------------------------------------*/

#define ATTINYX7 1       // backwards compatibility
#define __AVR_ATtinyX7__ // recommended

#define NUM_DIGITAL_PINS            (16)
#define NUM_ANALOG_INPUTS           (11)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
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

#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PB0)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0        (PIN_PA0)
#define PIN_A1        (PIN_PA1)
#define PIN_A2        (PIN_PA2)
#define PIN_A3        (PIN_PA3)
#define PIN_A4        (PIN_PA4)
#define PIN_A5        (PIN_PA5)
#define PIN_A6        (PIN_PA6)
#define PIN_A7        (PIN_PA7)
#define PIN_A8        (PIN_PB5)
#define PIN_A9        (PIN_PB6)
#define PIN_A10       (PIN_PB7)

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


/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)       (((p) >= 0 && (p) <= 15) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (digitalPinToPort(p) == PA ? 4 : 5)
#define digitalPinToPCMSK(p)      ((((p) >= 0 && (p) <= 15) ? (digitalPinToPort(p) == PA ? &PCMSK0 : &PCMSK1) : (uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)    (((p) >= 0 && (p) <= 15) ? (const_array_or_pgm_(pgm_read_byte, digital_pin_to_bit_pos_PGM, (p))) : -1)

#define digitalPinToInterrupt(p)    ((p) == 2 ? 0 : ((p)==11?1: NOT_AN_INTERRUPT))

extern const uint8_t PROGMEM digital_pin_to_analog_input_PGM[];
extern const uint8_t PROGMEM analog_input_to_digital_pin_PGM[];

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p) (((p) >= NUM_DIGITAL_PINS)  ? NOT_A_PIN : (const_array_or_pgm_(pgm_read_byte, analog_input_to_digital_pin_PGM, (p))))
#define digitalPinToAnalogInput(p) (((p) >= NUM_ANALOG_INPUTS) ? NOT_A_PIN : (const_array_or_pgm_(pgm_read_byte, digital_pin_to_analog_input_PGM, (p))))

/* Which pins have PWM? */
#define digitalPinHasPWM(p)        ((p) == 2 || ((p) > 3 && (p) < 10) || (p) == 15 )

/* We have multiple pin mappings on this part; all have a #define, where
 * multiple are present, these are for compatibility with versions that
 * used less-clear names. The first #define is recommended, all others are
 * deprecated. */

#define PINMAPPING_LEGACY
#define PINMAPPING_OLD

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.      1 = initialise.
// #define DEFAULT_INITIALIZE_ADC               1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS  1

// We have hardware serial, so don't use soft serial.
// #define USE_SOFTWARE_SERIAL                  0

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

#define PWM_CHANNEL_REMAPPING       (1) /*analogWrite() pin to enable PWM
After doing that, the normal ways of manipulating registers for PWM will
work for it unless or until digitalWrite() us called on it.
Methods that use the standard ways to turn off PWM will prevent it from working
afterwards if you then try to go back to core-provided PWM functions. The
"standard" way uses the COMnx0 and COMnx1 bits in the TCCRny registers. We set
those at power on only (so it has no space cost because we have to set them
anyway) and instead just use TOCPMCOE bits to control whether PWM is output */

/* Timer 0 - 8-bit timer async support and only 1 PWM channel */
#define TIMER0_TYPICAL              (0)
#define PIN_TIMER_OC0A              (PIN_PA2)

/* Timer 1 - 16-bit timer with PWM with automatic remapping with analogWrite to any pin on PORTB*/
#define TIMER1_TYPICAL              (0)
#define PIN_TIMER_T1                (PIN_PA5)
#define PIN_TIMER_ICP1              (PIN_PA4)

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash. ADC_REF definition here is longer because we need to
 * take the two high bits (AREF, XREF), and put them in position for where
 * they go in the AMISCR... better here than at runtime!
 *---------------------------------------------------------------------------*/
/* This is weird on the 87/167 - the internal references are selected by the
 * REFS bits, but two additional bits on the AMISCR (Analog MIScellaneous
 * Control Register) are used. AREF controls whether an external reference is
 * used (ignored if internal ref selected) and XREF which outputs the internal
 * reference on the AREF pin. They suggest a 5nF-10nF capacitor on that pin,
 * and note that load on it should be between 1uA and 100uA. No idea why they
 * used two bits, one of which is ignored when internal ref is in use and the
 * other which is ignored when internal ref not in use...
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)       ((((x) & 0x03) << 6) | (((x) & 0x0C) >> 1))

/* Analog Reference bit masks */
#define DEFAULT           ADC_REF(0x00)
#define EXTERNAL          ADC_REF(0x08) /* Apply external reference voltage to AREF pin */
#define INTERNAL1V1       ADC_REF(0x02) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL            INTERNAL1V1 /* deprecated *
#define INTERNAL2V56      ADC_REF(0x03) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL1V1_XREF  ADC_REF(0x06) /* Interna1 1.1V reference is output on AREF, and may be used by other devices */
#define INTERNAL2V56_XREF ADC_REF(0x07) /* Interna1 2.56V reference is output on AREF, and may be used by other devices */

/* Special Analog Channels */
#define ADC_TEMPERATURE    ADC_CH(0x0B)
#define ADC_INTERNAL1V1    ADC_CH(0x0C)
#define ADC_AVCCDIV4       ADC_CH(0x0D)
#define ADC_GROUND         ADC_CH(0x0E)

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

/* Analog Comparator - not used by core */
#define ANALOG_COMP_DDR         (DDRA)
#define ANALOG_COMP_PORT        (PORTA)
#define ANALOG_COMP_PIN         (PINA)
#define ANALOG_COMP_AIN0_BIT    (6)
#define ANALOG_COMP_AIN1_BIT    (7)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/*  This part has a USI, not a TWI module - but it DOES have an SPI module!
 *  We provide USI defines so that Arduino.h will sort out SCL, SDA pin assignment.
 *  The included version of Wire.h will use the USI for TWI if requested.
 *---------------------------------------------------------------------------*/

/* Hardware SPI */
#define MISO                  PIN_PA2
#define MOSI                  PIN_PA4
#define SCK                   PIN_PA5
#define SS                    PIN_PA6

/* USI */
#define USI_DI                PIN_PB0
#define USI_DO                PIN_PB1
#define USI_SCK               PIN_PB2

/* USI pins are ports and bits */
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

/* One hardware LIN port, which is a UART with a ton of wacky features */
#define PIN_HWSERIAL0_TX      PIN_PA1
#define PIN_HWSERIAL0_RX      PIN_PA0
#define HWSERIAL0_IS_LIN      1

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY167/ATTINY87
 * Legacy Arduino-compatible pin mapping - an inspired work of evil
 * This must have been a bet or something, what's the most consecutive
 * pin numbers we can have while making the code maximally unpleasant?
 *
 *                   +-\/-+
 *     RX ( 0) PA0  1|a   |20  PB0 ( 4)*
 *     TX ( 1) PA1  2|a   |19  PB1 ( 5)*
 *       *(12) PA2  3|a   |18  PB2 ( 6)*
 *        ( 3) PA3  4|a   |17  PB3 ( 7)*
 *            AVCC  5|    |16  GND
 *            AGND  6|    |15  VCC
 *   INT1 (11) PA4  7|a   |14  PB4 ( 8)*
 *        (13) PA5  8|a  a|13  PB5 ( 9)*
 *        (10) PA6  9|a  a|12  PB6 ( 2)* INT0
 *        (14) PA7 10|a  a|11  PB7 (15)* Reset
 *                   +----+
 *
 * * indicates PWM pin, a indicates ADC (analog input) pins
 *---------------------------------------------------------------------------*/

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
  (1), /* 0 */
  (1),
  (2), /* 2 */
  (1), /* 3 */
  (2),  /* 4 */
  (2),
  (2),
  (2),
  (2),
  (2),
  (1), /* 10 */
  (1),
  (1),
  (1),
  (1),
  (2) /* 15 */
};

// Wow, a core finally did such a perverse job of distributing pins that I needed to make a progmem array to sort out analog and digital pins.
const uint8_t PROGMEM digital_pin_to_analog_input_PGM[] = {
  (0), /* 0 */
  (1),
  (9), /* 2 */
  (3), /* 3 */
  NOT_A_PIN, /* 4 */
  NOT_A_PIN,
  NOT_A_PIN,
  NOT_A_PIN,
  NOT_A_PIN,
  (8),
  (6), /* 10 */
  (4),
  (2),
  (5),
  (7),
  (10) /* 15 */
};

const uint8_t PROGMEM analog_input_to_digital_pin_PGM [] = {
  (0), /* 0 */
  (1),
  (12), /* 2 */
  (3), /* 3 */
  (11), /* 4 */
  (13),
  (10),
  (14),
  (9),
  (2),
  (15)  /* 10 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
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

const uint8_t PROGMEM digital_pin_to_bit_pos_PGM[] = {
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

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
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
