/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - LGPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

/*===========================================================================
 * Microchip ATtiny841, ATtiny441
 *===========================================================================
 * ATTinyCore Legacy Pin Mapping (CounterClockwise)
 * CCW/Legacy pin mapping is inferior. Use the other one if you don't
 * have code or hardware marked with the CCW numbers. Not only does this require
 * more shuffling around of bits (so some macros are slower), if a crystal is
 * used that takes out pins 0 and 1 instead of pins at the end.
 *---------------------------------------------------------------------------*/

#define ATTINYX41 1
#define __AVR_ATtinyX41__

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS    (12)
#define NUM_ANALOG_INPUTS   (12)

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PB0             ( 0) /* XTAL2 */
#define PIN_PB1             ( 1) /* XTAL1 */
#define PIN_PB2             ( 2)
#define PIN_PA7             ( 3)
#define PIN_PA6             ( 4)
#define PIN_PA5             ( 5)
#define PIN_PA4             ( 6)
#define PIN_PA3             ( 7)
#define PIN_PA2             ( 8)
#define PIN_PA1             ( 9)
#define PIN_PA0             (10)
#define PIN_PB3             (11)  /* RESET */

#ifndef LED_BUILTIN
  #ifndef USB_TWOPLUS
    #define LED_BUILTIN (PIN_PB2)
  #else
    #define LED_BUILTIN (PIN_PB0)
  #endif
#endif

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0      (PIN_PA0)
#define PIN_A1      (PIN_PA1)
#define PIN_A2      (PIN_PA2)
#define PIN_A3      (PIN_PA3)
#define PIN_A4      (PIN_PA4)
#define PIN_A5      (PIN_PA5)
#define PIN_A6      (PIN_PA6)
#define PIN_A7      (PIN_PA7)
#define PIN_A8      (PIN_PB2)
#define PIN_A9      (PIN_PB3) /* RESET */
#define PIN_A10     (PIN_PB0)
#define PIN_A11     (PIN_PB1)

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
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)        (((p) >= 0 && (p) <= 10) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p)     (((p) <= 2) ? PCIE1 : PCIE0)
#define digitalPinToPCMSK(p)        (((p) <= 2) ? (&PCMSK1) : (((p) <= 10) ? (&PCMSK0) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p)     (((p) <= 2) ? (p) : (10 - (p)))

#define digitalPinToInterrupt(p)    ((p) == PIN_PB1 ? 0 : NOT_AN_INTERRUPT)

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  ((p) < 9 ? (10 - (p)) : ((p) == 9 ? 11 : 11 - (p)))
#define digitalPinToAnalogInput(p)  ((p) < 11 ? (((p) > 1) ? 10 - (p) : 11 - (p)) : 9)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) > 1 && (p) < 8)

/* We have multiple pin mappings on this part; all have a #define, where
 * multiple are present, these are for compatibility with versions that
 * used less-clear names. The first #define is recommended, all others are
 * deprecated. */
#define PINMAPPING_CCW

/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/
// Choosing not to initialise saves flash.           1 = initialise.
// #define DEFAULT_INITIALIZE_ADC                    1
// #define DEFAULT_INITIALIZE_SECONDARY_TIMERS       1

// We have hardware serial, so don't use soft serial.
#define USE_SOFTWARE_SERIAL                          0

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
work for it unless or until digitalWrite() is called on it.
Methods that use the standard ways to turn off PWM will prevent it from working
afterwards if you then try to go back to core-provided PWM functions. The
"standard" way uses the COMnx0 and COMnx1 bits in the TCCRny registers. We set
those at power on only (so it has no space cost because we have to set them
anyway) and instead just use TOCPMCOE bits to control whether PWM is output */

/* Timer 0 - 8-bit timer with PWM */
#define TIMER0_TYPICAL              (1)
#define PIN_TIMER_OC0A              (PIN_PA4) /* core default - TOCC3 */
#define PIN_TIMER_OC0B              (PIN_PA5) /* core default - TOCC4 */
#define PIN_TIMER_T0                (PIN_PA3)

/* Timer 1 - 16-bit timer with PWM */
#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PA6) /* core default - TOCC2*/
#define PIN_TIMER_OC1B              (PIN_PA3) /* core default - */
#define PIN_TIMER_T1                (PIN_PA4)
#define PIN_TIMER_ICP1              (PIN_PA7)

/* Timer 2 - 16-bit timer with PWM */
#define TIMER2_TYPICAL              (0) /* No, it's like timer1 */
#define PIN_TIMER_OC2A              (PIN_PA7) /* core default */
#define PIN_TIMER_OC2B              (PIN_PB2) /* core default */
#define PIN_TIMER_T2                (PIN_PA5)
#define PIN_TIMER_ICP2              (PIN_PB2)

