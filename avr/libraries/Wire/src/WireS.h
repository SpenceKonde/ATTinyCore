/*==============================================================================================================*

    @file     WireS.h
    @license  MIT (c) 2016 Nadav Matalon

    ATtiny841 I2C Slave Library

    Ver. 1.0.0 - First release (1.12.16)

 *===============================================================================================================*
    INTRODUCTION
 *===============================================================================================================*

    This library enables the newer members of the ATtiny family (ATtiny441/841, ATtiny1634 & ATtiny828) to become
    Slaves on an I2C Bus. The library is based on a hardware implementation of I2C which is faster than the
    software one, but requires using a specific set of pins for SDA (PA6 on ATtiny841) & SCL (PA4 on ATtiny841).

 *===============================================================================================================*
    LICENSE
 *===============================================================================================================*

    The MIT License (MIT)
    Copyright (c) 2016 Nadav Matalon

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    documentation files (the "Software"), to deal in the Software without restriction, including without
    limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
    LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 *==============================================================================================================*/

#if (!defined(WIRE_S_H) && (/*defined(__AVR_ATtiny1634__) ||*/defined(__AVR_ATtiny841__) || defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny828__) || defined(__AVR_ATtiny40__) || defined(__AVR_ATtiny20__))) && !defined(WIRE_MASTER_ONLY)
#define WIRE_S_H

#include <inttypes.h>
#include "Arduino.h"

#define I2C_BUFFER_LENGTH 32

#if defined(I2C_INTR_FLAG_PIN)
    #define I2C_INTR_FLAG_INIT do {             \
        pinMode(I2C_INTR_FLAG_PIN, OUTPUT);     \
        digitalWrite(I2C_INTR_FLAG_PIN, LOW);   \
    } while(0)
    #define I2C_INTR_FLAG_ON   do {digitalWrite(I2C_INTR_FLAG_PIN, HIGH);} while(0)
    #define I2C_INTR_FLAG_OFF  do {digitalWrite(I2C_INTR_FLAG_PIN, LOW);} while(0)
#else
    #define I2C_INTR_FLAG_INIT do{}while(0)
    #define I2C_INTR_FLAG_ON   do{}while(0)
    #define I2C_INTR_FLAG_OFF  do{}while(0)
#endif

struct i2cStruct {
    uint8_t  Buffer[I2C_BUFFER_LENGTH];      // Tx/Rx Buffer                      (ISR)
    volatile size_t   rxBufferIndex;         // Rx Index                          (User&ISR)
    volatile size_t   rxBufferLength;        // Rx Length                         (ISR)
    volatile size_t   txBufferIndex;         // Tx Index                          (User&ISR)
    volatile size_t   txBufferLength;        // Tx Length                         (User&ISR)
    volatile char     startCount;            // repeated START count              (User&ISR)
    volatile uint16_t Addr;                  // Tx/Rx address                     (User&ISR)
    boolean (*user_onAddrReceive)(uint16_t, uint8_t);  // Slave Addr Callback Function      (User)
    void (*user_onReceive)(size_t);          // Slave Rx Callback Function        (User)
    void (*user_onRequest)(void);            // Slave Tx Callback Function        (User)
    void (*user_onStop)(void);               // Stop Callback Function            (User)
};

extern "C" void i2c_isr_handler(struct i2cStruct* i2c);

class i2c_tinyS : public Stream {
    private:
        static struct i2cStruct i2cData;
        friend void i2c_isr_handler(void);
    public:
        struct i2cStruct* i2c;
        i2c_tinyS();
        ~i2c_tinyS();
        static void begin_(struct i2cStruct* i2c, uint8_t address, uint8_t mask);
        inline void begin(int address) {
            begin_(i2c, (uint8_t)address, 0);
        }
        inline void begin(uint8_t address, uint8_t mask) {
            begin_(i2c, address, mask);
        }
        size_t write(uint8_t data);
        inline size_t write(unsigned long n) { return write((uint8_t)n); }
        inline size_t write(long n)          { return write((uint8_t)n); }
        inline size_t write(unsigned int n)  { return write((uint8_t)n); }
        inline size_t write(int n)           { return write((uint8_t)n); }
        size_t write(const uint8_t* data, size_t quantity);
        inline size_t write(const char* str) { write((const uint8_t*)str, strlen(str)); }
        inline int available(void) { return i2c->rxBufferLength - i2c->rxBufferIndex; }
        static int read_(struct i2cStruct* i2c);
        inline int read(void) { return read_(i2c); }
        static int peek_(struct i2cStruct* i2c);
        inline int peek(void) { return peek_(i2c); }
        static uint8_t readByte_(struct i2cStruct* i2c);
        inline uint8_t readByte(void) { return readByte_(i2c); }
        static uint8_t peekByte_(struct i2cStruct* i2c);
        inline uint8_t peekByte(void) { return peekByte_(i2c); }
        inline void flush(void) {}
        inline uint16_t getRxAddr(void) { return i2c->Addr; }
        inline size_t getTransmitBytes(void) { return i2c->txBufferIndex; }
        inline void onAddrReceive(boolean (*function)(uint16_t, uint8_t)) { i2c->user_onAddrReceive = function; }
        inline void onReceive(void (*function)(size_t)) { i2c->user_onReceive = function; }
        inline void onRequest(void (*function)(void)) { i2c->user_onRequest = function; }
        inline void onStop(void (*function)(void)) { i2c->user_onStop = function; }
};

extern i2c_tinyS TinyWireS;

#endif
