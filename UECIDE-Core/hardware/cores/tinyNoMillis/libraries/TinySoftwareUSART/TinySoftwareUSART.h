
#ifndef TinySoftwareUSART_h
#define TinySoftwareUSART_h
#include <inttypes.h>
#include "Arduino.h"
#include "Stream.h"

#ifndef ACSR
#ifdef ACRSA
#define ACSR ACSRA
#endif
#endif

//SERIAL_BUFFER_SIZE must be power of 2, and <=256
#if (RAMEND < 250)
  #define SERIAL_BUFFER_SIZE 8
#elif (RAMEND < 500)
  #define SERIAL_BUFFER_SIZE 16
#elif (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 32
#else
  #define SERIAL_BUFFER_SIZE 64
#endif
typedef struct
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned char head;
  volatile unsigned char tail;
} USARTRingBuffer;

class TinySoftwareUSART : public Stream
{
  private:
    USARTRingBuffer *_rx_buffer;
    USARTRingBuffer *_tx_buffer;
  public:
    TinySoftwareUSART();
    void begin();
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

#endif
