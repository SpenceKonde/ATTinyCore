/*==============================================================================================================*

    @file     WireS.h
    @license  MIT (c) 2016 Nadav Matalon

    ATtiny841 I2C Slave Library

    Ver. 1.0.0 - First release (1.12.16)

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

#if (/*defined(__AVR_ATtiny1634__) ||*/ defined(__AVR_ATtiny841__) || defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny828__) || defined(__AVR_ATtiny40__) || defined(__AVR_ATtiny20__)) && !defined(WIRE_MASTER_ONLY)

#include <avr/io.h>
#include "WireS.h"

#define TWI_HIGH_NOISE_MODE 0

#ifndef TWAE
#define TWAE 0
#endif

#define SET_TENBIT   do { i2c->Addr |= 0x8000; } while(0)
#define CLEAR_TENBIT do { i2c->Addr &= 0x7FFF; } while(0)
#define IS_TENBIT (i2c->Addr & 0x8000)

struct i2cStruct i2c_tinyS::i2cData;

i2c_tinyS::i2c_tinyS() {
    i2c = &i2cData;
}

i2c_tinyS::~i2c_tinyS() {}

void i2c_tinyS::begin_(struct i2cStruct* i2c, uint8_t address, uint8_t mask) {
    I2C_INTR_FLAG_INIT;
    TWSA = (address << 1);
    TWSAM = mask;
    i2c->startCount = -1;
    TWSCRA = (_BV(TWSHE) | _BV(TWDIE) | _BV(TWASIE) | _BV(TWEN) | _BV(TWSIE));
}

size_t i2c_tinyS::write(uint8_t data) {
    if(i2c->txBufferLength < I2C_BUFFER_LENGTH) {
        i2c->Buffer[i2c->txBufferLength++] = data;
        return 1;
    }
    return 0;
}

size_t i2c_tinyS::write(const uint8_t* data, size_t quantity) {
    if(i2c->txBufferLength < I2C_BUFFER_LENGTH) {
        size_t avail = I2C_BUFFER_LENGTH - i2c->txBufferLength;
        uint8_t* dest = i2c->Buffer + i2c->txBufferLength;
        if(quantity > avail) quantity = avail;
        for (size_t count=quantity; count; count--) *dest++ = *data++;
        i2c->txBufferLength += quantity;
    }
}

int i2c_tinyS::read_(struct i2cStruct* i2c) {
    if(i2c->rxBufferIndex >= i2c->rxBufferLength) return -1;
    return i2c->Buffer[i2c->rxBufferIndex++];
}

int i2c_tinyS::peek_(struct i2cStruct* i2c) {
    if(i2c->rxBufferIndex >= i2c->rxBufferLength) return -1;
    return i2c->Buffer[i2c->rxBufferIndex];
}

uint8_t i2c_tinyS::readByte_(struct i2cStruct* i2c) {
    if(i2c->rxBufferIndex >= i2c->rxBufferLength) return 0;
    return i2c->Buffer[i2c->rxBufferIndex++];
}

uint8_t i2c_tinyS::peekByte_(struct i2cStruct* i2c) {
    if(i2c->rxBufferIndex >= i2c->rxBufferLength) return 0;
    return i2c->Buffer[i2c->rxBufferIndex];
}

void i2c_isr_handler() {
    struct i2cStruct *i2c = &(i2c_tinyS::i2cData);
    byte status = TWSSRA;
    if ((status & (_BV(TWC) | _BV(TWBE)))) {
        i2c->startCount = -1;
        CLEAR_TENBIT;
        TWSSRA |= (_BV(TWASIF) | _BV(TWDIF) | _BV(TWBE));
        return;
    }
    if ((status & _BV(TWASIF)) || IS_TENBIT) {
        if ((status & _BV(TWAS))) {
            if (IS_TENBIT) i2c->Addr = (((i2c->Addr & B110) << 7) | TWSD);
            else {
                i2c->Addr = TWSD;
                i2c->startCount++;
                if ((i2c->Addr & B11111001) == B11110000) {
                    SET_TENBIT;
                    TWSCRB = (B0011 | TWI_HIGH_NOISE_MODE);
                    return;
                }
            }
            if (i2c->user_onAddrReceive != (void *)NULL) {
                i2c->rxBufferIndex = 0;
                if (!i2c->user_onAddrReceive(i2c->Addr, i2c->startCount)) {
                    TWSCRB = (B0111 | TWI_HIGH_NOISE_MODE);
                    return;
                }
            }
            if ((status & _BV(TWDIR))) {
                i2c->txBufferLength = 0;
                if (i2c->user_onRequest != (void *)NULL) i2c->user_onRequest();
                i2c->txBufferIndex = 0;
            } else i2c->rxBufferLength = 0;
        } else {
            if ((status & _BV(TWDIR))) {
                if (i2c->user_onStop != (void *)NULL) i2c->user_onStop();
            } else {
                if (i2c->user_onReceive != (void *)NULL) {
                    i2c->rxBufferIndex = 0;
                    i2c->user_onReceive(i2c->rxBufferLength);
                }
            }
            i2c->startCount = -1;
            CLEAR_TENBIT;
            TWSSRA = _BV(TWASIF);
            return;
        }
    } else if ((status & _BV(TWDIF))) {
        if ((status & _BV(TWDIR))) {
            if (i2c->txBufferIndex < i2c->txBufferLength) TWSD = i2c->Buffer[i2c->txBufferIndex++];
            else {
                TWSCRB = (B0010 | TWI_HIGH_NOISE_MODE);
                return;
            }
        } else {
            if (i2c->rxBufferLength < I2C_BUFFER_LENGTH) i2c->Buffer[i2c->rxBufferLength++] = TWSD;
            else {
                TWSCRB = (B0110 | TWI_HIGH_NOISE_MODE);
                return;
            }
        }
    }
    TWSCRB = (B0011 | TWI_HIGH_NOISE_MODE);
}

ISR(TWI_SLAVE_vect) {
    I2C_INTR_FLAG_ON;
    i2c_isr_handler();
    I2C_INTR_FLAG_OFF;
}

i2c_tinyS TinyWireS  = i2c_tinyS();

#endif
