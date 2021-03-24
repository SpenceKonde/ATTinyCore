/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny85, ATtiny45, and ATtiny25
 *===========================================================================
 * Basic Pin Definitions | Interrupt Macros |
 *---------------------------------------------------------------------------*/

#define ATTINYX5 1       //backwards compatibility
#define __AVR_ATtinyX5__ //this is recommended way

#define NUM_DIGITAL_PINS         6
#define NUM_ANALOG_INPUTS        4

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PB0                 (0)
#define PIN_PB1                 (1)
#define PIN_PB2                 (2)
#define PIN_PB3                 (3)
#define PIN_PB4                 (4)
#define PIN_PB5                 (5)

#define LED_BUILTIN       (PIN_PB1)

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0            (PIN_PB5)
#define PIN_A1            (PIN_PB2)
#define PIN_A2            (PIN_PB4)
#define PIN_A3            (PIN_PB3)

/* An "analog pins" these map directly to analog channels */
static const uint8_t A0 = ADC_CH(0);
static const uint8_t A1 = ADC_CH(1);
static const uint8_t A2 = ADC_CH(2);
static const uint8_t A3 = ADC_CH(3);

#define digitalPinToInterrupt(p)    ((p) == 2 ? 0 : NOT_AN_INTERRUPT)

/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 5) ? (&GIMSK) : ((uint8_t *) NULL))
#define digitalPinToPCICRbit(p)     5
#define digitalPinToPCMSK(p)        (((p) >= 0 && (p) <= 5) ? (&PCMSK) : ((uint8_t *) NULL))
#define digitalPinToPCMSKbit(p)     (p)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  (((p) == 0) ? 5 : (((p) == 1) ? 2 : (((p) == 2) ? 4 : (((p) == 3) ? 3 : -1))))
#define digitalPinToAnalogInput(p)  (((p) == 5) ? 0 : (((p) == 2) ? 1 : (((p) == 4) ? 2 : (((p) == 3) ? 3 : -1))))

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == 0 || (p) == 1 || (p) == 4)


/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.      1 = initialise.
// #define DEFAULT_INITIALIZE_ADC               1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS  1

// Use OC1A in preference to OC0B for PWM on pin PB1
#define TIMER1_PWM

/* Builtin Software Serial "Serial"
 * TX is on AIN0, RX is on AIN1. Comparator interrupt used so PCINTs remain
 * available for other uses. Comparator pins in Analog section below.        */
#define USE_SOFTWARE_SERIAL                     1

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
#define INTERNAL2V56_NO_CAP ADC_REF(0x06) /* Not connected to AREF; AREF may be used for other purposes */
#define INTERNAL2V56_CAP    ADC_REF(0x07) /* Connect a capacitor between AREF and ground for improved reference stability */
#define INTERNAL2V56        INTERNAL2V56_NO_CAP
#define INTERNAL            INTERNAL

/* Special Analog Channels */
#define ADC_INTERNAL         ADC_CH(0x0C)
#define ADC_GROUND           ADC_CH(0x0D)
#define ADC_TEMPERATURE      ADC_CH(0x0F)

/* Differential Analog Channels */
#define DIFF_A2_A2_1X        ADC_CH(0x04)
#define DIFF_A2_A2_20X       ADC_CH(0x05)
#define DIFF_A2_A3_1X        ADC_CH(0x06)
#define DIFF_A2_A3_20X       ADC_CH(0x07)
#define DIFF_A0_A0_1X        ADC_CH(0x08)
#define DIFF_A0_A0_20X       ADC_CH(0x09)
#define DIFF_A0_A1_1X        ADC_CH(0x0A)
#define DIFF_A0_A1_20X       ADC_CH(0x0B)
#define DIFF_A3_A2_1X        ADC_CH(0x26)
#define DIFF_A3_A2_20X       ADC_CH(0x27)
#define DIFF_A1_A0_1X        ADC_CH(0x2A)
#define DIFF_A1_A0_20X       ADC_CH(0x2B)

/* Analog Comparator - used for soft-serial*/
#define ANALOG_COMP_DDR             DDRB
#define ANALOG_COMP_PORT           PORTB
#define ANALOG_COMP_PIN             PINB
#define ANALOG_COMP_AIN0_BIT           0
#define ANALOG_COMP_AIN1_BIT           1

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a USI, not an SPI or TWI module. Accordingly, there is no
 * MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as
 * master, DI is used as MISO, DO is MOSI; the defines here tell it where the
 * USI pins are. These are used in Arduino.h to generate MISO/MOSI/SCK for SPI
 * (this is for master mode, as there isn't support for SPI Slave in stock
 * SPI.h) and master mode is almost always what people want. A USI SPI slave
 * library should use the PIN_USI_role defines. The MISO/MOSI/SCK defines are
 * required for compatibility anyway.
 * Also, be aware that the MISO and MOSI markings on the pinout diagram in the
 * datasheet are for ISP programming, where the chip is the slave. The pinout
 * diagram included with this core attempts to clarify this,
 * The SS pin is chosen arbitrarily - libraries acting as master often expect
 * there to be an SS pin defined, and will throw errors if there isn't one.
 * Since we provide an SPI.h that mimics the interface of the standard one
 * we also provide a dummy SS pin macro. MISO/MOSI/SCK, SDA, SCL #defines
 * are in Arduino.h and refer back to these macros (PIN_USI_* )
 *---------------------------------------------------------------------------*/

#define USE_SOFTWARE_SPI      1

/* USI */
#define PIN_USI_SCK     PIN_PB2
#define PIN_USI_DO      PIN_PB1
#define PIN_USI_DI      PIN_PB0

#define SS              PIN_PB3

#define USI_DATA_DDR       DDRB
#define USI_DATA_PORT     PORTB
#define USI_DATA_PIN       PINB

#define USI_CLOCK_BIT     PINB2
#define USI_DO_BIT        PINB1
#define USI_DI_BIT        PINB0

#define USI_START_VECTOR  USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
  #define USI_START_COND_INT USISIF
#endif

/* Serial Ports - just the Software one */
#define PIN_SOFTSERIAL_TX                PIN_PB0
#define PIN_SOFTSERIAL_RX                PIN_PB1


#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY85/45/25 ATTinyCore Standard Pin Mapping
 *
 *                       +-\/-+
 *        A0  (5)  PB5  1|    |8   VCC
 *        A3  (3)  PB3  2|    |7   PB2  (2)  A1  INT0
 *  PWM   A2  (4)  PB4  3|    |6   PB1  (1)      PWM (OC0B or OC1A)
 * (OC1B)          GND  4|    |5   PB0  (0)      PWM (OC0A)
 *                       +----+
 * Currently we default to using the superior timer 1 for PW pin 1, which could also use the boring timer 0.
 *---------------------------------------------------------------------------*/

const uint8_t PROGMEM port_to_mode_PGM[] =
{
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  NOT_A_PORT,
  NOT_A_PORT,
  (uint8_t)(uint16_t)&PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  NOT_A_PIN,
  NOT_A_PIN,
  (uint8_t)(uint16_t)&PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PB, /* 0 */
  PB,
  PB,
  PB,
  PB,
  PB, /* 5 */

};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* 0, port B */
  _BV(1),
  _BV(2),
  _BV(3), /* 3 port B */
  _BV(4),
  _BV(5),

};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  TIMER0A, /* OC0A */
  #ifdef TIMER1_PWM
  TIMER1A, /* OC1A */
  #else
  TIMER0B, /* OC0B */
  #endif
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1B, /*OC1B*/
  NOT_ON_TIMER,
};

#endif

#endif