/*---------------------------------------------------------------------------
 * Chip Features - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)              (x << 5)

/* Analog reference bit masks. */
#define DEFAULT               ADC_REF(0x00)
#define INTERNAL1V1           ADC_REF(0x01)
#define INTERNAL2V2           ADC_REF(0x02)
#define INTERNAL4V096         ADC_REF(0x03)
#define EXTERNAL              ADC_REF(0x04)
#define INTERNAL1V1_CAP       ADC_REF(0x05)
#define INTERNAL2V2_CAP       ADC_REF(0x06)
#define INTERNAL4V096_CAP     ADC_REF(0x07)
#define INTERNAL1V1_NO_CAP    INTERNAL1V1
#define INTERNAL2V2_NO_CAP    INTERNAL2V2
#define INTERNAL4V096_NO_CAP  INTERNAL4V096
#define INTERNAL4V1           INTERNAL4V096
#define INTERNAL              INTERNAL1V1            /* deprecated */
#define INTERNAL4V            INTERNAL4V096          /* deprecated */
#define INTERNAL1V1_AREF      INTERNAL1V1_CAP        /* deprecated */
#define INTERNAL2V2_AREF      INTERNAL2V2_CAP        /* deprecated */
#define INTERNAL4V096_AREF    INTERNAL4V096_CAP      /* deprecated */
#define INTERNAL1V1NOBP       INTERNAL1V1            /* deprecated */
#define INTERNAL2V2NOBP       INTERNAL2V2            /* deprecated */
#define INTERNAL4V096NOBP     INTERNAL4V096          /* deprecated */

/* Gain Options */
#define GAIN_1X               (0xFF)
#define GAIN_20X              (0xFE)
#define GAIN_100X             (0xFD)
/* these are 255-GSEL, so that users can also just pass in 1, 20, or 100.
 * Wouldn't have had to do this except that 1 is the gain you get when you
 * pass 0 to the GSEL bits.
 * 0x03 is not a valid option for the GSEL bits. */


/* Special Single-Ended Channels */
#define ADC_TEMPERATURE ADC_CH(0x0C)
#define ADC_INTERNAL1V1 ADC_CH(0x0D)
#define ADC_GROUND      ADC_CH(0x0E)
#define ADC_VCC         ADC_CH(0x0F) /* Not very useful! */

/* Differential Channels */
#define DIFF_A0_A1      ADC_CH(0x10)
#define DIFF_A0_A3      ADC_CH(0x11)
#define DIFF_A1_A2      ADC_CH(0x12)
#define DIFF_A1_A3      ADC_CH(0x13)
#define DIFF_A2_A3      ADC_CH(0x14)
#define DIFF_A3_A4      ADC_CH(0x15)
#define DIFF_A3_A5      ADC_CH(0x16)
#define DIFF_A3_A6      ADC_CH(0x17)
#define DIFF_A3_A7      ADC_CH(0x18)
#define DIFF_A4_A5      ADC_CH(0x19)
#define DIFF_A4_A6      ADC_CH(0x1A)
#define DIFF_A4_A7      ADC_CH(0x1B)
#define DIFF_A5_A6      ADC_CH(0x1C)
#define DIFF_A5_A7      ADC_CH(0x1D)
#define DIFF_A6_A7      ADC_CH(0x1E)
#define DIFF_A8_A9      ADC_CH(0x1F)
#define DIFF_A0_A0      ADC_CH(0x20)
#define DIFF_A1_A1      ADC_CH(0x21)
#define DIFF_A2_A2      ADC_CH(0x22)
#define DIFF_A3_A3      ADC_CH(0x23)
#define DIFF_A4_A4      ADC_CH(0x24)
#define DIFF_A5_A5      ADC_CH(0x25)
#define DIFF_A6_A6      ADC_CH(0x26)
#define DIFF_A7_A7      ADC_CH(0x27)
#define DIFF_A8_A8      ADC_CH(0x28)
#define DIFF_A9_A9      ADC_CH(0x29)
#define DIFF_A10_A8     ADC_CH(0x2A)
#define DIFF_A10_A9     ADC_CH(0x2B)
#define DIFF_A11_A8     ADC_CH(0x2C)
#define DIFF_A11_A9     ADC_CH(0x2D)
#define DIFF_A1_A0      ADC_CH(0x30)
#define DIFF_A3_A0      ADC_CH(0x31)
#define DIFF_A2_A1      ADC_CH(0x32)
#define DIFF_A3_A1      ADC_CH(0x33)
#define DIFF_A3_A2      ADC_CH(0x34)
#define DIFF_A4_A3      ADC_CH(0x35)
#define DIFF_A5_A3      ADC_CH(0x36)
#define DIFF_A6_A3      ADC_CH(0x37)
#define DIFF_A7_A3      ADC_CH(0x38)
#define DIFF_A5_A4      ADC_CH(0x39)
#define DIFF_A6_A4      ADC_CH(0x3A)
#define DIFF_A7_A4      ADC_CH(0x3B)
#define DIFF_A6_A5      ADC_CH(0x3C)
#define DIFF_A7_A5      ADC_CH(0x3D)
#define DIFF_A7_A6      ADC_CH(0x3E)
#define DIFF_A9_A8      ADC_CH(0x3F)

