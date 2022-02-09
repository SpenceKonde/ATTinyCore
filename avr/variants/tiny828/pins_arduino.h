/* pins_arduino.h - Pin definition functions for ATTinyCore
   Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
   Copyright (c) 2015~2021 Spence Konde, (c) 2007 David A. Mellis
   Free Software - GPL 2.1, please see LICENCE.md for details */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>

/*===========================================================================
 * Microchip ATtiny828
 *===========================================================================
 * ATTinyCore Standard Pin Mapping
 *---------------------------------------------------------------------------*/

#define NUM_DIGITAL_PINS            28
#define NUM_ANALOG_INPUTS           28

/* Basic Pin Numbering - PIN_Pxn notation is always recommended
 * as it is totally unambiguous, but numbers may be used too */
#define PIN_PA0  ( 0)
#define PIN_PA1  ( 1)
#define PIN_PA2  ( 2)
#define PIN_PA3  ( 3)
#define PIN_PA4  ( 4)
#define PIN_PA5  ( 5)
#define PIN_PA6  ( 6)
#define PIN_PA7  ( 7)
#define PIN_PB0  ( 8)
#define PIN_PB1  ( 9)
#define PIN_PB2  (10)
#define PIN_PB3  (11)
#define PIN_PB4  (12)
#define PIN_PB5  (13)
#define PIN_PB6  (14)
#define PIN_PB7  (15)
#define PIN_PC0  (16)
#define PIN_PC1  (17)
#define PIN_PC2  (18)
#define PIN_PC3  (19)
#define PIN_PC4  (20)
#define PIN_PC5  (21)
#define PIN_PC6  (22)
#define PIN_PC7  (23)
#define PIN_PD0  (24)
#define PIN_PD1  (25)
#define PIN_PD2  (26)  /* RESET */
#define PIN_PD3  (27)

/* PIN_An is the digital pin with analog channel An on it. */
#define PIN_A0     (PIN_PA0)
#define PIN_A1     (PIN_PA1)
#define PIN_A2     (PIN_PA2)
#define PIN_A3     (PIN_PA3)
#define PIN_A4     (PIN_PA4)
#define PIN_A5     (PIN_PA5)
#define PIN_A6     (PIN_PA6)
#define PIN_A7     (PIN_PA7)
#define PIN_A8     (PIN_PB0)
#define PIN_A9     (PIN_PB1)
#define PIN_A10     (PIN_PB2)
#define PIN_A11     (PIN_PB3)
#define PIN_A12     (PIN_PB4)
#define PIN_A13     (PIN_PB5)
#define PIN_A14     (PIN_PB6)
#define PIN_A15     (PIN_PB7)
#define PIN_A16     (PIN_PC0)
#define PIN_A17     (PIN_PC1)
#define PIN_A18     (PIN_PC2)
#define PIN_A19     (PIN_PC3)
#define PIN_A20     (PIN_PC4)
#define PIN_A21     (PIN_PC5)
#define PIN_A22     (PIN_PC6)
#define PIN_A23     (PIN_PC7)
#define PIN_A24     (PIN_PD0)
#define PIN_A25     (PIN_PD1)
#define PIN_A26     (PIN_PD2)
#define PIN_A27     (PIN_PD3)

static const uint8_t  A0 = ADC_CH(0);
static const uint8_t  A1 = ADC_CH(1);
static const uint8_t  A2 = ADC_CH(2);
static const uint8_t  A3 = ADC_CH(3);
static const uint8_t  A4 = ADC_CH(4);
static const uint8_t  A5 = ADC_CH(5);
static const uint8_t  A6 = ADC_CH(6);
static const uint8_t  A7 = ADC_CH(7);
static const uint8_t  A8 = ADC_CH(8);
static const uint8_t  A9 = ADC_CH(9);
static const uint8_t A10 = ADC_CH(10);
static const uint8_t A11 = ADC_CH(11);
static const uint8_t A12 = ADC_CH(12);
static const uint8_t A13 = ADC_CH(13);
static const uint8_t A14 = ADC_CH(14);
static const uint8_t A15 = ADC_CH(15);
static const uint8_t A16 = ADC_CH(16);
static const uint8_t A17 = ADC_CH(17);
static const uint8_t A18 = ADC_CH(18);
static const uint8_t A19 = ADC_CH(19);
static const uint8_t A20 = ADC_CH(20);
static const uint8_t A21 = ADC_CH(21);
static const uint8_t A22 = ADC_CH(22);
static const uint8_t A23 = ADC_CH(23);
static const uint8_t A24 = ADC_CH(24);
static const uint8_t A25 = ADC_CH(25);
static const uint8_t A26 = ADC_CH(26);
static const uint8_t A27 = ADC_CH(27);


