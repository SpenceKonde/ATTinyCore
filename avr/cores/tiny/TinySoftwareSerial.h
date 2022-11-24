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
  #ifndef ANALOG_COMP_DDR
    #error Please define ANALOG_COMP_DDR in the pins_arduino.h file!
  #endif
  #define SOFTSERIAL_DDR ANALOG_COMP_DDR

  #ifndef ANALOG_COMP_PORT
    #error Please define ANALOG_COMP_PORT in the pins_arduino.h file!
  #endif
  #define SOFTSERIAL_PORT ANALOG_COMP_PORT

  #ifndef ANALOG_COMP_PIN
    #error Please define ANALOG_COMP_PIN in the pins_arduino.h file!
  #endif
  #define SOFTSERIAL_PIN ANALOG_COMP_PIN

  #ifndef ANALOG_COMP_AIN0_BIT
    #error Please define ANALOG_COMP_AIN0_BIT in the pins_arduino.h file!
  #endif

  #ifndef ANALOG_COMP_AIN1_BIT
    #error Please define ANALOG_COMP_AIN1_BIT in the pins_arduino.h file!
  #endif

  #if ((defined(ACSRB) && !defined(ANALOG_COMP_AIN2_BIT)) && (defined(SOFTSERIAL_RXAIN0) || defined(SOFTSERIAL_RXAIN2)))
    #ifndef ANALOG_COMP_AIN2_BIT
      #error Please define ANALOG_COMP_AIN2_BIT in the pins_arduino.h file!
    #endif
  #endif
  /* Spence: It would be pretty easy to adapt this to use INT0 instead if desired, controlled by the boards.txt menu */
  #if !defined(ANALOG_COMP_vect) && defined(ANA_COMP_vect)
  //rename the vector so we can use it.
    #define SOFTSERIAL_vect ANA_COMP_vect
  #elif defined (ANALOG_COMP_vect)
    #define SOFTSERIAL_vect ANALOG_COMP_vect
  #else
    #error "Tiny Software Serial cannot find the Analog comparator interrupt vector!"
  #endif
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
    #if defined(SOFTSERIAL_RXAIN0)
      #define SOFTSERIAL_RXBIT ANALOG_COMP_AIN0_BIT
      #define SOFTSERIAL_TXBIT ANALOG_COMP_AIN2_BIT
    #elif defined(SOFTSERIAL_RXAIN2)
      #define SOFTSERIAL_RXBIT ANALOG_COMP_AIN2_BIT
      #define SOFTSERIAL_TXBIT ANALOG_COMP_AIN0_BIT
    #else
      #define SOFTSERIAL_RXBIT ANALOG_COMP_AIN1_BIT
      #define SOFTSERIAL_TXBIT ANALOG_COMP_AIN0_BIT
    #endif
    struct soft_ring_buffer
    {
      volatile unsigned char buffer[SERIAL_BUFFER_SIZE];
      volatile uint8_t head; // Making these uint8_t's saves FIFTY BYTES of flash! ISR code-size amplification in action...
      volatile uint8_t tail;
    };
  #else
    #define SOFTSERIAL_TXBIT ANALOG_COMP_AIN0_BIT

  #endif
  extern "C"{
    void uartDelay() __attribute__ ((naked, used)); //used attribute needed to prevent LTO from throwing it out.
    #ifndef SOFT_TX_ONLY
      // manually inlined because the compiler refused to do it.
      //uint8_t getch();
      //void store_char(unsigned char c, soft_ring_buffer *buffer);
    #endif
  }
  class TinySoftwareSerial : public Stream
  {
    public: //should be private but needed by extern "C" {} functions.
      uint8_t _txmask;
    #if !defined(SOFT_TX_ONLY)
      soft_ring_buffer *_rx_buffer;
    #endif
    uint8_t _delayCount;
    public:
      #if !defined(SOFT_TX_ONLY)
        TinySoftwareSerial(soft_ring_buffer *rx_buffer);
      #else
        TinySoftwareSerial();
      #endif
      void begin(long);
      void setTxBit(uint8_t);
      bool stopListening();
      bool listen();
      void end();          // Basic printHex() forms for 8, 16, and 32-bit values
      void                printHex(const     uint8_t              b);
      void                printHex(const    uint16_t  w, bool s = 0);
      void                printHex(const    uint32_t  l, bool s = 0);
      // printHex(signed) and printHexln() - trivial implementation;
      void                printHex(const      int8_t  b)              {printHex((uint8_t )   b);           }
      void                printHex(const        char  b)              {printHex((uint8_t )   b);           }
      void              printHexln(const      int8_t  b)              {printHex((uint8_t )   b); println();}
      void              printHexln(const        char  b)              {printHex((uint8_t )   b); println();}
      void              printHexln(const     uint8_t  b)              {printHex(             b); println();}
      void              printHexln(const    uint16_t  w, bool s = 0)  {printHex(          w, s); println();}
      void              printHexln(const    uint32_t  l, bool s = 0)  {printHex(          l, s); println();}
      void              printHexln(const     int16_t  w, bool s = 0)  {printHex((uint16_t)w, s); println();}
      void              printHexln(const     int32_t  l, bool s = 0)  {printHex((uint16_t)l, s); println();}
      // The pointer-versions for mass printing uint8_t and uint16_t arrays.
      uint8_t *           printHex(          uint8_t* p, uint8_t len, char sep = 0            );
      uint16_t *          printHex(         uint16_t* p, uint8_t len, char sep = 0, bool s = 0);
      virtual int available(void);
      virtual int peek(void);
      virtual int read(void);
      virtual void flush(void);
      virtual size_t write(uint8_t);
      using Print::write; // pull in write(str) and write(buf, size) from Print
      operator bool();
    private:
      uint8_t _begun = 0;
  };

  extern TinySoftwareSerial Serial;

  //extern void putch(uint8_t);
  #endif
#endif
