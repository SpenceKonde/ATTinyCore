/*
<SoftSerial> library is exactly the same as the <SoftwareSerial> library but used with the <TinyPinChange> library which allows to share
the Pin Change Interrupt Vector.
<SoftwareSerial> monopolizes the Pin Change Interrupt Vector and don't allow sharing.
With <SoftSerial>, it's possible. Don't forget to #include <TinyPinChange> in your sketch!
Additionally, for small devices such as ATtiny85 (Digispark), it's possible to declare the same pin for TX and RX.
Data direction is set by using the new txMode() and rxMode() methods.
RC Navy (2012-2015): http://p.loussouarn.free.fr

SoftwareSerial.cpp (formerly NewSoftSerial.cpp) - 
Multi-instance software serial library for Arduino/Wiring
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
http://arduiniana.org.
*/

// When set, _DEBUG co-opts pins 11 and 13 for debugging with an
// oscilloscope or logic analyzer.  Beware: it also slightly modifies
// the bit times, so don't rely on it too much at high baud rates
#define _DEBUG 0
#define _DEBUG_PIN1 0//11
#define _DEBUG_PIN2 1//13
#define FAST_DEBUG //less intrusive
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 
// 
// Includes
// 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "SoftSerial.h"
//
// Lookup table
//
typedef struct _DELAY_TABLE
{
  long baud;
  unsigned short rx_delay_centering;
  unsigned short rx_delay_intrabit;
  unsigned short rx_delay_stopbit;
  unsigned short tx_delay;
} DELAY_TABLE;

#if F_CPU == 16000000

static const DELAY_TABLE PROGMEM table[] = 
{
  //  baud    rxcenter   rxintra    rxstop    tx
  { 115200,   0,         14,        14,       12,    },
  { 57600,    5,         34,        34,       32,    },
  { 38400,    15,        54,        54,       52,    },
  { 31250,    23,        67,        67,       65,    },/* By interpolation */
  { 28800,    26,        74,        74,       72,    },/* By interpolation */
  { 19200,    44,        113,       113,      112,   },
  { 14400,    74,        156,       153,      153,   },/* By interpolation */
  { 9600,     114,       234,       234,      233,   },
  { 4800,     233,       474,       474,      471,   },
  { 2400,     471,       940,       940,      945,   },
  { 1200,     947,       1902,      1902,     1895,  },
  { 300,      3804,      7617,      7617,     7614,  },
};
const int XMIT_START_ADJUSTMENT = 0;
#elif F_CPU == 16500000

static const DELAY_TABLE PROGMEM table[] = 
{
  //  baud    rxcenter   rxintra    rxstop    tx
  { 115200,   0,         15,        15,       13,    },
  { 57600,    3,         35,        35,       33,    },
  { 38400,    12,        56,        56,       54,    },
  { 31250,    32,        72,        72,       70,    },/* By interpolation */
  { 28800,    35,        79,        79,       76,    },/* By interpolation */
  { 19200,    52,        118,       118,      116,   },
  { 14400,    76,        161,       161,      158,   },/* By interpolation */
  { 9600,     118,       241,       241,      238,   },
  { 4800,     240,       487,       487,      485,   },
  { 2400,     486,       976,       976,      974,   },
  { 1200,     977,       1961,      1961,     1956,  },
  { 600,      1961,      3923,      3923,     3919,  },
  { 300,      3923,      7855,      7855,     7852,  },
};

const int XMIT_START_ADJUSTMENT = 0;
#elif F_CPU == 8000000

static const DELAY_TABLE table[] PROGMEM = 
{
  //  baud    rxcenter    rxintra    rxstop  tx
  { 115200,   1,          5,         5,      3,      },
  { 57600,    1,          15,        15,     13,     },
  { 38400,    2,          25,        26,     23,     },
  { 31250,    7,          32,        33,     29,     },
  { 28800,    11,         35,        35,     32,     },
  { 19200,    20,         55,        55,     52,     },
  { 14400,    30,         75,        75,     72,     },
  { 9600,     50,         114,       114,    112,    },
  { 4800,     110,        233,       233,    230,    },
  { 2400,     229,        472,       472,    469,    },
  { 1200,     467,        948,       948,    945,    },
  { 300,      1895,       3805,      3805,   3802,   },
};

