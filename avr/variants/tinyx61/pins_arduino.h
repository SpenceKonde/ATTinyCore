/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny861A, ATtiny461A, and ATtiny261A and non-A versions
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 * This is the newer, rationalized pin layout. The old one was so
 * convoluted that it impacted performance and code size due to
 * the complexity of the macros.
 *---------------------------------------------------------------------------*/

#define ATTINYX61 1  // backwards compatibility
#define __AVR_ATtinyX61__ // recommended

#define NUM_DIGITAL_PINS  (16)
#define NUM_ANALOG_INPUTS (11)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PA0           ( 0)
#define PIN_PA1           ( 1)
#define PIN_PA2           ( 2)
#define PIN_PA3           ( 3)
#define PIN_PA4           ( 4)
#define PIN_PA5           ( 5)
#define PIN_PA6           ( 6)
#define PIN_PA7           ( 7)
#define PIN_PB0           ( 8)
#define PIN_PB1           ( 9)
#define PIN_PB2           (10)
#define PIN_PB3           (11)
#define PIN_PB4           (12)
#define PIN_PB5           (13)
#define PIN_PB6           (14)
#define PIN_PB7           (15) /* RESET */

#ifndef LED_BUILTIN
  #if USING_BOOTLOADER == 0x08
    #define LED_BUILTIN   (PIN_PB5)
  #else
    #define LED_BUILTIN   (PIN_PB6)
  #endif
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0            (PIN_PA0)
#define PIN_A1            (PIN_PA1)
#define PIN_A2            (PIN_PA2)
#define PIN_A3            (PIN_PA4) /* PA3 is the AREF pin */
#define PIN_A4            (PIN_PA5)
#define PIN_A5            (PIN_PA6)
#define PIN_A6            (PIN_PA7)
#define PIN_A7            (PIN_PB4)
#define PIN_A8            (PIN_PB5)
#define PIN_A9            (PIN_PB6)
#define PIN_A10           (PIN_PB7)

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

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 16) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) >= 8 && (p) <= 11) ? (PCIE0) : (PCIE1))
#define digitalPinToPCMSK(p)        (((p) >= 0 && (p) <= 16) ? ((p < 8) ? (&PCMSK0) : (&PCMSK1)) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p)     ((p) & 0x07)


#define digitalPinToInterrupt(p)    ((p) == 14 ? 0 : ((p)==2 ? 1: NOT_AN_INTERRUPT))

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == PIN_PB1 || (p) == PIN_PB3 || (p) == PIN_PB5)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  ((p < 3) ? (p): (((p) < 7) ? ((p) + 1) : ((p) < 11) ? ((p) + 5) : NOT_A_PIN))
#define digitalPinToAnalogInput(p)  ((p < 3) ? (p): (((p) > 3) && (p) < 8) ? ((p) - 1) : (((p) < 16 && (p) > 11 ) ? ((p) - 5) : NOT_A_PIN))


/* We have multiple pin mappings on this part; all have a #define, where
 * multiple are present, these are for compatibility with versions that
 * used less-clear names. The first #define is recommended, all others are
 * deprecated. */
#define PINMAPPING_STANDARD
#define PINMAPPING_NEW

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/

// Choosing not to initialise saves flash.           1 = initialise.
// #define DEFAULT_INITIALIZE_ADC                    1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS       1

/* Builtin Software Serial "Serial"
 * TX is on AIN0, RX is on AIN1. Comparator interrupt used so PCINTs remain
 * available for other uses. Comparator pins in Analog section below.        */
#ifndef USE_SOFTWARE_SERIAL
  #define USE_SOFTWARE_SERIAL                  1
#endif

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

/* Timer 0 - 16-bit utility timer without PWM!*/
#define TIMER0_TYPICAL              (0)
#define PIN_TIMER_T0                (PIN_PB7)
#define PIN_TIMER_ICP0              (PIN_PA4)

/* Timer 1 - 10-bit high speed timer with three pwm channels */
#define TIMER1_TYPICAL              (0)
#define PIN_TIMER_OC1A              (PIN_PB1)
#define PIN_TIMER_OC1B              (PIN_PB3)
#define PIN_TIMER_OC1D              (PIN_PB5)

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash. Definition of ADC_REF is messy because we need to
 * reorder the bits so they line up. Aren't you glad that's not happening at
 * runtime?
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)          ((((x) & 0x03) << 6) | (((x) & 0x04) << 2))

