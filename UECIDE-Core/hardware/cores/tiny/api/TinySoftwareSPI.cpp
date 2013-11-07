/*
 * Copyright (c) 2012 by Thomas Carpenter
 * Software based SPI Master Library for Tiny core.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Currently, this runs at 125kHz on an 8MHz clock.
 */

#include "TinySoftwareSPI.h"
#include "Arduino.h"

SoftSPIClass::SoftSPIClass(){
	_bitOrder = MSBFIRST;
	_mode = SPI_MODE0;
	_running = false;
	transferType = &SoftSPIClass::noTransfer;
}

#if defined(SS) && defined(MOSI) && defined(MISO) && defined(SCK)
void SoftSPIClass::begin(){
	begin(SCK,MOSI,MISO,SS);
}
#endif

void SoftSPIClass::writeSS(boolean state){
	if (state) {
		*_SS_PORT |= _SS_HIGH;
	} else {
		*_SS_PORT &= _SS_LOW;
	}
}

void SoftSPIClass::begin(byte SCK_, byte MOSI_, byte MISO_, byte SS_){
	_SS = SS_;
	_SCK = SCK_;
	_MISO = MISO_;
	_MOSI = MOSI_;
	
	byte MOSIport = digitalPinToPort(_MOSI);
	byte SSport = digitalPinToPort(_SS);
	byte SCKport = digitalPinToPort(_SCK);
	byte MISOport = digitalPinToPort(_MISO);
		
	if ((MOSIport == NOT_A_PIN) ||
		(  SSport == NOT_A_PIN) ||
		( SCKport == NOT_A_PIN) ||
		(MISOport == NOT_A_PIN) ){
		end();
	} else {
		_running = true;
		pinMode(_MOSI, OUTPUT);
		pinMode(_MISO, INPUT);
		pinMode(_SCK, OUTPUT);
		pinMode(_SS, OUTPUT);
		_MOSI_PORT = portOutputRegister(MOSIport);
		_MOSI_HIGH = digitalPinToBitMask(_MOSI);
		_MOSI_LOW = ~_MOSI_HIGH;
		_SCK_PORT = portOutputRegister(SCKport);
		_SCK_HIGH = digitalPinToBitMask(_SCK);
		_SCK_LOW = ~_SCK_HIGH;
		_SS_PORT = portOutputRegister(SSport);
		_SS_HIGH = digitalPinToBitMask(_SS);
		_SS_LOW = ~_SS_HIGH;
		_MISO_PIN = portInputRegister(MISOport);
		_MISO_MASK = digitalPinToBitMask(_MISO);
		*_SS_PORT |= _SS_HIGH;
		*_SCK_PORT &= _SCK_LOW;
		*_MOSI_PORT &= _MOSI_LOW;
		
		//Default to Mode0.
		_mode = SPI_MODE0;
		transferType = &SoftSPIClass::transferMode0;
	}
}

byte SoftSPIClass::noTransfer(byte _data){
	//This does nothing. If you call SPI.transfer() before calling begin() or after calling end(), the call will be redirected here to avoid crash.
	return 0xFF;
}

byte SoftSPIClass::transferMode0(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		if(_data & 0x80){
			*_MOSI_PORT |= _MOSI_HIGH;
		} else {
			*_MOSI_PORT &= _MOSI_LOW;
		}
		_data <<= 1;
		*_SCK_PORT |= _SCK_HIGH;
		_newData <<= 1;
		_newData |= ((*_MISO_PIN & _MISO_MASK) ? 1 : 0);
		*_SCK_PORT &= _SCK_LOW;
	}
	return _newData;
}
byte SoftSPIClass::transferMode1(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		*_SCK_PORT |= _SCK_HIGH;
		if(_data & 0x80){
			*_MOSI_PORT |= _MOSI_HIGH;
		} else {
			*_MOSI_PORT &= _MOSI_LOW;
		}
		_data <<= 1;
		*_SCK_PORT &= _SCK_LOW;
		_newData <<= 1;
		_newData |= ((*_MISO_PIN & _MISO_MASK) ? 1 : 0);
	}
	return _newData;
}
byte SoftSPIClass::transferMode2(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		if(_data & 0x80){
			*_MOSI_PORT |= _MOSI_HIGH;
		} else {
			*_MOSI_PORT &= _MOSI_LOW;
		}
		_data <<= 1;
		*_SCK_PORT &= _SCK_LOW;
		_newData <<= 1;
		_newData |= ((*_MISO_PIN & _MISO_MASK) ? 1 : 0);
		*_SCK_PORT |= _SCK_HIGH;
	}
	return _newData;
}
byte SoftSPIClass::transferMode3(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		*_SCK_PORT &= _SCK_LOW;
		if(_data & 0x80){
			*_MOSI_PORT |= _MOSI_HIGH;
		} else {
			*_MOSI_PORT &= _MOSI_LOW;
		}
		_data <<= 1;
		*_SCK_PORT |= _SCK_HIGH;
		_newData <<= 1;
		_newData |= ((*_MISO_PIN & _MISO_MASK) ? 1 : 0);
	}
	return _newData;
}

byte SoftSPIClass::transfer(byte _data){
	byte _newData = 0;
    byte oldSREG = SREG;
	cli();
	if (_bitOrder == MSBFIRST){
	//Send data
		_newData = (*this.*transferType)(_data);
		SREG = oldSREG;
		return _newData;
	} else {
		//flip the data
		for(byte i = 0; i < 8; i++){
		  _newData <<= 1;
		  _newData |= _data & 1;
		  _data >>= 1;
		}
		//SPI transfer
		_newData = (*this.*transferType)(_newData);
		SREG = oldSREG;
		//flip data back.
		_data = 0;
		for(byte i = 0; i < 8; i++){
		  _data <<= 1;
		  _data |= _newData & 1;
		  _newData >>= 1;
		}
		return _data;
	}
}

void SoftSPIClass::end(){
	_running = false;
	transferType = &SoftSPIClass::noTransfer;
}

void SoftSPIClass::setBitOrder(uint8_t bitOrder) {
	_bitOrder = bitOrder;
}

void SoftSPIClass::setDataMode(uint8_t mode)
{
	_mode = mode;
	if(_mode == SPI_MODE0){
		transferType = &SoftSPIClass::transferMode0;
	} else if (_mode == SPI_MODE1){
		transferType = &SoftSPIClass::transferMode1;
	} else if (_mode == SPI_MODE2){
		transferType = &SoftSPIClass::transferMode2;
	} else if (_mode == SPI_MODE3){
		transferType = &SoftSPIClass::transferMode3;
	} else {
		_mode = SPI_MODE0;
		transferType = &SoftSPIClass::transferMode0;
	}
	if(_mode & 0x02){
		*_SCK_PORT |= _SCK_HIGH;
	} else {
		*_SCK_PORT &= _SCK_LOW;
	}
}

void SoftSPIClass::setClockDivider(uint8_t rate)
{
	
	//does nothing as the speed cannot be changed - fixed at Fcpu/16
	
}

SoftSPIClass SPI;