const int XMIT_START_ADJUSTMENT = 4;

#elif F_CPU == 20000000

// 20MHz support courtesy of the good people at macegr.com.
// Thanks, Garrett!

static const DELAY_TABLE PROGMEM table[] =
{
  //  baud    rxcenter    rxintra    rxstop  tx
  { 115200,   3,          21,        21,     18,     },
  { 57600,    20,         43,        43,     41,     },
  { 38400,    37,         73,        73,     70,     },
  { 31250,    45,         89,        89,     88,     },
  { 28800,    46,         98,        98,     95,     },
  { 19200,    71,         148,       148,    145,    },
  { 14400,    96,         197,       197,    194,    },
  { 9600,     146,        297,       297,    294,    },
  { 4800,     296,        595,       595,    592,    },
  { 2400,     592,        1189,      1189,   1186,   },
  { 1200,     1187,       2379,      2379,   2376,   },
  { 300,      4759,       9523,      9523,   9520,   },
};

const int XMIT_START_ADJUSTMENT = 6;

#else

#error This version of SoftSerial supports only 20, 16, 16.5 and 8MHz processors

#endif

//
// Statics
//
SoftSerial *SoftSerial::active_object = 0;
char SoftSerial::_receive_buffer[_SS_MAX_RX_BUFF]; 
volatile uint8_t SoftSerial::_receive_buffer_tail = 0;
volatile uint8_t SoftSerial::_receive_buffer_head = 0;

//
// Debugging
//
// This function generates a brief pulse
// for debugging or measuring on an oscilloscope.
#if _DEBUG
#if defined(FAST_DEBUG)
#define DebugPulse(a, bit) sbi(PINB, bit)
#else
inline void DebugPulse(uint8_t pin, uint8_t count)
{
  volatile uint8_t *pport = portOutputRegister(digitalPinToPort(pin));

  uint8_t val = *pport;
  while (count--)
  {
    *pport = val | digitalPinToBitMask(pin);
    *pport = val;
  }
}
#endif
#else
// no debug
inline void DebugPulse(uint8_t pin, uint8_t count)
{
}
#endif
//
// Private methods
//

