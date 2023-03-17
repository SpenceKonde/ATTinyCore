/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny26
 *===========================================================================
 * This is a hopelessly obsolete chip! Do not choose this part voluntarily.
 * It is for use by those who have large numbers of these ancient devices to work with
 *---------------------------------------------------------------------------*/

#define NUM_DIGITAL_PINS            (16)
#define NUM_ANALOG_INPUTS           (11)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PA0          ( 0)
#define PIN_PA1          ( 1)
#define PIN_PA2          ( 2)
#define PIN_PA3          ( 3)
#define PIN_PA4          ( 4)
#define PIN_PA5          ( 5)
#define PIN_PA6          ( 6)
#define PIN_PA7          ( 7)
#define PIN_PB0          ( 8)
#define PIN_PB1          ( 9)
#define PIN_PB2          (10)
#define PIN_PB3          (11)
#define PIN_PB4          (12)
#define PIN_PB5          (13)
#define PIN_PB6          (14)
#define PIN_PB7          (15) /* RESET */

#ifndef LED_BUILTIN
  #define LED_BUILTIN (PIN_PB6)
#endif

/* PIN_An is the digital pin with analog channel An on it. */
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

#define digitalPinToPCICR(p)        ((((p) >= 6 && (p) <= 15) || (p)==3) ? (volatile uint8_t *)(&GIMSK) : ((volatile uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     ( ((p) >= 8 && (p) <  12)  ?  5 : 4)
#define digitalPinToPCMSK(p)        (badCall("there IS NO PCMSK HERE. You can enable PCINTs on PB0:3 and on all the other PCINT pins."))
#define digitalPinToPCMSKbit(p)     (badCall("there IS NO PCMSK HERE. You can enable PCINTs on PB0:3 and on all the other PCINT pins."))

#define digitalPinToInterrupt(p)    ((p) == 14 ? 0 : NOT_AN_INTERRUPT)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == PIN_PB1 || (p) == PIN_PB3)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  ((p < 3) ? (p): (((p) < 7) ? ((p) + 1) : ((p) < 11) ? ((p) + 5) : NOT_A_PIN))
#define digitalPinToAnalogInput(p)  ((p < 3) ? (p): (((p) > 3) && (p) < 8) ? ((p) - 1) : (((p) < 16 && (p) > 11 ) ? ((p) - 5) : NOT_A_PIN))


/*--------00-------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/

// Choosing not to initialise saves flash.           1 = initialise.
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

/* Timer 0 - 8-bit timer without PWM */
#define TIMER0_TYPICAL              (0)

/* Timer 1: 8 bit high speed timer with PWM */
#define TIMER1_TYPICAL              (0)
#define PIN_TIMER_OC1A              (PIN_PB1)
#define PIN_TIMER_OC1B              (PIN_PB3)


/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash. Definition of ADC_REF is messy because we need to
 * reorder the bits so they line up. Aren't you glad that's not happening at
 * runtime?
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)          (((x) & 0x03) << 6)

/* This is such an old part they didn't have an ADCSRB so ADCSRA is called ADCSR */
#define ADCSRA ADCSR

/* Analog Reference bit masks */
#define DEFAULT             ADC_REF(0x00)
#define EXTERNAL            ADC_REF(0x01)
#define INTERNAL2V56        ADC_REF(0x02) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL2V56_CAP    ADC_REF(0x03) /* Connect a capacitor between AREF and ground for improved reference stability */
#define INTERNAL2V56_NO_CAP INTERNAL2V56
#define INTERNAL2V56NOBP    INTERNAL2V56  /* deprecated */
#define INTERNAL            INTERNAL2V56   /* deprecated */

/* Special Analog Channels */
#define ADC_INTERNAL1V1      ADC_CH(0x1E)
#define ADC_GROUND           ADC_CH(0x1F)
#define ADC_TEMPERATURE      ADC_CH(0x3F)

/* Differential Analog Channels */
#define DIFF_A0_A1_20X       ADC_CH(0x0B)
#define DIFF_A0_A1_1X        ADC_CH(0x0C)
#define DIFF_A1_A1_20X       ADC_CH(0x0D)
#define DIFF_A2_A1_20X       ADC_CH(0x0E)
#define DIFF_A2_A1_1X        ADC_CH(0x0F)

#define DIFF_A2_A3_1X        ADC_CH(0x10)
#define DIFF_A3_A3_20X       ADC_CH(0x11)
#define DIFF_A4_A3_20X       ADC_CH(0x12)
#define DIFF_A4_A3_1X        ADC_CH(0x13)

#define DIFF_A4_A5_20X       ADC_CH(0x14)
#define DIFF_A4_A5_1X        ADC_CH(0x15)
#define DIFF_A5_A5_20X       ADC_CH(0x16)
#define DIFF_A6_A5_20X       ADC_CH(0x17)
#define DIFF_A6_A5_1X        ADC_CH(0x18)

#define DIFF_A8_A9_20X       ADC_CH(0x19)
#define DIFF_A8_A9_1X        ADC_CH(0x1A)
#define DIFF_A9_A9_20X       ADC_CH(0x1B)
#define DIFF_A10_A9_20X      ADC_CH(0x1C)
#define DIFF_A10_A9_1X       ADC_CH(0x1D)

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR            (DDRA)
#define ANALOG_COMP_PORT           (PORTA)
#define ANALOG_COMP_PIN            (PINA)
#define ANALOG_COMP_AIN0_BIT       (6)
#define ANALOG_COMP_AIN1_BIT       (7)

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

#define USE_SOFTWARE_SPI 1

/* USI */
#define USI_DI                PIN_PB0
#define USI_DO                PIN_PB1
#define USI_SCK               PIN_PB2
#define SS                    PIN_PB3

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

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY26 ATTinyCore Standard Pin Mapping
 *
 *                 +-\/-+
 *      ( 8) PB0  1|   a|20  PA0 ( 0)
 *     *( 9) PB1  2|   a|19  PA1 ( 1)
 *      (10) PB2  3|   a|18  PA2 ( 2)
 *     *(11) PB3  4|    |17  PA3 ( 3)
 *           VCC  5|    |16  AGND
 *           GND  6|    |15  AVCC
 *      (12) PB4  7|ax a|14  PA4 ( 4)
 *      (13) PB5  8|ax a|13  PA5 ( 5)
 * INT0 (14) PB6  9|a  a|12  PA6 ( 6)
 *      (15) PB7 10|a  a|11  PA7 ( 7)
 *                 +----+
 *
 * a indicates ADC pin
 * x indicates XTAL pin
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
  PB,
  PB,
  PB,
  PB,
  PB, /* 15 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0 */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* 8 */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7), /* 15 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1A,
  NOT_ON_TIMER,
  TIMER1B,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};

#endif

#endif
