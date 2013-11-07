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
  value based on input from a serial port and print the value to the serial 
  port.

================================================================================  

  ATtiny84 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB0 / Pin 0 to receive on the serial converter
  
  - Connect PB1 / Pin 1 to transmit on the serial converter
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor.  A welcome message should be displayed.
  
  - Continue sending single 'x' characters (no carriage-return; no line-feed)
    until the calibration finishes

================================================================================  
    
  ATtiny85 / ATtiny45 Instructions...
  
  - Select the correct board / serial port
  
  - Upload this Sketch to the processor
  
  - Connect PB3 / Pin 3 to receive on the serial converter
  
  - Connect PB4 / Pin 4 to transmit on the serial converter
  
  - Start your favorite terminal program, open the correct serial port, change
    the baud rate to 9600
    
  - Reset the processor.  A welcome message should be displayed.
  
  - Continue sending single 'x' characters (no carriage-return; no line-feed)
    until the calibration finishes
  
==============================================================================*/

#include <TinyTuner.h>
#include <avr/pgmspace.h>

const char Hello[] PROGMEM = 
"\r\n\r\n\r\n\r\n"
"--------------------------------------------------------------------------------\r\n"
"Poor Man's Tiny Tuner\r\n"
"Slowly send lowercase 'x' to tune the oscillator...\r\n\r\n";

const char StartingValue1[] PROGMEM = 
"  // Starting OSCCAL value is 0x";

const char StartingValue2[] PROGMEM = 
"\r\n\r\n";

const char Header[] PROGMEM =
"Step  OSCCAL\r\n";

const char Bye1[] PROGMEM = 
"\r\n"
"Copy-and-paste the following line of code at the top of setup...\r\n\r\n"
"  OSCCAL = 0x";

const char Bye2[] PROGMEM = 
";\r\n\r\n"
"--------------------------------------------------------------------------------\r\n";

const char Pad5[] PROGMEM = "     ";

void setup( void )
{
  Serial.begin( 9600 );
  Serial_printP( Hello );
  Serial_printP( StartingValue1 );
  Serial.print( OSCCAL, HEX );
  Serial_printP( StartingValue2 );
}

void loop( void )
{
  TinyTuner tt;
  bool KeepGoing;
  unsigned Step;

  Serial_printP( Header );
  
  KeepGoing = true;
  Step = 0;
  
  while ( KeepGoing )
  {
    ++Step;
    Serial.write( ' ' );
    
    if ( Step < 10 )
    {
      Serial.write( ' ' );
    }
    Serial.print( Step );
    Serial_printP( Pad5 );

    KeepGoing = tt.update();
    
    Serial.print( OSCCAL, HEX );
    Serial.println();
  }
  Serial_printP( Bye1 );
  Serial.print( OSCCAL, HEX );
  Serial_printP( Bye2 );
}

void Serial_printP( const char* p )
{
  char ch;
  
  ch = pgm_read_byte( p );
  
  while ( ch != 0 )
  {
    Serial.write( ch );
    ++p;
    ch = pgm_read_byte( p );
  }
}