/* static */ 
inline void SoftSerial::tunedDelay(uint16_t delay) { 
  uint8_t tmp=0;

  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
    : "+r" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

// This function sets the current object as the "listening"
// one and returns true if it replaces another 
bool SoftSerial::listen()
{
  if (active_object != this)
  {
    _buffer_overflow = false;
    uint8_t oldSREG = SREG;
    cli();
    _receive_buffer_head = _receive_buffer_tail = 0;
    active_object = this;
    SREG = oldSREG;
    return true;
  }

  return false;
}

//
// The receive routine called by the interrupt handler
//
void SoftSerial::recv()
{

#if GCC_VERSION < 40302
// Work-around for avr-gcc 4.3.0 OSX version bug
// Preserve the registers that the compiler misses
// (courtesy of Arduino forum user *etracer*)
  asm volatile(
    "push r18 \n\t"
    "push r19 \n\t"
    "push r20 \n\t"
    "push r21 \n\t"
    "push r22 \n\t"
    "push r23 \n\t"
    "push r26 \n\t"
    "push r27 \n\t"
    ::);
#endif  

  uint8_t d = 0;

  // If RX line is high, then we don't see any start bit
  // so interrupt is probably not for us
  if (_inverse_logic ? rx_pin_read() : !rx_pin_read())
  {
    // Wait approximately 1/2 of a bit width to "center" the sample
    tunedDelay(_rx_delay_centering);
    DebugPulse(_DEBUG_PIN2, 1);

    // Read each of the 8 bits
    for (uint8_t i=0x1; i; i <<= 1)
    {
      tunedDelay(_rx_delay_intrabit);
      DebugPulse(_DEBUG_PIN2, 1);
      uint8_t noti = ~i;
      if (rx_pin_read())
        d |= i;
      else // else clause added to ensure function timing is ~balanced
        d &= noti;
    }

    // skip the stop bit
    tunedDelay(_rx_delay_stopbit);
    DebugPulse(_DEBUG_PIN2, 1);

    if (_inverse_logic)
      d = ~d;

    // if buffer full, set the overflow flag and return
    if ((_receive_buffer_tail + 1) % _SS_MAX_RX_BUFF != _receive_buffer_head) 
    {
      // save new data in buffer: tail points to where byte goes
      _receive_buffer[_receive_buffer_tail] = d; // save new byte
      _receive_buffer_tail = (_receive_buffer_tail + 1) % _SS_MAX_RX_BUFF;
    } 
    else 
    {
#if _DEBUG // for scope: pulse pin as overflow indictator
      DebugPulse(_DEBUG_PIN1, 1);
#endif
      _buffer_overflow = true;
    }
  }

#if GCC_VERSION < 40302
// Work-around for avr-gcc 4.3.0 OSX version bug
// Restore the registers that the compiler misses
  asm volatile(
    "pop r27 \n\t"
    "pop r26 \n\t"
    "pop r23 \n\t"
    "pop r22 \n\t"
    "pop r21 \n\t"
    "pop r20 \n\t"
    "pop r19 \n\t"
    "pop r18 \n\t"
    ::);
#endif
}

void SoftSerial::tx_pin_write(uint8_t pin_state)
{
  if (pin_state == LOW)
    *_transmitPortRegister &= ~_transmitBitMask;
  else
    *_transmitPortRegister |= _transmitBitMask;
}

uint8_t SoftSerial::rx_pin_read()
{
  return *_receivePortRegister & _receiveBitMask;
}

//
// Interrupt handling
//

/* static */
inline void SoftSerial::handle_interrupt()
{
  if (active_object)
  {
    active_object->recv();
  }
}
#if 0 /* Do not use Interrupt Vector here: Interrupt Vector is shared through TinyPinChange library */
#if defined(PCINT0_vect)
ISR(PCINT0_vect)
{
  SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT1_vect)
ISR(PCINT1_vect)
{
  SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT2_vect)
ISR(PCINT2_vect)
{
  SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT3_vect)
ISR(PCINT3_vect)
{
  SoftSerial::handle_interrupt();
}
#endif
#endif
//
// Constructor
//
SoftSerial::SoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic /* = false */) : 
  _rx_delay_centering(0),
  _rx_delay_intrabit(0),
  _rx_delay_stopbit(0),
  _tx_delay(0),
  _buffer_overflow(false),
  _inverse_logic(inverse_logic)
{
  setRX(receivePin);
  setTX(transmitPin);
  TinyPinChange_RegisterIsr(receivePin, SoftSerial::handle_interrupt);
}

//
// Destructor
//
SoftSerial::~SoftSerial()
{
  end();
}

void SoftSerial::setTX(uint8_t tx)
{
  _transmitBitMask = digitalPinToBitMask(tx);
  if(_transmitBitMask!=_receiveBitMask)
  {
    pinMode(tx, OUTPUT);
    digitalWrite(tx, HIGH);
  }
//  _transmitBitMask = digitalPinToBitMask(tx);
  uint8_t port = digitalPinToPort(tx);
  _transmitPortRegister = portOutputRegister(port);
}

void SoftSerial::setRX(uint8_t rx)
{
  pinMode(rx, INPUT);
  if (!_inverse_logic)
    digitalWrite(rx, HIGH);  // pullup for normal logic!
  _receivePin = rx;
  _receiveBitMask = digitalPinToBitMask(rx);
  uint8_t port = digitalPinToPort(rx);
  _receivePortRegister = portInputRegister(port);
}

//
// Public methods
//

