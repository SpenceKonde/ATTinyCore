/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "Arduino.h"
#include "wiring_private.h"

// this next line disables the entire HardwareSerial.cpp,
// this is so I can support Attiny series and any other chip without a uart
#if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(LINBRRH)) && !USE_SOFTWARE_SERIAL

#include "HardwareSerial.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
#if (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 16
#else
  #define SERIAL_BUFFER_SIZE 64
#endif

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  byte head;
  byte tail;
};

#if defined(UBRRH) || defined(UBRR0H) || defined(LINBRRH)
  ring_buffer rx_buffer  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR1H)
  ring_buffer rx_buffer1  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer1  =  { { 0 }, 0, 0 };
#endif

inline void store_char(unsigned char c, ring_buffer *buffer)
{
  byte i = (buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

#if defined(USART_RX_vect)
  SIGNAL(USART_RX_vect)
  {
  #if defined(UDR0)
    unsigned char c  =  UDR0;
  #elif defined(UDR)
    unsigned char c  =  UDR;  //  atmega8535
  #else
    #error UDR not defined
  #endif
    store_char(c, &rx_buffer);
  }
#elif defined(SIG_USART0_RECV) && defined(UDR0)
  SIGNAL(SIG_USART0_RECV)
  {
    unsigned char c  =  UDR0;
    store_char(c, &rx_buffer);
  }
#elif defined(SIG_UART0_RECV) && defined(UDR0)
  SIGNAL(SIG_UART0_RECV)
  {
    unsigned char c  =  UDR0;
    store_char(c, &rx_buffer);
  }
//#elif defined(SIG_USART_RECV)
#elif defined(USART0_RX_vect)
  // fixed by Mark Sproul this is on the 644/644p
  //SIGNAL(SIG_USART_RECV)
  SIGNAL(USART0_RX_vect)
  {
  #if defined(UDR0)
    unsigned char c  =  UDR0;
  #elif defined(UDR)
    unsigned char c  =  UDR;  //  atmega8, atmega32
  #else
    #error UDR not defined
  #endif
    store_char(c, &rx_buffer);
  }
#elif defined(SIG_UART_RECV)
  // this is for atmega8
  SIGNAL(SIG_UART_RECV)
  {
  #if defined(UDR0)
    unsigned char c  =  UDR0;  //  atmega645
  #elif defined(UDR)
    unsigned char c  =  UDR;  //  atmega8
  #endif
    store_char(c, &rx_buffer);
  }
#elif defined(LIN_TC_vect)
  // this is for attinyX7
  SIGNAL(LIN_TC_vect)
  {
	if(LINSIR & _BV(LRXOK)) {
      unsigned char c  =  LINDAT;
      store_char(c, &rx_buffer);
	}
    if(LINSIR & _BV(LTXOK)){
      PINA |= _BV(PINA5);
      if (tx_buffer.head == tx_buffer.tail) {
      // Buffer empty, so disable interrupts
        cbi(LINENIR,LENTXOK);
      } else {
        // There is more data in the output buffer. Send the next byte
        unsigned char c = tx_buffer.buffer[tx_buffer.tail];
        tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
        
        LINDAT = c;
      }
	}
  }
#else
  #error No interrupt handler for usart 0
#endif

//#if defined(SIG_USART1_RECV)
#if defined(USART1_RX_vect)
  //SIGNAL(SIG_USART1_RECV)
  SIGNAL(USART1_RX_vect)
  {
    unsigned char c = UDR1;
    store_char(c, &rx_buffer1);
  }
#elif defined(SIG_USART1_RECV)
  #error SIG_USART1_RECV
#endif

#if !defined(UART0_UDRE_vect) && !defined(UART_UDRE_vect) && !defined(USART0_UDRE_vect) && !defined(USART_UDRE_vect) && !defined(LIN_TC_vect)
  #error "Don't know what the Data Register Empty vector is called for the first UART"
#elif ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#endif
{
  if (tx_buffer.head == tx_buffer.tail) {
	// Buffer empty, so disable interrupts
#if defined(UCSR0B)
    cbi(UCSR0B, UDRIE0);
#else
    cbi(UCSRB, UDRIE);
#endif
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
	
  #if defined(UDR0)
    UDR0 = c;
  #elif defined(UDR)
    UDR = c;
  #else
    #error UDR not defined
  #endif
  }
}
#endif

#ifdef USART1_UDRE_vect
ISR(USART1_UDRE_vect)
{
  if (tx_buffer1.head == tx_buffer1.tail) {
	// Buffer empty, so disable interrupts
    cbi(UCSR1B, UDRIE1);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer1.buffer[tx_buffer1.tail];
    tx_buffer1.tail = (tx_buffer1.tail + 1) % SERIAL_BUFFER_SIZE;
	
    UDR1 = c;
  }
}
#endif


// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer
#if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
    ,
    volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
    volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
    volatile uint8_t *udr,
    uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x
  )
{
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
  _ubrrh = ubrrh;
  _ubrrl = ubrrl;
  _ucsra = ucsra;
  _ucsrb = ucsrb;
  _udr = udr;
  _rxen = rxen;
  _txen = txen;
  _rxcie = rxcie;
  _udrie = udrie;
  _u2x = u2x;
}
#else
  )
{
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
}
#endif

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long baud)
{
#if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
  uint16_t baud_setting;
  bool use_u2x = true;

#if F_CPU == 16000000UL
  // hardcoded exception for compatibility with the bootloader shipped
  // with the Duemilanove and previous boards and the firmware on the 8U2
  // on the Uno and Mega 2560.
  if (baud == 57600) {
    use_u2x = false;
  }
#endif
  
try_again:
  
  if (use_u2x) {
    *_ucsra = 1 << _u2x;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  
  if ((baud_setting > 4095) && use_u2x)
  {
    use_u2x = false;
    goto try_again;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  sbi(*_ucsrb, _rxen);
  sbi(*_ucsrb, _txen);
  sbi(*_ucsrb, _rxcie);
  cbi(*_ucsrb, _udrie);
#else
  LINCR = (1 << LSWRES); 
  LINBRR = (((F_CPU * 10L / 16L / baud) + 5L) / 10L) - 1; 
  LINBTR = (1 << LDISR) | (16 << LBT0); 
  LINCR = _BV(LENA) | _BV(LCMD2) | _BV(LCMD1) | _BV(LCMD0); 
  sbi(LINENIR,LENRXOK);
#endif
}

void HardwareSerial::end()
{
  while (_tx_buffer->head != _tx_buffer->tail)
    ;
#if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H))
  cbi(*_ucsrb, _rxen);
  cbi(*_ucsrb, _txen);
  cbi(*_ucsrb, _rxcie); 
  cbi(*_ucsrb, _udrie); 
#else
  cbi(LINENIR,LENTXOK);
  cbi(LINENIR,LENRXOK);
  cbi(LINCR,LENA);
  cbi(LINCR,LCMD0);
  cbi(LINCR,LCMD1);
  cbi(LINCR,LCMD2);
#endif
  _rx_buffer->head = _rx_buffer->tail;
}

int HardwareSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

int HardwareSerial::peek(void)
{
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

int HardwareSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

void HardwareSerial::flush()
{
  while (_tx_buffer->head != _tx_buffer->tail)
    ;
}

size_t HardwareSerial::write(uint8_t c)
{
  byte i = (_tx_buffer->head + 1) % SERIAL_BUFFER_SIZE;
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  // ???: return 0 here instead?
  while (i == _tx_buffer->tail)
    ;
	
  _tx_buffer->buffer[_tx_buffer->head] = c;
  _tx_buffer->head = i;
  #if ( defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) )
  sbi(*_ucsrb, _udrie);
  #else
  if(!(LINENIR & _BV(LENTXOK))){
    //The buffer was previously empty, so enable TX Complete interrupt and load first byte.
    sbi(LINENIR,LENTXOK);
    unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    LINDAT = c;
  }
  #endif
  return 1;
}

HardwareSerial::operator bool() {
	return true;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(UBRRH) && defined(UBRRL)
  HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRRH, &UBRRL, &UCSRA, &UCSRB, &UDR, RXEN, TXEN, RXCIE, UDRE, U2X);
#elif defined(UBRR0H) && defined(UBRR0L)
  HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UDR0, RXEN0, TXEN0, RXCIE0, UDRE0, U2X0);
#elif defined(LINBRRH)
  HardwareSerial Serial(&rx_buffer, &tx_buffer);
#endif

#if defined(UBRR1H)
  HardwareSerial Serial1(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UDR1, RXEN1, TXEN1, RXCIE1, UDRE1, U2X1);
#endif

#elif !USE_SOFTWARE_SERIAL
#warning There is no Hardware UART, and Sofware Serial is not enabled. There will be no serial port.
#endif // whole file
