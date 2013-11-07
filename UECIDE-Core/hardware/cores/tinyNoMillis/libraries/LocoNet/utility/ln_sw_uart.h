/****************************************************************************
    Copyright (C) 2002 Alex Shepherd

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************

 Title :   LocoNet Software UART Access library (ln_sw_uart.h)
 Author:   Alex Shepherd <kiwi64ajs@sourceforge.net>
 Date:     13-Aug-2002
 Software:  AVR-GCC with AVR-AS
 Target:    any AVR device

 DESCRIPTION
  Basic routines for interfacing to the LocoNet via any output pin and
  either the Analog Comparator pins or the Input Capture pin

  The receiver uses the Timer1 Input Capture Register and Interrupt to detect
  the Start Bit and then the Compare A Register for timing the subsequest
  bit times.

  The Transmitter uses just the Compare A Register for timing all bit times
       
 USAGE
  See the C include ln_interface.h file for a description of each function
       
*****************************************************************************/

#ifndef _LN_SW_UART_INCLUDED
#define _LN_SW_UART_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#ifndef LN_SW_UART_SET_TX_LOW                               // putting a 1 to the pin to switch on NPN transistor
#define LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT)  LN_TX_PORT |= (1 << LN_TX_BIT)   // to pull down LN line to drive low level
#endif

#ifndef LN_SW_UART_SET_TX_HIGH                              // putting a 0 to the pin to switch off NPN transistor
#define LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT) LN_TX_PORT &= ~(1 << LN_TX_BIT)   // master pull up will take care of high LN level
#endif

// For now we will simply check that TX and RX ARE NOT THE SAME as our circuit
// requires the TX signal to be INVERTED.  If they are THE SAME then we have a 
// Collision.  
// Define LN_SW_UART_TX_NON_INVERTED in your board header if your circuit doesn't
// (for example) use a NPN between TX pin and the Loconet port...

#ifdef LN_SW_UART_TX_NON_INVERTED
#define IS_LN_COLLISION()	(((LN_TX_PORT >> LN_TX_BIT) & 0x01) != ((LN_RX_PORT >> LN_RX_BIT) & 0x01))
#else  								// inverted is the normal case, RX same as TX means a collision...
#define IS_LN_COLLISION()	(((LN_TX_PORT >> LN_TX_BIT) & 0x01) == ((LN_RX_PORT >> LN_RX_BIT) & 0x01))
#endif


#define LN_ST_IDLE            0   // net is free for anyone to start transmission
#define LN_ST_CD_BACKOFF      1   // timer interrupt is counting backoff bits
#define LN_ST_TX_COLLISION    2   // just sending break after creating a collision
#define LN_ST_TX              3   // transmitting a packet
#define LN_ST_RX              4   // receiving bytes

#define LN_COLLISION_TICKS 15
#define LN_TX_RETRIES_MAX  25

          // The Start Bit period is a full bit period + half of the next bit period
          // so that the bit is sampled in middle of the bit
#define LN_TIMER_RX_START_PERIOD    LN_BIT_PERIOD + (LN_BIT_PERIOD / 2)
#define LN_TIMER_RX_RELOAD_PERIOD   LN_BIT_PERIOD 
#define LN_TIMER_TX_RELOAD_PERIOD   LN_BIT_PERIOD 

// ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// LN_TIMER_TX_RELOAD_ADJUST is a value for an error correction. This is needed for 
// every start of a byte. The first bit is to long. Therefore we need to reduce the 
// reload value of the bittimer.
// The following value depences highly on used compiler, optimizationlevel and hardware.
// Define the value in sysdef.h. This is very project specific.
// For the FREDI hard- and software it is nearly a quarter of a LN_BIT_PERIOD.
// Olaf Funke, 19th October 2007
// ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef LN_TIMER_TX_RELOAD_ADJUST
	#define LN_TIMER_TX_RELOAD_ADJUST   0
	// #error detect value by oszilloscope
#endif

// ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void initLocoNetHardware( LnBuf *RxBuffer );
void setTxPortAndPin(volatile uint8_t *newTxPort, uint8_t newTxPin);
LN_STATUS sendLocoNetPacketTry(lnMsg *TxData, unsigned char ucPrioDelay);

#endif
