/****************************************************************************
 * 	Copyright (C) 2004 Alex Shepherd
 * 
 * 	Portions Copyright (C) Digitrax Inc.
 * 
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 * 
 * 	This library is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 * 
 * 	You should have received a copy of the GNU Lesser General Public
 * 	License along with this library; if not, write to the Free Software
 * 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Digitrax, Inc.
 * 	and are used with permission as part of the EmbeddedLocoNet project. That
 * 	permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	EmbeddedLocoNet, please contact Digitrax Inc, for specific permission.
 * 
 * 	Note: The sale any LocoNet device hardware (including bare PCB's) that
 * 	uses this or any other LocoNet software, requires testing and certification
 * 	by Digitrax Inc. and will be subject to a licensing agreement.
 * 
 * 	Please contact Digitrax Inc. for details.
 * 
 *****************************************************************************
 * 
 * 	Title :   LocoNet Buffer header file
 * 	Author:   Alex Shepherd <kiwi64ajs@sourceforge.net>
 * 	Date:     13-Feb-2004
 * 	Software:  AVR-GCC
 * 	Target:    AtMega8
 * 	
 * 	DESCRIPTION
 * 	This module provides functions that manage the sending and receiving of LocoNet packets.
 * 	
 * 	As bytes are received from the LocoNet, they are stored in a circular
 * 	buffer and after a valid packet has been received it can be read out.
 * 	
 * 	When packets are sent successfully, they are also appended to the Receive
 * 	circular buffer so they can be handled like they had been received from
 * 	another device.
 * 
 * 	Statistics are maintained for both the send and receiving of packets.
 * 
 * 	Any invalid packets that are received are discarded and the stats are
 * 	updated approproately.
 * 
 *****************************************************************************/

#ifndef _LN_BUF_INCLUDED
#define _LN_BUF_INCLUDED

#include <inttypes.h>
#include "ln_config.h"
#include "ln_opc.h"

#if defined (__cplusplus)
	extern "C" {
#endif

#ifndef LN_BUF_SIZE
#define LN_BUF_SIZE 128
#endif

typedef struct
{
  uint16_t RxPackets ;
  uint16_t RxErrors ;
  uint16_t TxPackets ;
  uint16_t TxErrors ;
  uint16_t Collisions ;
} 
LnBufStats ;

typedef struct
{
  uint8_t     Buf[ LN_BUF_SIZE ] ;
  uint8_t     WriteIndex ;
  uint8_t     ReadIndex ;
  uint8_t     ReadPacketIndex ;
  uint8_t     CheckSum ;
  uint8_t     ReadExpLen ;
  LnBufStats  Stats ;
} 
LnBuf ;

void initLnBuf( LnBuf *Buffer ) ;
lnMsg *recvLnMsg( LnBuf *Buffer ) ;
LnBufStats *getLnBufStats( LnBuf *Buffer ) ;
uint8_t getLnMsgSize( volatile lnMsg * newMsg ) ;

static inline void addByteLnBuf( LnBuf *Buffer, uint8_t newByte )
{
  Buffer->Buf[ Buffer->WriteIndex++ ] = newByte ;
  if( Buffer->WriteIndex >= LN_BUF_SIZE )
    Buffer->WriteIndex = 0 ;
}

static inline void addMsgLnBuf( LnBuf *Buffer, volatile lnMsg * newMsg )
{
  uint8_t	Index ;
  uint8_t 	Length ;

  Length = getLnMsgSize( newMsg ) ;
  for( Index = 0; Index < Length; Index++ )
    addByteLnBuf(Buffer, newMsg->data[ Index ] ) ;
}

static inline int lnPacketReady(LnBuf * Buffer) {
  return (Buffer->ReadIndex != Buffer->WriteIndex );
}

#if defined (__cplusplus)
}
#endif

#endif