/* Analog Reference bit masks */
#define DEFAULT             ADC_REF(0x00)
#define EXTERNAL            ADC_REF(0x01)
#define INTERNAL1V1         ADC_REF(0x02) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL2V56        ADC_REF(0x06) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL2V56_CAP    ADC_REF(0x07) /* Connect a capacitor between AREF and ground for improved reference stability */
#define INTERNAL2V56_NO_CAP INTERNAL2V56
#define INTERNAL2V56NOBP    INTERNAL2V56  /* deprecated */
#define INTERNAL            INTERNAL1V1   /* deprecated */

/* Special Analog Channels */
#define ADC_INTERNAL1V1      ADC_CH(0x1E)
#define ADC_GROUND           ADC_CH(0x1F)
#define ADC_TEMPERATURE      ADC_CH(0x3F)

/* Differential Analog Channels */

// A0-A2 - duplicates of some options in first triad
#define DIFF_A0_A1_20XA      ADC_CH(0x0B)
#define DIFF_A0_A1_1XA       ADC_CH(0x0C)
#define DIFF_A1_A1_20XA      ADC_CH(0x0D)
#define DIFF_A2_A1_20XA      ADC_CH(0x0E)
#define DIFF_A2_A1_1XA       ADC_CH(0x0F)
// A2~A4
#define DIFF_A2_A3_1X        ADC_CH(0x10)
#define DIFF_A3_A3_20X       ADC_CH(0x11)
#define DIFF_A4_A3_20X       ADC_CH(0x12)
#define DIFF_A4_A3_1X        ADC_CH(0x13)
// A4-A6 - duplicates of some options in second triad
#define DIFF_A4_A5_20XA      ADC_CH(0x14)
#define DIFF_A4_A5_1XA       ADC_CH(0x15)
#define DIFF_A5_A5_20XA      ADC_CH(0x16)
#define DIFF_A6_A5_20XA      ADC_CH(0x17)
#define DIFF_A6_A5_1XA       ADC_CH(0x18)
// A8~A10 - high oddballs
#define DIFF_A8_A9_20X       ADC_CH(0x19)
#define DIFF_A8_A9_1X        ADC_CH(0x1A)
#define DIFF_A9_A9_20X       ADC_CH(0x1B)
#define DIFF_A10_A9_20X      ADC_CH(0x1C)
#define DIFF_A10_A9_1X       ADC_CH(0x1D)
/* These support gain selection GSEL
 * is passed as high bit of the channel
 * so that analogRead still works  */
// A0-2 are a full function triad.
// A0 & A1
#define DIFF_A0_A1_20X       ADC_CH(0x20)
#define DIFF_A0_A1_32X       ADC_CH(0x60)
#define DIFF_A0_A1_1X        ADC_CH(0x21)
#define DIFF_A0_A1_8X        ADC_CH(0x61)
#define DIFF_A1_A0_20X       ADC_CH(0x22)
#define DIFF_A1_A0_32X       ADC_CH(0x62)
#define DIFF_A1_A0_1X        ADC_CH(0x23)
#define DIFF_A1_A0_8X        ADC_CH(0x63)
// A1 & A2
#define DIFF_A1_A2_20X       ADC_CH(0x24)
#define DIFF_A1_A2_32X       ADC_CH(0x64)
#define DIFF_A1_A2_1X        ADC_CH(0x25)
#define DIFF_A1_A2_8X        ADC_CH(0x65)
#define DIFF_A2_A1_20X       ADC_CH(0x26)
#define DIFF_A2_A1_32X       ADC_CH(0x66)
#define DIFF_A2_A1_1X        ADC_CH(0x27)
#define DIFF_A2_A1_8X        ADC_CH(0x67)
// A0 & A2
#define DIFF_A2_A0_20X       ADC_CH(0x28)
#define DIFF_A2_A0_32X       ADC_CH(0x68)
#define DIFF_A2_A0_1X        ADC_CH(0x29)
#define DIFF_A2_A0_8X        ADC_CH(0x69)
#define DIFF_A0_A2_20X       ADC_CH(0x2A)
#define DIFF_A0_A2_32X       ADC_CH(0x6A)
#define DIFF_A0_A2_1X        ADC_CH(0x2B)
#define DIFF_A0_A2_8X        ADC_CH(0x6B)
// A4-6 are a full function triad.
// A4 & A5
#define DIFF_A4_A5_20X       ADC_CH(0x2C)
#define DIFF_A4_A5_32X       ADC_CH(0x6C)
#define DIFF_A4_A5_1X        ADC_CH(0x2D)
#define DIFF_A4_A5_8X        ADC_CH(0x6D)
#define DIFF_A5_A4_20X       ADC_CH(0x2E)
#define DIFF_A5_A4_32X       ADC_CH(0x6E)
#define DIFF_A5_A4_1X        ADC_CH(0x2F)
#define DIFF_A5_A4_8X        ADC_CH(0x6F)
// A5 & A6
#define DIFF_A5_A6_20X       ADC_CH(0x30)
#define DIFF_A5_A6_32X       ADC_CH(0x70)
#define DIFF_A5_A6_1X        ADC_CH(0x31)
#define DIFF_A5_A6_8X        ADC_CH(0x71)
#define DIFF_A6_A5_20X       ADC_CH(0x32)
#define DIFF_A6_A5_32X       ADC_CH(0x72)
#define DIFF_A6_A5_1X        ADC_CH(0x33)
#define DIFF_A6_A5_8X        ADC_CH(0x73)
// A4 & A6
#define DIFF_A6_A4_20X       ADC_CH(0x34)
#define DIFF_A6_A4_32X       ADC_CH(0x74)
#define DIFF_A6_A4_1X        ADC_CH(0x35)
#define DIFF_A6_A4_8X        ADC_CH(0x75)
#define DIFF_A4_A6_20X       ADC_CH(0x36)
#define DIFF_A4_A6_32X       ADC_CH(0x76)
#define DIFF_A4_A6_1X        ADC_CH(0x37)
#define DIFF_A4_A6_8X        ADC_CH(0x77)
/* Same channels on both sides
 * for offset correction */
