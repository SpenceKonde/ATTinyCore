#include "Arduino.h"
#if (!USE_SOFTWARE_SERIAL && !DISABLE_UART && !DISABLE_UART0)
  #include "HardwareSerial.h"
  #if defined(UBRRH) || defined(UBRR0H) || defined(LINBRRH)
    ring_buffer rx_buffer  =  { { 0 }, 0, 0 };
    ring_buffer tx_buffer  =  { { 0 }, 0, 0 };

  #if !defined(UART0_UDRE_vect) && !defined(UART_UDRE_vect) && !defined(USART0_UDRE_vect) && !defined(USART_UDRE_vect) && !defined(LIN_TC_vect)
      #error "Don't know what the Data Register Empty vector is called for the first UART"
    #else
    #if defined(UART0_UDRE_vect)
      ISR(UART0_UDRE_vect)
    #elif defined(UART_UDRE_vect)
      ISR(UART_UDRE_vect)
    #elif defined(USART0_UDRE_vect)
      ISR(USART0_UDRE_vect)
    #elif defined(USART_UDRE_vect)
      ISR(USART_UDRE_vect)
    #endif
    #if !defined(LIN_TC_vect)
      {
        if (tx_buffer.head == tx_buffer.tail) {
        // Buffer empty, so disable interrupts
        #if defined(UCSR0B)
          UCSR0B &= ~(1 << UDRIE0);
        #else
          UCSRB &= ~(1 << UDRIE);
        #endif
        } else {
          // There is more data in the output buffer. Send the next byte
          unsigned char c = tx_buffer.buffer[tx_buffer.tail];
          tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;

          #if defined(UDR)
            UDR = c;
          #elif defined(UDR0)
            UDR0 = c;
          #else
            #error UDR not defined
          #endif
        }
      }
    #endif
  #endif

  #if defined(USART_RX_vect)
    ISR(USART_RX_vect)
    {
    #if defined(UDR)
      unsigned char c  =  UDR;
    #elif defined(UDR0)
      unsigned char c  =  UDR0;  //  atmega8535
    #else
      #error UDR not defined
    #endif
      store_char(c, &rx_buffer);
    }
  #elif defined(USART0_RX_vect)
    ISR(USART0_RX_vect)
    {
    #if defined(UDR)
      unsigned char c  =  UDR;
    #elif defined(UDR0)
      unsigned char c  =  UDR0;  //  atmega8535
    #else
      #error UDR not defined
    #endif
      store_char(c, &rx_buffer);
    }
  #elif defined(UART_RX_vect)
    ISR(UART_RX_vect)
    {
    #if defined(UDR)
      unsigned char c  =  UDR;
    #elif defined(UDR0)
      unsigned char c  =  UDR0;  //  atmega8535
    #else
      #error UDR not defined
    #endif
      store_char(c, &rx_buffer);
    }
  #elif defined(UART0_RX_vect)
    ISR(UART0_RX_vect)
    {
    #if defined(UDR)
      unsigned char c  =  UDR;
    #elif defined(UDR0)
      unsigned char c  =  UDR0;  //  atmega8535
    #else
      #error UDR not defined
    #endif
      store_char(c, &rx_buffer);
    }
  #elif defined(LIN_TC_vect)
    // this is for attinyX7
    ISR(LIN_TC_vect)
    {
      if(LINSIR & _BV(LRXOK)) {
          unsigned char c  =  LINDAT;
          store_char(c, &rx_buffer);
      }
      if(LINSIR & _BV(LTXOK)){
        //PINA |= _BV(PINA5); //debug
        if (tx_buffer.head == tx_buffer.tail) {
        // Buffer empty, so disable interrupts
          LINENIR = LENRXOK; //unset LENTXOK
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
  #endif
  #if defined(UBRRH) && defined(UBRRL)
    HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRRH, &UBRRL, &UCSRA, &UCSRB, &UDR, (1 << RXEN), (1 << TXEN), (1 << RXCIE), (1 << UDRE), (1 << U2X));
  #elif defined(UBRR0H) && defined(UBRR0L)
    HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UDR0, (1 << RXEN0), (1 << TXEN0), (1 << RXCIE0), (1 << UDRE0), (1 << U2X0));
  #elif defined(LINBRRH)
    HardwareSerial Serial(&rx_buffer, &tx_buffer);
  #endif
#endif
