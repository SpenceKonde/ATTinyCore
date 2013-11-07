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
#include <NewSoftSerial.h>
#include <TinyTuner.h>

NewSoftSerial nss( 2, 1 );

void setup( void )
{
  nss.begin( 9600 );

  nss.println( "\r\n\r\n\r\n" );
//nss.println( "--------------------------------------------------" );
  nss.println( "Poor Man's Tiny Tuner\n" );
  nss.println( "Slowly send lowercase 'x' to tune the oscillator..." );
}


void loop( void )
{
  Tiny84Tuner tt;
  bool KeepGoing = true;

  while ( KeepGoing )
  {
    nss.print( OSCCAL, HEX );
    nss.print( ": " );

    KeepGoing = tt.update();
    
    switch ( tt.getState() )
    {
      case Tiny84Tuner::sFirstPass:  
        nss.print( "FirstPass " );
        break;
      case Tiny84Tuner::sBigSteps:   
        nss.print( "BigSteps " );
        break;
      case Tiny84Tuner::sConfirm:    
        nss.print( "Confirm " );
        break;
      case Tiny84Tuner::sFinished:   
        nss.print( "Finished " );
        break;
    }
    nss.println();
  }
  nss.println();
//nss.println( "Copy-and-paste the following line of code at the top of setup...\r\n" );
  nss.print( "  OSCCAL = 0x" );
  nss.print( OSCCAL, HEX );
  nss.println( ";\r\n" );
//nss.println( "\r\n--------------------------------------------------" );
}