/* Interrupt macros to go from pin to PCMSK register and bit within it, and
 * the register to enable/disable banks of PCINTs, and bit within it PCICR
 * is almost always the same for all PCINTs; but must return null pointer
 * if the pin is invalid. The PCICRbit and PCMSK are almost always directly
 * mapped to port; particularly on ugly mappings like this, taking advantage
 * of this is more efficient and easier to write.
 * digitalPinToInterrupt gets the number of the "full service" pin interrupt
 *---------------------------------------------------------------------------*/

#define digitalPinToPCICR(p)        (&PCICR)
#define digitalPinToPCICRbit(p)     ((p) >> 3)
#define digitalPinToPCMSK(p)        ((p) > 23 ? (&PCMSK3) : (&PCMSK0 + (p >> 3)))
#define digitalPinToPCMSKbit(p)     ((p) & 0x07)

#define digitalPinToInterrupt(p)    ((p) == PIN_PC1 ? 0 : ((p) == PIN_PC2 ? 1 : NOT_AN_INTERRUPT))

/* Analog Channel <-> Digital Pin macros */
#define analogInputToDigitalPin(p)  ((p < 28) ? (p): -1)
#define digitalPinToAnalogInput(p)  ((p < 28) ? (p): -1)

/* Which pins have PWM? */
#define digitalPinHasPWM(p)         ((p) == PIN_PC0 || (p) == PIN_PC4 || (p) == PIN_PC5 || (p) == PIN_PC6)


/*---------------------------------------------------------------------------
 * Core Configuration where these are not the defaults
 *---------------------------------------------------------------------------*/

// Choosing not to initialise saves flash.   1 = initialise.
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
 * have additional non-standard behavior allowing the remapping of output from
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
work for it unless or until digitalWrite() uis called on it.
Methods that use the standard ways to turn off PWM will prevent it from working
afterwards if you then try to go back to core-provided PWM functions. The
"standard" way uses the COMnx0 and COMnx1 bits in the TCCRny registers. We set
those at power on only (so it has no space cost because we have to set them
anyway) and instead just use TOCPMCOE bits to control whether PWM is output */

#define TIMER0_TYPICAL              (1)
#define PIN_TIMER_OC0A              (PIN_PC0) /* core default */
#define PIN_TIMER_OC0B              (PIN_PC4) /* core default */
#define PIN_TIMER_T0                (PIN_PC5)

#define TIMER1_TYPICAL              (1)
#define PIN_TIMER_OC1A              (PIN_PC5) /* core default */
#define PIN_TIMER_OC1B              (PIN_PC7) /* core default */
#define PIN_TIMER_T1                (PIN_PC7)
#define PIN_TIMER_ICP1              (PIN_PC5)



/*---------------------------------------------------------------------------
 * Chip Features (or lack thereof) - Analog stuff
 *---------------------------------------------------------------------------
 * Analog reference constants are pre-shifted to their final position in the
 * registers to avoid leftshifting at runtime, which is surprisingly slow and
 * wasteful of flash.
 * Note: REFS in slightly different place on these - the only surviving bit
 * of the ADMUXB register, standing all alone sobbing. He was the one "lucky"
 * enough to make it out of the foundry... but what kind of a life is this?
 * All alone in the crumby ADC of a forgotten, unloved device lacking the
 * feature that was to be it's claim to fame?
 *---------------------------------------------------------------------------*/
#define ADC_REF(x)            (x << 5)

/* Analog Reference bit masks */
#define DEFAULT          ADC_REF(0x00)
#define INTERNAL1V1      ADC_REF(0x01)
#define INTERNAL           INTERNAL1V1 /* deprecated */
/* Special Analog Channels */
#define ADC_GROUND        ADC_CH(0x1C)
#define ADC_INTERNAL1V1   ADC_CH(0x1D)
#define ADC_TEMPERATURE   ADC_CH(0x1E)
#define ADC_VCC           ADC_CH(0x1F)

/* Not a differential ADC */

/* Analog Comparator - not used by core */
#define ANALOG_COMP_DDR           DDRA
#define ANALOG_COMP_PORT         PORTA
#define ANALOG_COMP_PIN           PINA
#define ANALOG_COMP_AIN0_BIT       (1)
#define ANALOG_COMP_AIN1_BIT       (2)

/*---------------------------------------------------------------------------
 * Chip Features - SPI, I2C, USART, etc
 *---------------------------------------------------------------------------*/
