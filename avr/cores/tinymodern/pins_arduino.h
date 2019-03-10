/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 14-10-2009 for attiny45 Saposoft
  Modified 26-02-2015 for attiny841 and other parts that use the PUEx registers.
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#include "core_build_options.h"




#define NOT_A_PIN 0
#define NOT_A_PORT 0

#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER2A 5
#define TIMER2B 6

//changed it to uint16_t to uint8_t
extern const uint8_t PROGMEM digital_pin_to_port_PGM[];
// extern const uint8_t PROGMEM digital_pin_to_bit_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
//extern const uint8_t PROGMEM digital_pin_to_timer_PGM[];

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
// These perform slightly better as macros compared to inline functions
//
//#define digitalPinToTimer(P) ( pgm_read_byte( digital_pin_to_timer_PGM + (P) ) )
#define analogInPinToBit(P) (P)

// in the following lines modified pgm_read_word in pgm_read_byte, word doesn't work on attiny45

#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )

#if defined(__AVR_ATtiny1634__) || defined(__AVR_ATtiny828__)
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define portOutputRegister(P) ( (volatile uint8_t *)(uint16_t)( pgm_read_byte( port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)(uint16_t)( pgm_read_byte( port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)(uint16_t)( pgm_read_byte( port_to_mode_PGM + (P))) )
#define portPcMaskRegister(P) ( (volatile uint8_t *)(uint16_t)( pgm_read_byte( port_to_pcmask_PGM + (P))) )
#define portPullupRegister(P) ( (volatile uint8_t *)(uint16_t)( pgm_read_byte( port_to_pullup_PGM + (P))) )
extern const uint8_t PROGMEM port_to_mode_PGM[];
extern const uint8_t PROGMEM port_to_input_PGM[];
extern const uint8_t PROGMEM port_to_output_PGM[];
extern const uint8_t PROGMEM port_to_pcmask_PGM[];
extern const uint8_t PROGMEM port_to_pullup_PGM[];

#endif


#if defined(__AVR_ATtinyX41__)

#define PORT_A_ID 1
#define PORT_B_ID 2

#ifdef OLD_PINOUT

#define SS      3
#define MISO    5
#define MOSI    4
#define SCK     6
#define MOSI_REMAP 9
#define MISO_REMAP 10
#define SCK_REMAP 7
#define SS_REMAP 8


#define ADC_TEMPERATURE 12

// use ternaries for this to save space.
#define digitalPinToPort(P) (P==11?2:(P<3?2:1))
#define portOutputRegister(P) (P==1?&PORTA:(P?&PORTB:NOT_A_PORT))
#define portInputRegister(P)  (P==1?&PINA:(P?&PINB:NOT_A_PORT ))
#define portModeRegister(P)   (P==1?&DDRA:(P?&DDRB:NOT_A_PORT ))
#define portPcMaskRegister(P) (P==1?&PCMSK0:(P?&PCMSK2:NOT_A_PORT ))
#define portPullupRegister(P) (P==1?&PUEA:(P?&PUEB:NOT_A_PORT ))

#define analogInputToDigitalPin(p)  ((p < 9) ? 10 -(p): (p==9)?11:(11-(p)))

#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 10) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) <= 2) ? PCIE1 : PCIE0)
#define digitalPinToPCMSK(p)    (((p) <= 2) ? (&PCMSK1) : (((p) <= 10) ? (&PCMSK0) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) <= 2) ? (p) : (10 - (p)))
#else //New pinout
#define SS      7
#define MISO    5
#define MOSI    6
#define SCK     4
#define MOSI_REMAP 1
#define MISO_REMAP 0
#define SCK_REMAP 3
#define SS_REMAP 2


#define ADC_TEMPERATURE 12

// use ternaries for this to save space.
#define digitalPinToPort(P) (P<8?1:2)
#define portOutputRegister(P) (P==1?&PORTA:(P?&PORTB:NOT_A_PORT))
#define portInputRegister(P)  (P==1?&PINA:(P?&PINB:NOT_A_PORT ))
#define portModeRegister(P)   (P==1?&DDRA:(P?&DDRB:NOT_A_PORT ))
#define portPcMaskRegister(P) (P==1?&PCMSK0:(P?&PCMSK2:NOT_A_PORT ))
#define portPullupRegister(P) (P==1?&PUEA:(P?&PUEB:NOT_A_PORT ))

#define analogInputToDigitalPin(p)  (p<9?p:(p==11?9:p-1))

#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 10) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) > 7) ? PCIE1 : PCIE0)
#define digitalPinToPCMSK(p)    (((p)  < 8) ? (&PCMSK0) : (((p) <= 11) ? (&PCMSK0) : ((uint8_t *)NULL)))
#define digitalPinToPCMSKbit(p) (((p) <8 ) ? (p) : ((p==8?2:(p==11?3:(p==9?1:0)))))
#endif

#endif

#if defined(__AVR_ATtiny1634__)
#define PORT_A_ID 1
#define PORT_B_ID 2
#define PORT_C_ID 3

//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)

#define SS 13
#define MISO 16
#define MOSI 15
#define SCK 12
#define USI_DDR_PORT DDRB
#define USI_SCK_PORT DDRC
#define USCK_DD_PIN DDRC1
#define DO_DD_PIN DDRB2
#define DI_DD_PIN DDRB1


#define ADC_TEMPERATURE 30

#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define DDR_USI_CL DDRC
#  define PORT_USI_CL PORTC
#  define PIN_USI_CL PINC
#  define PORT_USI_SDA PORTB1
#  define PORT_USI_SCL PORTC1
#  define PIN_USI_SDA PINB1
#  define PIN_USI_SCL PINC1
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif


#define digitalPinToPCX(p,s1,s2,s3,s4,s5,s6) \
    (((p) >= 0) \
        ? (((p) <   1) ? (s1)  /*  0  -  0  ==>  B0      */  \
        : (((p) <=  8) ? (s2)  /*  1  -  8  ==>  A7 - A0 */  \
        : (((p) <= 10) ? (s3)  /*  9  - 10  ==>  C5 - C4 */  \
        : (((p) <= 13) ? (s4)  /*  11 - 13  ==>  C2 - C0 */  \
        : (((p) <= 16) ? (s5)  /*  14 - 16  ==>  B3 - B1 */  \
        : (s6)))))) \
        : (s6))
//                                                   s1 b     s2 A     s3 C     s3 C     s5 B
#define digitalPinToPCICR(p)    digitalPinToPCX( p, &GIMSK,  &GIMSK,  &GIMSK,  &GIMSK,  &GIMSK,  NULL )
#define digitalPinToPCICRbit(p) digitalPinToPCX( p, PCIE1,   PCIE0,   PCIE2,   PCIE2,   PCIE1,   0    )
#define digitalPinToPCMSK(p)    digitalPinToPCX( p, &PCMSK1, &PCMSK0, &PCMSK2, &PCMSK2, &PCMSK1, NULL )
#define digitalPinToPCMSKbit(p) digitalPinToPCX( p, p,       8-p,     14-p,    13-p,    17-p,    0    )


#define analogInputToDigitalPin(p)  ((p < 6) ? 5 -(p):(22-(p)))

#endif

#if defined(__AVR_ATtiny828__)
#define PORT_A_ID 1
#define PORT_B_ID 2
#define PORT_C_ID 3
#define PORT_D_ID 4
#define SS 16
#define MISO 25
#define MOSI 24
#define SCK 27


#define ADC_TEMPERATURE 30

#define analogInputToDigitalPin(p)  (p)

#define digitalPinToPCICR(p)    (&PCICR)
#define digitalPinToPCICRbit(p) (1<<(p>>3))
#define digitalPinToPCMSK(p)    (p<8?&PCMSK0:(p<16?&PCMSK1:(p<24?&PCMSK2:&PCMSK3)))
#define digitalPinToPCMSKbit(p) (1<<(p&7))



#endif

#endif //end of include guard
