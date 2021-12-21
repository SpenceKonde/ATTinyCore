#ifndef TinySoftwareSerial_h
#define TinySoftwareSerial_h
#include <inttypes.h>
#include "Stream.h"

/* Where should this work?
 * ATtiny x5:   Yes - RX on PB1, TX defaults to PB0, can be PB0~4.
 * ATtiny x4:   Yes - RX on PA2, TX defaults to PA1, can be PA0~7.
 * ATtiny 26:   Yes - RX on PA7, TX defaults to PA6, can be PA0~7. Flash will be mighty tight.
 * ATtiny x61:  Yes - RX on PA7, TX defaults to PA6, can be PA0~7.
 * ATtiny x8:   Yes - RX on PD7, TX defaults to PD6, can be PD0~7.
 * ATtiny 43:   Yes - RX on PA-5, TX defaults to PA4, can be PA0~4 or PA6.
 * ATtiny x313: Yes, but it's got a hardware serial port so not enabled. RX would be on PB1.
 * ATtiny x7:   No - but it's got a hardware serial port.
 * ATtiny x41:  No - but it's got two hardware serial ports!
 * ATtiny 828:  No - but it's got a hardware serial port.
 * ATtiny 1634: No - but it's got two hardware serial ports!
 */

#if USE_SOFTWARE_SERIAL

  #if !defined(ACSR) && defined(ACSRA)
    #define ACSR ACSRA
  #endif

  #ifndef SOFT_TX_ONLY
    #if (RAMEND < 250)
      #define SERIAL_BUFFER_SIZE 8
    #elif (RAMEND < 500)
      #define SERIAL_BUFFER_SIZE 16
    #elif (RAMEND < 1000)
      #define SERIAL_BUFFER_SIZE 32
    #else
      /* never true for supported parts */
      #define SERIAL_BUFFER_SIZE 128
    #endif
    struct soft_ring_buffer
    {
      volatile unsigned char buffer[SERIAL_BUFFER_SIZE];
      volatile int head;
      volatile int tail;
    };
  #endif
  extern "C"{
    void uartDelay() __attribute__ ((naked,used)); //used attribute needed to prevent LTO from throwing it out.
    #ifndef SOFT_TX_ONLY
      uint8_t getch();
      void store_char(unsigned char c, soft_ring_buffer *buffer);
    #endif
  }
  class TinySoftwareSerial : public Stream
  {
    public: //should be private but needed by extern "C" {} functions.
    uint8_t _rxmask;
    uint8_t _txmask;
    uint8_t _txunmask;
    soft_ring_buffer *_rx_buffer;
    uint8_t _delayCount;
    public:
      TinySoftwareSerial(soft_ring_buffer *rx_buffer, uint8_t txBit, uint8_t rxBit);
      void begin(long);
      void setTxBit(uint8_t);
      void end();
      virtual int available(void);
      virtual int peek(void);
      virtual int read(void);
      virtual void flush(void);
      virtual size_t write(uint8_t);
      using Print::write; // pull in write(str) and write(buf, size) from Print
      operator bool();
  };

  extern TinySoftwareSerial Serial;

  //extern void putch(uint8_t);
  #endif
#endif