/* Analog Comparators - not used by core */
#define ANALOG_COMP0_DDR               DDRA
#define ANALOG_COMP0_PORT              PORTA
#define ANALOG_COMP0_PIN               PINA
#define ANALOG_COMP0_AIN0_BIT          (1)
#define ANALOG_COMP0_AIN1_BIT          (2)

#define ANALOG_COMP1_DDR               DDRA
#define ANALOG_COMP1_PORT              PORTA
#define ANALOG_COMP1_PIN               PINA
#define ANALOG_COMP1_AIN0_BIT          (3)
#define ANALOG_COMP1_AIN1_BIT          (4)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a real SPI module and a slave-only TWI module
 * The included Wire.h will use the TWI hardware for TWI slave, or
 * a markedly inferior software TWI master implementation if that is requested.
 *---------------------------------------------------------------------------*/

/* Used by hardware I2C slave and software I2C master */
#define SCL                      PIN_PA4
#define SCL_PORT                 (PORTA)
#define SCL_PIN                  (4)
#define SDA                      PIN_PA6
#define SDA_PORT                 (PORTA)
#define SDA_PIN                  (6)

/* Hardware SPI */
#if defined(SET_REMAP) && SET_REMAP > 1
  /* remapped pins */
  #define MISO                     PIN_PA0
  #define MOSI                     PIN_PA1
  #define SS                       PIN_PA2
  #define SCK                      PIN_PA3

#else
  /* default pins */
  #define SCK                      PIN_PA4
  #define MISO                     PIN_PA5
  #define MOSI                     PIN_PA6
  #define SS                       PIN_PA7
#endif

/* Two hardware serial ports */
#if defined(SET_REMAP) && SET_REMAP & 1
  /* alt pins */
  #define PIN_HWSERIAL0_TX         PIN_PA7
  #define PIN_HWSERIAL0_RX         PIN_PB2
#else
  /* default pins */
  #define PIN_HWSERIAL0_TX         PIN_PA1
  #define PIN_HWSERIAL0_RX         PIN_PA2
#endif
#define PIN_HWSERIAL1_TX         PIN_PA5
#define PIN_HWSERIAL1_RX         PIN_PA4


#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL ATTINY841/441  ATTinyCore Legacy (Counterclockwise) Pin Mapping
 *
 *                                +-\/-+
 *                          VCC  1|    |14  GND
 *                 (A11 0)  PB0  2|x   |13  PA0  (A0 10)     *MISO   AREF
 *                 (A10 1)  PB1  3|x   |12  PA1  (A1  9) TX0 *MOSI   *PWM
 *           RESET (A9 11)  PB3  4|    |11  PA2  (A2  8) RX0 *SS     *PWM
 *  PWM INT0  *RX0 (A8  2)  PB2  5|    |10  PA3  (A3  7)     *SCK     PWM
 *  PWM SS    *TX0 (A7  3)  PA7  6|    |9   PA4  (A4  6) RX1 SCK  SCL PWM
 *  PWM SDA   MOSI (A6  4)  PA6  7|    |8   PA5  (A5  5) TX1 MISO     PWM
 *                                +----+
 *
 *  Some pins can be remapped, remapped options for peripherals marked with a *. The default for those same options is shown without the *.
 *  Any of the 8 PWM pins can use any of the three timers, TOCC0, 2, 4, and 6 use channel A, and TOCC1, 3, 5, and 7 use channel B.
 *---------------------------------------------------------------------------*/

#warning "This is the COUNTERCLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in counter clockwise order"
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

const uint8_t PROGMEM port_to_pullup_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PUEA,
  (uint8_t)(uint16_t) &PUEB,
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
  PA, /* 3 */
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PB, /* 11 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0), /* 0 PB0 */
  _BV(1),
  _BV(2),
  _BV(7), /* 3 PA7 */
  _BV(6),
  _BV(5),
  _BV(4),
  _BV(3),
  _BV(2),
  _BV(1),
  _BV(0),
  _BV(3), /* RESET PB3 */
};


const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TOCC7 | TIMER2A,  /* OCR2A */
  TOCC6 | TIMER2B,  /* OCR2B */
  TOCC5 | TIMER1A,  /* OCR1A */
  TOCC4 | TIMER0B,  /* OCR0B this is serial 1 TX */
  TOCC3 | TIMER0A,  /* OCR0A this is serial 1 RX, too, so let's give it the least desirable timer */
  TOCC2 | TIMER1B,  /* OCR1B */
  TOCC1 | NOT_ON_TIMER,   /* this is serial 0 RX */
  TOCC0 | NOT_ON_TIMER,   /* this is serial 0 TX, lets not use it for pwm */
  NOT_ON_TIMER,
  NOT_ON_TIMER      /* RESET */
};

#endif
#endif
