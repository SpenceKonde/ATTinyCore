#include "Arduino.h"
#if (!DISABLE_UART1 && !DISABLE_UART)
  #include "HardwareSerial.h"
  #if defined(UBRR1H)
    ring_buffer rx_buffer1  =  { { 0 }, 0, 0 };
    ring_buffer tx_buffer1  =  { { 0 }, 0, 0 };
    HardwareSerial Serial1(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UDR1);
  #endif
  #if defined(USART1_RX_vect)
    ISR(USART1_RX_vect)
    {
      unsigned char c = UDR1;
      store_char(c, &rx_buffer1);
    }
  #elif defined(USART1_RXC_vect)
    ISR(USART1_RXC_vect )
    {
      unsigned char c = UDR1;
      store_char(c, &rx_buffer1);
    }
  #else
    //no UART1
  #endif
  #ifdef USART1_UDRE_vect
    ISR(USART1_UDRE_vect)
    {
      if (tx_buffer1.head == tx_buffer1.tail) {
      // Buffer empty, so disable interrupts
        UCSR1B &= ~(1 << UDRIE1);
      }
      else {
        // There is more data in the output buffer. Send the next byte
        unsigned char c = tx_buffer1.buffer[tx_buffer1.tail];
        tx_buffer1.tail = (tx_buffer1.tail + 1) % SERIAL_BUFFER_SIZE;

        UDR1 = c;
      }
    }
  #endif
#endif
