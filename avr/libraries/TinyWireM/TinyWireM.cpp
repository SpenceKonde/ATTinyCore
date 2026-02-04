/*
   TinyWireM.cpp - a wrapper class for TWI/I2C Master library for the ATtiny on Arduino
  1/21/2011 BroHogan -  brohoganx10 at gmail dot com

  **** See TinyWireM.h for Credits and Usage information ****

  This library is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2.1 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

extern "C" {
  //#include "USI_TWI_Master.h"
  //#include <USI_TWI_Master.h>
  //#include <USI_TWI_Master\USI_TWI_Master.h>
  //#include <USI_TWI_Master/USI_TWI_Master.h>
}

#include "USI_TWI_Master.h"
#include "TinyWireM.h"


// Initialize Class Variables //////////////////////////////////////////////////
	uint8_t USI_TWI::USI_Buf[USI_BUF_SIZE];             // holds I2C send and receive data
	uint8_t USI_TWI::USI_BufIdx = 0;                    // current number of bytes in the send buff
	uint8_t USI_TWI::USI_LastRead = 0;                  // number of bytes read so far
	uint8_t USI_TWI::USI_BytesAvail = 0;                // number of bytes requested but not read

// Constructors ////////////////////////////////////////////////////////////////

USI_TWI::USI_TWI(){
}

// Public Methods //////////////////////////////////////////////////////////////

//int USI_TWI::peek(){}
//void USI_TWI::flush(){}

void USI_TWI::begin(){ // initialize I2C lib
  USI_TWI_Master_Initialise();          
}

void USI_TWI::beginTransmission(uint8_t slaveAddr){ // setup address & write bit
  USI_BufIdx = 0; 
  USI_Buf[USI_BufIdx] = (slaveAddr<<TWI_ADR_BITS) | USI_SEND; 
}

size_t USI_TWI::write(uint8_t data){ // buffers up data to send
  if (USI_BufIdx >= USI_BUF_SIZE) return 0;       // dont blow out the buffer
  USI_BufIdx++;                                   // inc for next byte in buffer
  USI_Buf[USI_BufIdx] = data;
  return 1;
}

uint8_t USI_TWI::endTransmission() {
  endTransmission(1);
}

uint8_t USI_TWI::endTransmission(uint8_t stop){ // actually sends the buffer
  bool xferOK = false;
  uint8_t errorCode = 0;
  xferOK = USI_TWI_Start_Read_Write(USI_Buf,USI_BufIdx+1); // core func that does the work
  USI_BufIdx = 0;
  if (xferOK) {
    if (stop) {
      errorCode = USI_TWI_Master_Stop();
      if (errorCode == 0) {
        errorCode = USI_TWI_Get_State_Info();
        return errorCode;
      }
    }
    return 0;
  }
  else {                                  // there was an error
    errorCode = USI_TWI_Get_State_Info(); // this function returns the error number
    return errorCode;
  }
}

uint8_t USI_TWI::requestFrom(uint8_t slaveAddr, uint8_t numBytes){ // setup for receiving from slave
  bool xferOK = false;
  uint8_t errorCode = 0;
  USI_LastRead = 0;
  USI_BytesAvail = numBytes; // save this off in a global
  numBytes++;                // add extra byte to transmit header
  USI_Buf[0] = (slaveAddr<<TWI_ADR_BITS) | USI_RCVE;   // setup address & Rcve bit
  xferOK = USI_TWI_Start_Read_Write(USI_Buf,numBytes); // core func that does the work
  // USI_Buf now holds the data read
  if (xferOK) {
    errorCode = USI_TWI_Master_Stop();
    if (errorCode == 0) {
      errorCode = USI_TWI_Get_State_Info();
      return errorCode;
    }
    return 0;
  }
  else {                                  // there was an error
    errorCode = USI_TWI_Get_State_Info(); // this function returns the error number
    return errorCode;
  }
}

int USI_TWI::read(){ // returns the bytes received one at a time
  USI_LastRead++;     // inc first since first uint8_t read is in USI_Buf[1]
  return USI_Buf[USI_LastRead];
}

int USI_TWI::available(){ // the bytes available that haven't been read yet
  return USI_BytesAvail - (USI_LastRead); 
}


// Preinstantiate Objects //////////////////////////////////////////////////////

USI_TWI TinyWireM = USI_TWI();

