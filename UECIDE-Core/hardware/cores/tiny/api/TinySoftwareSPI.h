/*
 * Copyright (c) 2012 by Thomas Carpenter
 * Software based SPI Master Library for Tiny core.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
//Uses the same guard as the SPI class as the two cannot be used together
#define _SPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

class SoftSPIClass;
typedef byte (SoftSPIClass::*TransferFunction)(byte _data);

class SoftSPIClass {
public:
	SoftSPIClass();
private:
	TransferFunction transferType;
	byte noTransfer(byte _data);
	byte transferMode0(byte _data);
	byte transferMode1(byte _data);
	byte transferMode2(byte _data);
	byte transferMode3(byte _data);
public:
	byte transfer(byte _data);
	
	// SPI Configuration methods
	#if defined(SS) && defined(MOSI) && defined(MISO) && defined(SCK)
	void begin(); // Default to the preset SPI pins
	#endif
	void begin(byte SCK_, byte MOSI_, byte MISO_, byte SS_); //No SS specified, so require pin designation
	void end();
	
	void setBitOrder(uint8_t);
	void setDataMode(uint8_t);
	void setClockDivider(uint8_t);
	
	void writeSS(boolean state);
private:
	byte _rate;
	byte _bitOrder;
	byte _mode;
	boolean _running;
	byte _SS;
	byte _SCK;
	byte _MISO;
	byte _MOSI;
	volatile uint8_t* _MOSI_PORT;
	volatile uint8_t* _SS_PORT;
	volatile uint8_t* _SCK_PORT;
	volatile uint8_t* _MISO_PIN;
	byte _SS_HIGH;
	byte _MOSI_HIGH;
	byte _SCK_HIGH;
	byte _SS_LOW;
	byte _MOSI_LOW;
	byte _SCK_LOW;
	byte _MISO_MASK;
};

extern SoftSPIClass SPI;

#endif