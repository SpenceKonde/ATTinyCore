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
    
  ATtiny85 Instructions...
  
  - Note: This Sketch is too large to fit on the ATtiny45 processor.  Use the 
    Interactive_to_Serial example instead.
    
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

const char Header[] PROGMEM =
"                                     Left                               This                               Right\r\n"
"                                     -------------------------------    -------------------------------    -------------------------------\r\n"
"Step  OSCCAL  State      Position    Cal   9bt  Err  #   C9bt   CClk    Cal   9bt  Err  #   C9bt   CClk    Cal   9bt  Err  #   C9bt   CClk\r\n"
"----  ------  ---------  --------    ---  ----  ---  -  -----  -----    ---  ----  ---  -  -----  -----    ---  ----  ---  -  -----  -----\r\n";

const char StateFirstPass[] PROGMEM = "FirstPass  ";
const char StateBigSteps[]  PROGMEM = "BigSteps   ";
const char StateConfirm[]   PROGMEM = "Confirm    ";
const char StateFinished[]  PROGMEM = "Finished   ";

const char PositionLeft[]  PROGMEM = "Left ";
const char PositionThis[]  PROGMEM = "This ";
const char PositionRight[] PROGMEM = "Right";
const char PositionMax[]   PROGMEM = "Max  ";

/*
                                     Left                               This                               Right
                                     -------------------------------    -------------------------------    -------------------------------
Step  OSCCAL  State      Position    Cal   9bt  Err  #   C9bt   CClk    Cal   9bt  Err  #   C9bt   CClk    Cal   9bt  Err  #   C9bt   CClk
----  ------  ---------  --------    ---  ----  ---  -  -----  -----    ---  ----  ---  -  -----  -----    ---  ----  ---  -  -----  -----
  1     9B    BigSteps   This         -1     0    0  0      0      0     27     0    0  0      0      0     33  1550  250  0      0      0
  2     9A    BigSteps   This         -1     0    0  0      0      0     26     0    0  0      0      0     27  1508   40  0      0      0
  3     9A    Confirm    This         25  1498   10  0      0      0     26  1498   10  1   1498      0     27  1508   40  1   1508      0
  4     9A    Confirm    This         25  1498   10  0      0      0     26  1497   10  2   2995      0     27  1508   40  1   1508      0
  5     9A    Confirm    This         25  1498   10  0      0      0     26  1500   10  0      0      0     27  1508   40  1   1508      0
  6     9A    Confirm    This         25  1498   10  0      0      0     26  1500   10  1   1500      0     27  1508   40  1   1508      0
  7     9A    Confirm    This         25  1498   10  0      0      0     26  1500   10  2   3000      0     27  1508   40  1   1508      0
  8     99    Confirm    Left         25  1498   10  0      0      0     26  1499   10  3   4499      0     27  1508   40  1   1508      0
  9     99    Confirm    Left         25  1493   10  1   1493      0     26  1499   10  3   4499      0     27  1508   40  1   1508      0
 10     99    Confirm    Left         25  1492   10  2   2985      0     26  1499   10  3   4499      0     27  1508   40  1   1508      0
 11     9B    Confirm    Right        25  1493   10  3   4478      0     26  1499   10  3   4499      0     27  1508   40  1   1508      0
 12     9B    Confirm    Right        25  1493   10  3   4478      0     26  1499   10  3   4499      0     27  1508   40  2   3016      0
 13     9A    Finished   This         25  1493   37  3   4478   7463     26  1499    2  3   4499   7498     27  1509   42  3   4525   7542
*/

const char Bye1[] PROGMEM = 
"\r\n"
"Copy-and-paste the following line of code at the top of setup...\r\n\r\n"
"  OSCCAL = 0x";

const char Bye2[] PROGMEM = 
";\r\n\r\n"
"--------------------------------------------------\r\n";

const char Pad2[] PROGMEM = "  ";
const char Pad3[] PROGMEM = "   ";
const char Pad4[] PROGMEM = "    ";
const char Pad5[] PROGMEM = "     ";
const char Pad7[] PROGMEM = "       ";

void setup( void )
{
  Serial.begin( 9600 );
  Serial_printP( Hello );
}

void loop( void )
{
  TinyTuner tt;
  bool KeepGoing;
  unsigned Step;
  TinyTuner::position_t position;
  TinyTuner::debug_t debug;

/*
Find a resolution to this...

c:/arduino/arduino-0019/hardware/tools/avr/bin/../lib/gcc/avr/4.3.2/../../../../avr/lib/avr25/crttn85.o: In function `__vector_default':
(.vectors+0x8): relocation truncated to fit: R_AVR_13_PCREL against symbol `__vector_4' defined in .text.__vector_4 section in core.a(wiring.c.o)

...and then it will be possible to...

  tt.fullInit();
*/

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
    
    tt.getDebug( debug );
    Serial.print( debug.calibration, HEX );
    Serial_printP( Pad4 );
    switch ( debug.state )
    {
      case TinyTuner::sFirstPass:
        Serial_printP( StateFirstPass );
        break;
      case TinyTuner::sBigSteps:
        Serial_printP( StateBigSteps );
        break;
      case TinyTuner::sConfirm:
        Serial_printP( StateConfirm );
        break;
      case TinyTuner::sFinished:
        Serial_printP( StateFinished );
        break;
    }
    switch ( debug.position )
    {
      case TinyTuner::pLeft:
        Serial_printP( PositionLeft );
        break;
      case TinyTuner::pThis:
        Serial_printP( PositionThis );
        break;
      case TinyTuner::pRight:
        Serial_printP( PositionRight );
        break;
      case TinyTuner::pMax:
        Serial_printP( PositionMax );
        break;
    }
    Serial_printP( Pad3 );
    for ( position=TinyTuner::pLeft; position < TinyTuner::pMax; position=(TinyTuner::position_t)(position+1) )
    {
      Serial_printP( Pad4 );
      Serial_printPaddedIntger( debug.info[position].OsccalValue, 3 );
      Serial_printP( Pad2 );
      Serial_printPaddedIntger( debug.info[position].NineBitTime, 4 );
      Serial_printP( Pad2 );
      Serial_printPaddedIntger( debug.info[position].Error, 3 );
      Serial_printP( Pad2 );
      Serial_printPaddedIntger( debug.info[position].ConfirmCount, 1 );
      Serial_printP( Pad2 );
      Serial_printPaddedIntger( debug.info[position].ConfirmNineBitTime, 5 );
      Serial_printP( Pad2 );
      Serial_printPaddedIntger( debug.info[position].ConfirmClocks, 5 );
    }
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

void Serial_printPaddedIntger( long v, int width )
{
  bool Negative;
  long Power10;
  int i;
  
  --width;
  
  if ( v < 0 )
  {
    v = -v;
    Negative = true;
    --width;
  }
  else
  {
    Negative = false;
  }

  Power10 = 10L;
  for ( i=1; i <= 9; ++i )
  {
    if ( v >= Power10 )
    {
      --width;
    }
    else
    {
      break;
    }
    Power10 = Power10 * 10;
  }
  
  while ( width > 0 )
  {
    Serial.write( ' ' );
    --width;
  }
  
  if ( Negative )
  {
    Serial.write( '-' );
  }
  
  Serial.print( v, DEC );
}
