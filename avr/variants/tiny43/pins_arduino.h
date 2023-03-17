/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny43
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 * Straight-forward PortB to PortA, in order. PA7 thus comes last, and is the
 * reset pin so that works out. The power supply related pins are not directly
 * controilled by the CPU, so there's nothing to do about them (well, other
 * than connect an appropriate inductor and a diode with very low reverse
 * leakage to them per datasheet.
 *---------------------------------------------------------------------------*/
#define ATTINY43 1
#define __AVR_ATtiny43__

#define NUM_DIGITAL_PINS  (12)
#define NUM_ANALOG_INPUTS ( 4)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PA0           ( 8)
#define PIN_PA1           ( 9)
#define PIN_PA2           (10)
#define PIN_PA3           (11)
#define PIN_PA4           (12)
#define PIN_PA5           (13)
#define PIN_PA6           (14)
#define PIN_PA7           (15) /* RESET */
#define PIN_PB0           ( 0)
#define PIN_PB1           ( 1)
#define PIN_PB2           ( 2)
#define PIN_PB3           ( 3)
#define PIN_PB4           ( 4)
#define PIN_PB5           ( 5)
#define PIN_PB6           ( 6)
#define PIN_PB7           ( 7)

#ifndef LED_BUILTIN
  #define LED_BUILTIN     (PIN_PA5)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0            (PIN_PA0)
#define PIN_A1            (PIN_PA1)
#define PIN_A2            (PIN_PA2)
#define PIN_A3            (PIN_PA3)

/* An "analog pins" these map directly to analog channels */
static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 15) ? (volatile uint8_t *)(&GIMSK) : ((volatile uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) >= 0 && (p) <= 7) ? 5 : 4)
#define digitalPinToPCMSK(p)        (((p) >= 0 && (p) <= 7) ? (volatile uint8_t *)(&PCMSK1) : ((p) <= 15) ? (volatile uint8_t *)(&PCMSK0) : ((volatile uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)     ((p) & 0x07)

#define digitalPinToInterrupt(p)    ((p) == PIN_PB7 ? 0 : NOT_AN_INTERRUPT)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) < 4) ? (p) + 8 : -1)
#define digitalPinToAnalogInput(p)  (((p) > 7) && ((p) < 12) ? (p) - 8: -1)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == PIN_PB1 || (p) == PIN_PB2 || (p) == PIN_PB4 || (p) == PIN_PB5)


/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.   1 = initialise.
// #define DEFAULT_INITIALIZE_ADC                    1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS       1

/* Builtin Software Serial "Serial"
 * TX is on AIN0, RX is on AIN1. Comparator interrupt used so PCINTs remain
 * available for other uses. Comparator pins in Analog section below.        */
#define USE_SOFTWARE_SERIAL                  1

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
#define TIMER0_TYPICAL        (1)
#define PIN_TIMER_T0          (PIN_PB0)
#define PIN_TIMER_OC0A        (PIN_PB1)
#define PIN_TIMER_OC0B        (PIN_PB2)

/* Timer 1 - 8-bit timer with PWM - laaaaaame */
#define TIMER1_TYPICAL        (0)
#define PIN_TIMER_T1          (PIN_PB3)
#define PIN_TIMER_OC1A        (PIN_PB4)
#define PIN_TIMER_OC1B        (PIN_PB5)

/*---------------------------------------------------------------------------
 * Chip Features (or lack thereof) - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)            (x << 6)

/* Analog reference bit masks.*/
#define DEFAULT               ADC_REF(0)
#define INTERNAL1V1           ADC_REF(1)
#define INTERNAL              INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_GROUND            ADC_CH(0x04)
#define ADC_INTERNAL1V1       ADC_CH(0x05)
#define ADC_VBATDIV2          ADC_CH(0x06)
#define ADC_TEMPERATURE       ADC_CH(0x07)

/* Not a differential ADC     *
 * single ended channels only */

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR       (DDRA)
#define ANALOG_COMP_PORT      (PORTA)
#define ANALOG_COMP_PIN       (PINA)
#define ANALOG_COMP_AIN0_BIT  (4)
#define ANALOG_COMP_AIN1_BIT  (5)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a USI, not an SPI or TWI module. Accordingly, there is no MISO/MOSI in hardware.
 * There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI;
 * the defines here specify the pins for master mode, as SPI master is much more commonly used
 * in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware
 * of this when using the USI SPI functionality (and also, be aware that the MISO and MOSI
 * markings on the pinout diagram in the datasheet are for ISP programming, where the chip is
 * a slave. The pinout diagram included with this core attempt to clarify this.
 * The SS pin is chosen arbitrarily - we have no SPI slave library included with the core, but
 * libraries acting as master often expect there to be an SS pin defined, and will throw errors
 * if there isn't one. Since we provide an SPI.h that mimics the interface of the standard one
 * we also provide a dummy SS pin macro. MISO/MOSI/SCK, SDA, SCL #defines are in Arduino.h and
 * refer back to these macros (USI_* )
 *---------------------------------------------------------------------------*/

#define USE_SOFTWARE_SPI      1

/* USI */
#define USI_DI                PIN_PB4
#define USI_DO                PIN_PB5
#define USI_SCK               PIN_PB6
#define SS                    PIN_PB7

#define USI_DDR               DDRB
#define USI_PORT              PORTB
#define USI_PIN               PINB
#define USI_CLOCK_BIT         PINB6
#define USI_DO_BIT            PINB5
#define USI_DI_BIT            PINB4

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY43 ATTinyCore Standard Pin Mapping
 *
 *                    +-\/-+
 *        ( 0)  PB0  1|    |20  PA7  (15)  RESET
 *  PWM   ( 1)  PB1  2|    |19  PA6  (14)
 *  PWM   ( 2)  PB3  3|    |18  PA5  (13)
 *        ( 3)  PB2  4|    |17  PA4  (12)
 *  PWM   ( 4)  PB3  5|   a|16  PA3  (11)
 *  PWM   ( 5)  PB4  6|   a|15  PA2  (10)
 *        ( 6)  PB5  7|   a|14  PA1  ( 9)
 *  INT0  ( 7)  PB7  8|   a|13  PA0  ( 8)
 *              Vcc  9|    |12  VBat
 *              Gnd 10|    |11  LSW
 *                    +----+
 * a indicates ADC (analog input) pins
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
  PB, /* 0 */
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* 0, port A */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7)
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER,
  TIMER0A, /* OC0A */
  TIMER0B, /* OC0B */
  NOT_ON_TIMER,
  TIMER1A, /* OC1A */
  TIMER1B, /* OC1B */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER
};

#endif

#endif
