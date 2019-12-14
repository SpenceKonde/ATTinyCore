/*
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/
#include <avr/io.h>
#ifdef TWDR
extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #include "twi.h"
}

#include "Wire.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxBufferIndex = 0;
uint8_t TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txBufferIndex = 0;
uint8_t TwoWire::txBufferLength = 0;

uint8_t TwoWire::transmitting = 0;
void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire()
{
}

// Public Methods //////////////////////////////////////////////////////////////

void TwoWire::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  twi_init();
}

void TwoWire::begin(uint8_t address)
{
  twi_setAddress(address);
  twi_attachSlaveTxEvent(onRequestService);
  twi_attachSlaveRxEvent(onReceiveService);
  begin();
}

void TwoWire::begin(int address)
{
  begin((uint8_t)address);
}

void TwoWire::end(void)
{
  twi_disable();
}

void TwoWire::setClock(uint32_t frequency)
{
  TWBR = ((F_CPU / frequency) - 16) / 2;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop)
{
  if (isize > 0) {
  // send internal address; this mode allows sending a repeated start to access
  // some devices' internal registers. This function is executed by the hardware
  // TWI module on other processors (for example Due's TWI_IADR and TWI_MMR registers)

  beginTransmission(address);

  // the maximum size of internal address is 3 bytes
  if (isize > 3){
    isize = 3;
  }

  // write internal register address - most significant byte first
  while (isize-- > 0)
    write((uint8_t)(iaddress >> (isize*8)));
  endTransmission(false);
  }

  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform blocking read into buffer
  uint8_t read = twi_readFrom(address, rxBuffer, quantity, sendStop);
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0, (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void TwoWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

//
//  Originally, 'endTransmission' was an f(void) function.
//  It has been modified to take one parameter indicating
//  whether or not a STOP should be performed on the bus.
//  Calling endTransmission(false) allows a sketch to
//  perform a repeated start.
//
//  WARNING: Nothing in the library keeps track of whether
//  the bus tenure has been properly ended with a STOP. It
//  is very possible to leave the bus in a hung state if
//  no call to endTransmission(true) is made. Some I2C
//  devices will behave oddly if they do not see a STOP.
//
uint8_t TwoWire::endTransmission(uint8_t sendStop)
{
  // transmit buffer (blocking)
  int8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1, sendStop);
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return ret;
}

//  This provides backwards compatibility with the original
//  definition, and expected behaviour, of endTransmission
//
uint8_t TwoWire::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      setWriteError();
      return 0;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer
    txBufferLength = txBufferIndex;
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(&data, 1);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  if(transmitting){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(data, quantity);
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void)
{
  int value = -1;

  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void)
{
  int value = -1;

  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
  }

  return value;
}

void TwoWire::flush(void)
{
  // XXX: to be implemented.
}

// behind the scenes function that is called when data is received
void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void TwoWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

TwoWire Wire = TwoWire();

#else
#ifdef USIDR //BEGIN USI TWI IMPLEMENTATION
extern "C" {
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "USI_TWI_Slave/USI_TWI_Slave.h"
#include "USI_TWI_Master/USI_TWI_Master.h"
}

#include "Wire.h"

const uint8_t WIRE_BUFFER_LENGTH = TWI_BUFFER_SIZE - 1; //reserve slave addr

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t *TwoWire::Buffer = TWI_Buffer;
uint8_t TwoWire::BufferIndex = 0;
uint8_t TwoWire::BufferLength = 0;

uint8_t TwoWire::transmitting = 0;

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire() {
}

// Public Methods //////////////////////////////////////////////////////////////

void TwoWire::begin(void) {
  BufferIndex = 0;
  BufferLength = 0;

  transmitting = 0;

  USI_TWI_Master_Initialise();
}

void TwoWire::begin(uint8_t address) {
  USI_TWI_Slave_Initialise(address);
}

void TwoWire::begin(int address) {
  begin((uint8_t)address);
}

void TwoWire::end(void) {
  USI_TWI_Slave_Disable();
}

void TwoWire::setClock(uint32_t clock) {
  // XXX: to be implemented.
  (void)clock; //disable warning
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint32_t iaddress, uint8_t isize,
                             uint8_t sendStop) {
  if (isize > 0) {
    // send internal address; this mode allows sending a repeated
    // start to access some devices' internal registers. This function
    // is executed by the hardware TWI module on other processors (for
    // example Due's TWI_IADR and TWI_MMR registers)

    beginTransmission(address);

    // the maximum size of internal address is 3 bytes
    if (isize > 3) {
      isize = 3;
    }

    // write internal register address - most significant byte first
    while (isize-- > 0) {
      write((uint8_t)(iaddress >> (isize*8)));
    }
    endTransmission(false);
  }

  // reserve one byte for slave address
  quantity++;
  // clamp to buffer length
  if (quantity > TWI_BUFFER_SIZE) {
    quantity = TWI_BUFFER_SIZE;
  }
  // set address of targeted slave and read mode
  Buffer[0] = (address << TWI_ADR_BITS) | (1 << TWI_READ_BIT);
  // perform blocking read into buffer
  uint8_t ret = USI_TWI_Start_Transceiver_With_Data_Stop(Buffer, quantity,
                                                         sendStop);
  // set rx buffer iterator vars
  BufferIndex = 1; // ignore slave address
  // check for error
  if (ret == FALSE) {
    BufferLength = BufferIndex;
    return 0;
  }
  BufferLength = quantity;

  return quantity - 1; // ignore slave address
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0,
                     (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address) {
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave and write mode
  Buffer[0] = (address << TWI_ADR_BITS) | (0 << TWI_READ_BIT);
  // reset tx buffer iterator vars
  BufferIndex = 1; // reserved by slave address
  BufferLength = BufferIndex;
}

void TwoWire::beginTransmission(int address) {
  beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(uint8_t sendStop) {
  // transmit buffer (blocking)
  uint8_t ret = USI_TWI_Start_Transceiver_With_Data_Stop(Buffer,
                                                         BufferLength,
                                                         sendStop);
  // reset tx buffer iterator vars
  BufferIndex = 0;
  BufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  // check for error
  if (ret == FALSE) {
    switch (USI_TWI_Get_State_Info()) {
    case USI_TWI_DATA_OUT_OF_BOUND:
      return 1; //data too long to fit in transmit buffer
    case USI_TWI_NO_ACK_ON_ADDRESS:
      return 2; //received NACK on transmit of address
    case USI_TWI_NO_ACK_ON_DATA:
      return 3; //received NACK on transmit of data
    }
    return 4; //other error
  }
  return 0; //success
}

uint8_t TwoWire::endTransmission(void) {
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data) {
  if (transmitting) { // in master transmitter mode
    // don't bother if buffer is full
    if (BufferLength >= TWI_BUFFER_SIZE) {
      return 0;
    }
    // put byte in tx buffer
    Buffer[BufferIndex] = data;
    ++BufferIndex;
    // update amount in buffer
    BufferLength = BufferIndex;
  } else { // in slave send mode
    // don't bother if buffer is full
    if (!USI_TWI_Space_In_Transmission_Buffer()) {
      return 0;
    }
    // reply to master
    USI_TWI_Transmit_Byte(data);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity) {
  size_t numBytes = 0;
  for (size_t i = 0; i < quantity; ++i){
    numBytes += write(data[i]);
  }
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const char *str) {
  if (str == NULL) return 0;
  return write((const uint8_t *)str, strlen(str));
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  if (BufferLength) {
    return BufferLength - BufferIndex;
  } else {
    return USI_TWI_Data_In_Receive_Buffer();
  }
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  int value = -1;

  // get each successive byte on each call
  if (available()) {
    if (BufferLength) {
      value = Buffer[BufferIndex];
      ++BufferIndex;
    } else {
      value = USI_TWI_Receive_Byte();
    }
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void) {
  int value = -1;

  if (available()) {
    if (BufferLength) {
      value = Buffer[BufferIndex];
    } else {
      value = USI_TWI_Peek_Receive_Byte();
    }
  }

  return value;
}

void TwoWire::flush(void) {
  // XXX: to be implemented.
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) ) {
  USI_TWI_On_Slave_Receive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) ) {
  USI_TWI_On_Slave_Transmit = function;
}

// return true on I2C/TWI activity
uint8_t TwoWire::isActive(void) {
  return USI_TWI_Slave_Is_Active();
}

// Preinstantiate Objects //////////////////////////////////////////////////////s
TwoWire Wire = TwoWire();
#else
#if defined(TWSD) && !defined(__AVR_ATtiny1634__)

#include "Wire.h"
//#include "TWSWire.h"
#include <string.h>

#if defined(__AVR_ATtiny841__) || defined(__AVR_ATtiny441__)
#define SCL_PIN 4
#define SCL_PORT PORTA
#define SDA_PIN 6
#define SDA_PORT PORTA
#elif defined(__AVR_ATtiny828__)
#define SCL_PIN 3
#define SCL_PORT PORTD
#define SDA_PIN 0
#define SDA_PORT PORTD
#else
#error No pins for software I2C defined
#endif
#define I2C_TIMEOUT 100
#ifndef WIRE_SLAVE_ONLY
#include "SoftWire.h"
static SoftWire softWire;
#endif

#ifndef WIRE_MASTER_ONLY
#include "WireS.h"
#endif

// Constructors ////////////////////////////////////////////////////////////////
//#ifndef WIRE_MASTER_ONLY
TwoWire::TwoWire() : slaveMode(false) {
}
//#else
//TwoWire::TwoWire()
//#endif
// Public Methods //////////////////////////////////////////////////////////////

#ifdef WIRE_BOTH
void TwoWire::begin(void) {
  softWire.begin();
  slaveMode = false;
}
#endif
#ifdef WIRE_MASTER_ONLY
void TwoWire::begin(void) {
  softWire.begin();
}
#endif


#ifndef WIRE_MASTER_ONLY
void TwoWire::begin(uint8_t address) {
  TinyWireS.begin(address, 0);
  slaveMode = true;
}

void TwoWire::begin(int address) {
  begin((uint8_t)address);
}
#endif

void TwoWire::end(void) {
  // XXX: to be implemented.
}

void TwoWire::setClock(uint32_t clock) {
  // XXX: to be implemented.
  (void)clock; //disable warning
}
#ifndef WIRE_SLAVE_ONLY
uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
  uint32_t iaddress, uint8_t isize,
  uint8_t sendStop) {
  #ifndef WIRE_MASTER_ONLY
  if (!slaveMode) {
    #endif
    return softWire.requestFrom(address, quantity, iaddress, isize, sendStop);
  #ifndef WIRE_MASTER_ONLY
  }
  return 0;
  #endif
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
  uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0,
    (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address) {
  if (!slaveMode) {
    softWire.beginTransmission(address);
  }
}

void TwoWire::beginTransmission(int address) {
  beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(uint8_t sendStop) {

  #ifndef WIRE_MASTER_ONLY
  if (!slaveMode) {
  #endif
    return softWire.endTransmission(sendStop);
  #ifndef WIRE_MASTER_ONLY
  }
  return 4; //other error
  #endif
}

uint8_t TwoWire::endTransmission(void) {
  return endTransmission(true);
}

#endif //stuff that's not there in slave only

// must be called in:
// slave tx event callback
// or after beginTransmission(address)



#if defined(WIRE_BOTH)
size_t TwoWire::write(uint8_t data) {
  size_t numBytes = 0;
  if (!slaveMode) {
    numBytes = softWire.write(data);
  }
  else {
    numBytes = TinyWireS.write(data);
  }
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)

// must be called in:
// slave tx event callback
// or after beginTransmission(address)


// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  int value = -1;
  if (!slaveMode) {
    value = softWire.available();
  }
  else {
    value = TinyWireS.available();
  }
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  int value = -1;
  if (!slaveMode) {
    value = softWire.read();
  }
  else {
    value = TinyWireS.read();
  }
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void) {
  int value = -1;
  if (!slaveMode) {
    value = softWire.peek();
  }
  else {
    value = TinyWireS.peek();
  }
  return value;
}
#elif defined(WIRE_MASTER_ONLY)
size_t TwoWire::write(uint8_t data) {
  size_t numBytes = 0;
    numBytes = softWire.write(data);
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)


// must be called in:
// slave tx event callback
// or after beginTransmission(address)


// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  int value = -1;
    value = softWire.available();
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  int value = -1;
  value = softWire.read();
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void) {
  int value = -1;
  value = softWire.peek();
  return value;
}
#else //WIRE_SLAVE_ONLY
size_t TwoWire::write(uint8_t data) {
  size_t numBytes = 0;
  numBytes = TinyWireS.write(data);
  return numBytes;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)


// must be called in:
// slave tx event callback
// or after beginTransmission(address)


// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  int value = -1;
  value = TinyWireS.available();
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  int value = -1;
    value = TinyWireS.read();
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void) {
  int value = -1;
  value = TinyWireS.peek();
  return value;
}

#endif

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
  size_t numBytes = 0;
  for (size_t i = 0; i < quantity; ++i) {
    numBytes += write(data[i]);
  }
  return numBytes;
}

size_t TwoWire::write(const char *str) {
  if (str == NULL) return 0;
  return write((const uint8_t *)str, strlen(str));
}

void TwoWire::flush(void) {
  // XXX: to be implemented.
}
#ifndef WIRE_MASTER_ONLY
// sets function called on slave write
//void TwoWire::onReceive(void(*function)(int)) {
//  TinyWireS.onReceive(function);
//}
void TwoWire::onReceive( void (*function)(size_t) ) {
  TinyWireS.onReceive(function);
}


void TwoWire::onReceive( void (*function)(int) ) {
  // arduino api compatibility fixer:
  // really hope size parameter will not exceed 2^31 :)
  static_assert(sizeof(int) == sizeof(size_t), "something is wrong in Arduino kingdom");
  TinyWireS.onReceive(reinterpret_cast<void(*)(size_t)>(function));
}

// sets function called on slave read
void TwoWire::onRequest(void(*function)(void)) {
  TinyWireS.onRequest(function);
}
#endif

// Preinstantiate Objects //////////////////////////////////////////////////////
TwoWire Wire = TwoWire();
#else
#error No Wire support on unknown board
#endif // TWSD
#endif // USIDR
#endif // TWDR
