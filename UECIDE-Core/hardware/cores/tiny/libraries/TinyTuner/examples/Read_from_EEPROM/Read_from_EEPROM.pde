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

  The purpose of this example is to print the optimal calibration register 
  value from a previous run of the Save_to_EEPROM example.

================================================================================  

  ATtiny84 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB0 / Pin 0 to receive on the serial converter
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor
  
  - The value from EEPROM address zero should be displayed

================================================================================  
    
  ATtiny85 / ATtiny45 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB3 / Pin 3 to receive on the serial converter
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor
  
  - The value from EEPROM address zero should be displayed
  
==============================================================================*/

#include <EEPROM.h>

void setup( void )
{
  Serial.begin( 9600 );

  Serial.println( "\r\n\r\n\r\n" );
  Serial.println( "Poor Man's Tiny Tuner\n" );
  Serial.println( "Place this line of code at the top of setup...\r\n" );
  Serial.print( "  OSCCAL = 0x" );
  Serial.print( EEPROM.read( 0 ), HEX );
  Serial.println( ";\r\n" );
}

void loop( void )
{
}