void SoftSerial::begin(long speed)
{
  _rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;

  for (unsigned i=0; i<sizeof(table)/sizeof(table[0]); ++i)
  {
    long baud = pgm_read_dword(&table[i].baud);
    if (baud == speed)
    {
      _rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
      _rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
      _rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
      _tx_delay = pgm_read_word(&table[i].tx_delay);
      break;
    }
  }

  // Set up RX interrupts, but only if we have a valid RX baud rate
  if (_rx_delay_stopbit)
  {
    if (digitalPinToPCICR(_receivePin))
    {
      *digitalPinToPCICR(_receivePin) |= _BV(digitalPinToPCICRbit(_receivePin));
      *digitalPinToPCMSK(_receivePin) |= _BV(digitalPinToPCMSKbit(_receivePin));
    }
    tunedDelay(_tx_delay); // if we were low this establishes the end
  }

#if _DEBUG
  pinMode(_DEBUG_PIN1, OUTPUT);
  pinMode(_DEBUG_PIN2, OUTPUT);
#endif

  listen();
}

void SoftSerial::end()
{
  if (digitalPinToPCMSK(_receivePin))
    *digitalPinToPCMSK(_receivePin) &= ~_BV(digitalPinToPCMSKbit(_receivePin));
}


// Read data from buffer
int SoftSerial::read()
{
  if (!isListening())
    return -1;

  // Empty buffer?
  if (_receive_buffer_head == _receive_buffer_tail)
    return -1;

  // Read from "head"
  uint8_t d = _receive_buffer[_receive_buffer_head]; // grab next byte
  _receive_buffer_head = (_receive_buffer_head + 1) % _SS_MAX_RX_BUFF;
  return d;
}

int SoftSerial::available()
{
  if (!isListening())
    return 0;

  return (_receive_buffer_tail + _SS_MAX_RX_BUFF - _receive_buffer_head) % _SS_MAX_RX_BUFF;
}

size_t SoftSerial::write(uint8_t b)
{
  if (_tx_delay == 0) {
    setWriteError();
    return 0;
  }

  uint8_t oldSREG = SREG;
  cli();  // turn off interrupts for a clean txmit

  // Write the start bit
  tx_pin_write(_inverse_logic ? HIGH : LOW);
  tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);

  // Write each of the 8 bits
  if (_inverse_logic)
  {
    for (byte mask = 0x01; mask; mask <<= 1)
    {
      if (b & mask) // choose bit
        tx_pin_write(LOW); // send 1
      else
        tx_pin_write(HIGH); // send 0
    
      tunedDelay(_tx_delay);
    }

    tx_pin_write(LOW); // restore pin to natural state
  }
  else
  {
    for (byte mask = 0x01; mask; mask <<= 1)
    {
      if (b & mask) // choose bit
        tx_pin_write(HIGH); // send 1
      else
        tx_pin_write(LOW); // send 0
    
      tunedDelay(_tx_delay);
    }

    tx_pin_write(HIGH); // restore pin to natural state
  }

  SREG = oldSREG; // turn interrupts back on
  tunedDelay(_tx_delay);
  
  return 1;
}

void SoftSerial::flush()
{
  if (!isListening())
    return;

  uint8_t oldSREG = SREG;
  cli();
  _receive_buffer_head = _receive_buffer_tail = 0;
  SREG = oldSREG;
}

int SoftSerial::peek()
{
  if (!isListening())
    return -1;

  // Empty buffer?
  if (_receive_buffer_head == _receive_buffer_tail)
    return -1;

  // Read from "head"
  return _receive_buffer[_receive_buffer_head];
}

/* RC Navy: hack to use SofSerial as single wire bidirectional serial port */
void SoftSerial::txMode()
{
  /* Disable Pin Change Interrupt capabilities for this pin */
  TinyPinChange_DisablePin(_receivePin);
  /* Switch Pin to Output */
  pinMode(_receivePin, OUTPUT);
  digitalWrite(_receivePin, _inverse_logic?LOW:HIGH);
}

void SoftSerial::rxMode()
{
  /* Enable Pin Change Interrupt capabilities for this pin */
  TinyPinChange_EnablePin(_receivePin);
  /* Switch Pin to Input */
  pinMode(_receivePin, INPUT);
  if (!_inverse_logic)
    digitalWrite(_receivePin, HIGH);  // pullup for normal logic!
}