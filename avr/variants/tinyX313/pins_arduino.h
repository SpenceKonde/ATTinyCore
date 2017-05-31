
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#define ATTINYX313 1       //backwards compatibility
#define __AVR_ATtinyX313__ //recommended
#define USE_SOFTWARE_SPI 1

#define LED_BUILTIN (13) 

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            18
#define NUM_ANALOG_INPUTS           0
#define analogInputToDigitalPin(p)  -1

#define digitalPinHasPWM(p)         ((p) == 7 || (p) == 11 || (p) == 12 || (p) == 13)

//This part has a USI, not an SPI module. Accordingly, there is no MISO/MOSI in hardware. There's a DI and a DO. When the chip is used as master, DI is used as MISO, DO is MOSI; the defines here specify the pins for master mode, as SPI master is much more commonly used in Arduino-land than SPI slave, and these defines are required for compatibility. Be aware of this when using the USI SPI fucntionality (and also, be aware that the MISO and MOSI markings on the pinout diagram in the datasheet are for ISP programming, where the chip is a slave. The pinout diagram included with this core attempts to clarify this)


#define SS   10
#define MOSI 15
#define MISO 14
#define SCK  16

#define USI_DDR_PORT DDRB
#define USI_SCK_PORT DDRB
#define USCK_DD_PIN DDB7
#define DO_DD_PIN DDB6
#define DI_DD_PIN DDB5
#  define DDR_USI DDRB
#  define PORT_USI PORTB
#  define PIN_USI PINB
#  define PORT_USI_SDA PORTB5
#  define PORT_USI_SCL PORTB7
#  define PIN_USI_SDA PINB5
#  define PIN_USI_SCL PINB7
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#  define DDR_USI_CL DDR_USI
#  define PORT_USI_CL PORT_USI
#  define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif

static const uint8_t SDA = 14;
static const uint8_t SCL = 16;

//----------------------------------------------------------
//----------------------------------------------------------
//Core Configuration (used to be in core_build_options.h)

//If Software Serial communications doesn't work, run the TinyTuner sketch provided with the core to give you a calibrated OSCCAL value.
//Change the value here with the tuned value. By default this option uses the default value which the compiler will optimise out. 
#define TUNED_OSCCAL_VALUE                        OSCCAL
//e.g
//#define TUNED_OSCCAL_VALUE                        0x57


//Choosing not to initialise saves power and flash. 1 = initialise.
#ifndef INITIALIZE_SECONDARY_TIMERS
#define INITIALIZE_SECONDARY_TIMERS              0
#endif
/*
  The old standby ... millis on Timer 0.
*/
#define TIMER_TO_USE_FOR_MILLIS                   0

/*
  Where to put the software serial? (Arduino Digital pin numbers)
*/
//WARNING, if using software, TX is on AIN0, RX is on AIN1. Comparator is favoured to use its interrupt for the RX pin.
#define USE_SOFTWARE_SERIAL						  0
//Please define the port on which the analog comparator is found.
#define ANALOG_COMP_DDR						 	  DDRB
#define ANALOG_COMP_PORT						  PORTB
#define ANALOG_COMP_PIN						 	  PINB
#define ANALOG_COMP_AIN0_BIT					  0
#define ANALOG_COMP_AIN1_BIT					  1


/*
  Analog reference bit masks.
*/
//No ADC, so no reference masks.


//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------


#define digitalPinToPCICR(p)    (((p) >= 9 && (p) <= 16) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (5)
#define digitalPinToPCMSK(p)    (((p) >= 9 && (p) <= 16) ? (&PCMSK) : ((uint8_t *)NULL))
#define digitalPinToPCMSKbit(p) ((p) - 9)


#define digitalPinToInterrupt(p)  ((p) == 5 ? 0 : ((p)==4?1: NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN
// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATTINY2313
//
//                   +-\/-+
//      (D 17) PA2  1|    |20  VCC
// RX   (D  0) PD0  2|    |19  PB7 (D  16)
// TX   (D  1) PD1  3|    |18  PB6 (D  15)
//      (D  2) PA1  4|    |17  PB5 (D  14)
//      (D  3) PA0  5|    |16  PB4 (D  13)*
// INT0 (D  4) PD2  6|    |15  PB3 (D  12)*
// INT1 (D  5) PD3  7|    |14  PB2 (D  11)*
//      (D  6) PD4  8|    |13  PB1 (D  10)
//     *(D  7) PD5  9|    |12  PB0 (D  9)
//             GND 10|    |11  PD6 (D  8)
//                   +----+
//
// * indicates PWM port

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&DDRA,
	(uint16_t)&DDRB,
	NOT_A_PORT,
	(uint16_t)&DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&PORTA,
	(uint16_t)&PORTB,
	NOT_A_PORT,
	(uint16_t)&PORTD,
};

const uint16_t PROGMEM port_to_input_PGM[] = 
{
	NOT_A_PORT,
	(uint16_t)&PINA,
	(uint16_t)&PINB,
	NOT_A_PORT,
	(uint16_t)&PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = 
{
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

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = 
{
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

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = 
{
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

#endif



//Old code, just here for temporary backup until I decide it is not needed.
/*//WARNING, if using software, RX must be on a pin which has a Pin change interrupt <= 7 (e.g. PCINT6, or PCINT1, but not PCINT8)
#define USE_SOFTWARE_SERIAL						  0
//These are set to match Optiboot pins.
#define SOFTWARE_SERIAL_PORT 					  PORTB
#define SOFTWARE_SERIAL_TX 						  10
#define SOFTWARE_SERIAL_PIN 					  PINB
#define SOFTWARE_SERIAL_RX 						  9*/
