/*==============================================================================

  Copyright 2010 Rowdy Dog Software.

  This file is part of TinyTuner.

  TinyTuner is free software: you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option)
  any later version.

  TinyTuner is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with TinyTuner.  If not, see <http://www.gnu.org/licenses/>.

  Inspired by the work of oPossum (Kevin Timmerman)...
  http://forums.adafruit.com/viewtopic.php?t=5078

==============================================================================*/

/*==============================================================================

  The purpose of this example is to determine the optimal calibration register 
  value based on input from a serial port and store the value at EEPROM 
  address zero.

================================================================================  

  ATtiny84 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB1 / Pin 1 to transmit on the serial converter
  
  - Connect an LED + resistor --> ground on PA6 / Pin 4
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor
  
  - Continue sending single 'x' characters (no carriage-return; no line-feed)
    until the LED glows solid.  As each 'x' is sent, the LED should blink.
    
  - The calibration register value is stored at EEPROM address zero

================================================================================  
    
  ATtiny85 / ATtiny45 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB4 / Pin 4 to transmit on the serial converter
  
  - Connect an LED + resistor --> ground on PB0 / Pin 0
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor
  
  - Continue sending single 'x' characters (no carriage-return; no line-feed)
    until the LED glows solid.  As each 'x' is sent, the LED should blink.
    
  - The calibration register value is stored at EEPROM address zero
  
==============================================================================*/

#include <EEPROM.h>
#include <TinyTuner.h>
#include "s2eLed.h"

void setup( void )
{
  pinMode( PinLED, OUTPUT );
}

void loop( void )
{
  TinyTuner tt;
  bool KeepGoing = true;

  while ( KeepGoing )
  {
    KeepGoing = tt.update();

    for ( uint8_t i=0; i < 2; ++i )
    {    
      digitalWrite( PinLED, ! digitalRead( PinLED ) );
      delay( 100 );
    }
  }
  uint8_t Temp = OSCCAL;
  EEPROM.write( 0, Temp );
  
  if ( EEPROM.read(0) == Temp )
  {
    digitalWrite( PinLED, HIGH );
  }
  else
  {
    digitalWrite( PinLED, LOW );
  }
  while ( 1 );
}