// Offset cal for A0, all gain options.
#define DIFF_A0_A0_20X       ADC_CH(0x38)
#define DIFF_A0_A0_32X       ADC_CH(0x78)
#define DIFF_A0_A0_1X        ADC_CH(0x39)
#define DIFF_A0_A0_8X        ADC_CH(0x79)
// Offset cal for A1, A2, high gain options only
#define DIFF_A1_A1_20X       ADC_CH(0x3A)
#define DIFF_A1_A1_32X       ADC_CH(0x7A)
#define DIFF_A2_A2_20X       ADC_CH(0x3B)
#define DIFF_A2_A2_32X       ADC_CH(0x7B)
// Offset cal for second triad, high gain options only
#define DIFF_A4_A4_20X       ADC_CH(0x3C)
#define DIFF_A4_A4_32X       ADC_CH(0x7C)
#define DIFF_A5_A5_20X       ADC_CH(0x3D)
#define DIFF_A5_A5_32X       ADC_CH(0x7D)
#define DIFF_A6_A6_20X       ADC_CH(0x3E)
#define DIFF_A6_A6_32X       ADC_CH(0x7E)

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR           (DDRA)
#define ANALOG_COMP_PORT          (PORTA)
#define ANALOG_COMP_PIN           (PINA)
#define ANALOG_COMP_AIN2_BIT      (5)
#define ANALOG_COMP_AIN0_BIT      (6)
#define ANALOG_COMP_AIN1_BIT      (7)

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
#ifndef SET_REMAPUSI
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
#else
  #define USI_DI                PIN_PA0
  #define USI_DO                PIN_PA1
  #define USI_SCK               PIN_PA2
  #define SS                    PIN_PA3

  #define USI_DDR               DDRA
  #define USI_PORT              PORTA
  #define USI_PIN               PINA

  #define USI_CLOCK_BIT         PINA2
  #define USI_DO_BIT            PINA1
  #define USI_DI_BIT            PINA0
#endif

#define USI_START_VECTOR      USI_START_vect
#define USI_OVERFLOW_VECTOR   USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT  USISIF
#endif

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY861 ATTinyCore Standard Pin Mapping
 *
 *                 +-\/-+
 *      ( 8) PB0  1|   a|20  PA0 ( 0)
 *     *( 9) PB1  2|   a|19  PA1 ( 1)
 *      (10) PB2  3|   a|18  PA2 ( 2) INT1
 *     *(11) PB3  4|    |17  PA3 ( 3)
 *           VCC  5|    |16  AGND
 *           GND  6|    |15  AVCC
 *      (12) PB4  7|ax a|14  PA4 ( 4)
 *     *(13) PB5  8|ax a|13  PA5 ( 5)
 * INT0 (14) PB6  9|a  a|12  PA6 ( 6) AIN0/TX
 *      (15) PB7 10|a  a|11  PA7 ( 7) AIN1/RX
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
  TIMER1D,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
};

#endif

#endif
