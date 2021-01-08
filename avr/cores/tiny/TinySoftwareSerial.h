#if USE_SOFTWARE_SERIAL
#ifndef TinySoftwareSerial_h
#define TinySoftwareSerial_h
#include <inttypes.h>
#include "Stream.h"

#if !defined(ACSR) && defined(ACSRA)
#define ACSR ACSRA
#endif

#if (RAMEND < 250)
  #define SERIAL_BUFFER_SIZE 8
#elif (RAMEND < 500)
  #define SERIAL_BUFFER_SIZE 16
#elif (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 32
#else
  #define SERIAL_BUFFER_SIZE 128
#endif
struct soft_ring_buffer
{
  volatile unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile int head;
  volatile int tail;
};

extern "C"{
  void uartDelay() __attribute__ ((naked,used)); //used attribute needed to prevent LTO from throwing it out.
  uint8_t getch();
  void store_char(unsigned char c, soft_ring_buffer *buffer);
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

#if (!defined(UBRRH) && !defined(UBRR0H)) || USE_SOFTWARE_SERIAL
  extern TinySoftwareSerial Serial;
#endif

//extern void putch(uint8_t);
#endif
#endif