/* This part has a real SPI module and a slave-only TWI module
 * The included Wire.h will use the TWI hardware for TWI slave, or
 * a markedly inferior software TWI master implementation if that is requested.
 * worse still, the TWI is on the same pins as the SPI!
 * Note that we change the pins that I2C master uses only as long as user
 * selects master only mode from the submenus.
 * PD0, PD3 used by SPI too, better to use something else
 * and since it's software I2C, we can pick any pin. PA4, PA5 seem
 * non-specifically-useful for other purposes, so let's use them...
 *---------------------------------------------------------------------------*/

#if defined(WIRE_MASTER_ONLY)
  /* Software I2C Master */
  #define SDA PIN_PA4
  #define SCL PIN_PA5
  #define BIT_SDA 4
  #define PORT_SDA PORTA
  #define BIT_SCL 5
  #define PORT_SCL PORTA
#else
  /* Hardware I2C Slave with or without Software I2C Master */
  #define SCL  PIN_PD3
  #define SDA  PIN_PD0
  #define BIT_SDA 0
  #define PORT_SDA PORTD
  #define BIT_SCL 3
  #define PORT_SCL PORTD
#endif

/* Hardware SPI */
#define MISO PIN_PD1
#define MOSI PIN_PD0
#define SCK  PIN_PD3
#define SS   PIN_PC0

/* One hardware serial port */
#define PIN_HWSERIAL0_TX         PIN_PC3
#define PIN_HWSERIAL0_RX         PIN_PC2

#ifdef ARDUINO_MAIN
/*---------------------------------------------------------------------------
 * ATMEL/Microchip ATtiny828 ATTinyCore Standard Pin Mapping
 *
 *            INT0    RESET
 *               16*   26   24   14
 *            17    27   25   15
 *               PC0  PD2  PD0  PB6
 *            PC1  PD3  PD1   PB7
 *    INT1     _________________
 * 18 RX  PC2 | *               | PB5   13
 * 19 TX  PC3 |                 | PB4   12
 * 20 *   PC4 |                 | PB3   11
 *        VCC |                 | GND
 *        GND |                 | PB2   10
 * 21 *   PC5 |                 | PB1    9
 * 22 *   PC6 |                 | AVCC
 * 23     PC7 |_________________| PB0    8
 *            PA0  PA2  PA4  PA6
 *              PA1  PA3  PA5  PA7
 *             0    2    4    6
 *               1    3    5    7
 *
 * * indicates PWM pin
 * ADC input on all pins.
 *---------------------------------------------------------------------------*/


const uint8_t PROGMEM port_to_mode_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &DDRA,
  (uint8_t)(uint16_t) &DDRB,
  (uint8_t)(uint16_t) &DDRC,
  (uint8_t)(uint16_t) &DDRD
};

const uint8_t PROGMEM port_to_output_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PORTA,
  (uint8_t)(uint16_t) &PORTB,
  (uint8_t)(uint16_t) &PORTC,
  (uint8_t)(uint16_t) &PORTD
};

const uint8_t PROGMEM port_to_pullup_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PUEA,
  (uint8_t)(uint16_t) &PUEB,
  (uint8_t)(uint16_t) &PUEC,
  (uint8_t)(uint16_t) &PUED
};

const uint8_t PROGMEM port_to_input_PGM[] = {
  NOT_A_PORT,
  (uint8_t)(uint16_t) &PINA,
  (uint8_t)(uint16_t) &PINB,
  (uint8_t)(uint16_t) &PINC,
  (uint8_t)(uint16_t) &PIND
};


const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PB,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PD,
  PD,
  PD, /*RESET*/
  PD
};


const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0),
  _BV(1),
  _BV(2), /*RESET*/
  _BV(3),
};

/* Include the TOCCn's in order to use the TOCPMOE register to easily turn PWM on and off. */
const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
  NOT_ON_TIMER, /*  PA0 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, /*  PB0 */
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TOCC0 | TIMER0A,       /*  PC1 */
  TOCC1 | NOT_ON_TIMER,  /*  PC2 */
  TOCC2 | NOT_ON_TIMER,  /*  PC3 */
  TOCC3 | NOT_ON_TIMER,  /*  PC4 */
  TOCC4 | TIMER0B,       /*  PC5 */
  TOCC5 | TIMER1A,       /*  PC6 */
  TOCC6 | NOT_ON_TIMER,  /* CLKI */
  TOCC7 | TIMER1B,       /*  PC7 */
  NOT_ON_TIMER, /*  PD0 */
  NOT_ON_TIMER,
  NOT_ON_TIMER, /* RESET*/
  NOT_ON_TIMER
};

#endif

#endif
