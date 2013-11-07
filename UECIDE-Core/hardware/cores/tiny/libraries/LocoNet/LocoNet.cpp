/****************************************************************************
 * 	Copyright (C) 2009 Alex Shepherd
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

#include "LocoNet.h"
#include "ln_sw_uart.h"
#include "ln_config.h"

LocoNetClass::LocoNetClass()
{
}

void LocoNetClass::init(void)
{
  init(6); // By default use pin 6 as the Tx pin to be compatible with the previous library default 
}

void LocoNetClass::init(uint8_t txPin)
{
  initLnBuf(&LnBuffer) ;
  setTxPin(txPin);
  initLocoNetHardware(&LnBuffer);
}

void LocoNetClass::setTxPin(uint8_t txPin)
{
  pinMode(txPin, OUTPUT);
  
	// Not figure out which Port bit is the Tx Bit from the Arduino pin number
  uint8_t bitMask = digitalPinToBitMask(txPin);
  uint8_t bitMaskTest = 0x01;
  uint8_t bitNum = 0;
  
  uint8_t port = digitalPinToPort(txPin);
  volatile uint8_t *out = portOutputRegister(port);
  
  while(bitMask != bitMaskTest)
	bitMaskTest = 1 << ++bitNum;
	
  setTxPortAndPin(out, bitNum);
}

lnMsg* LocoNetClass::receive()
{
  return recvLnMsg(&LnBuffer);
}

LN_STATUS LocoNetClass::send(lnMsg *pPacket)
{
  unsigned char ucTry;
  unsigned char ucPrioDelay = LN_BACKOFF_INITIAL;
  LN_STATUS enReturn;
  unsigned char ucWaitForEnterBackoff;

  for (ucTry = 0; ucTry < LN_TX_RETRIES_MAX; ucTry++)
  {

    // wait previous traffic and than prio delay and than try tx
    ucWaitForEnterBackoff = 1;  // don't want to abort do/while loop before
    do                          // we did not see the backoff state once
    {
      enReturn = sendLocoNetPacketTry(pPacket, ucPrioDelay);

      if (enReturn == LN_DONE)  // success?
        return LN_DONE;

      if (enReturn == LN_PRIO_BACKOFF)
        ucWaitForEnterBackoff = 0; // now entered backoff -> next state != LN_BACKOFF is worth incrementing the try counter
    }
    while ((enReturn == LN_CD_BACKOFF) ||                             // waiting CD backoff
    (enReturn == LN_PRIO_BACKOFF) ||                           // waiting master+prio backoff
    ((enReturn == LN_NETWORK_BUSY) && ucWaitForEnterBackoff)); // or within any traffic unfinished
    // failed -> next try going to higher prio = smaller prio delay
    if (ucPrioDelay > LN_BACKOFF_MIN)
      ucPrioDelay--;
  }
  LnBuffer.Stats.TxErrors++ ;
  return LN_RETRY_ERROR;
}

LN_STATUS LocoNetClass::send(lnMsg *pPacket, uint8_t ucPrioDelay)
{
  return sendLocoNetPacketTry(pPacket, ucPrioDelay);
}

LN_STATUS LocoNetClass::send( uint8_t OpCode, uint8_t Data1, uint8_t Data2 )
{
  lnMsg SendPacket ;

  SendPacket.data[ 0 ] = OpCode ;
  SendPacket.data[ 1 ] = Data1 ;
  SendPacket.data[ 2 ] = Data2 ;

  return send( &SendPacket ) ;
}

LN_STATUS LocoNetClass::send( uint8_t OpCode, uint8_t Data1, uint8_t Data2, uint8_t PrioDelay )
{
  lnMsg SendPacket ;

  SendPacket.data[ 0 ] = OpCode ;
  SendPacket.data[ 1 ] = Data1 ;
  SendPacket.data[ 2 ] = Data2 ;

  return sendLocoNetPacketTry( &SendPacket, PrioDelay ) ;
}

uint8_t LocoNetClass::processSwitchSensorMessage( lnMsg *LnPacket )
{
  uint16_t Address ;
  uint8_t  Direction ;
  uint8_t  Output ;
  uint8_t  ConsumedFlag = 1 ;

  Address = (LnPacket->srq.sw1 | ( ( LnPacket->srq.sw2 & 0x0F ) << 7 )) ;
  if( LnPacket->sr.command != OPC_INPUT_REP )
    Address++;

  switch( LnPacket->sr.command )
  {
  case OPC_INPUT_REP:
    Address <<= 1 ;
    Address += ( LnPacket->ir.in2 & OPC_INPUT_REP_SW ) ? 2 : 1 ;

    if(notifySensor)
      notifySensor( Address, LnPacket->ir.in2 & OPC_INPUT_REP_HI ) ;
    break ;

  case OPC_SW_REQ:
    if(notifySwitchRequest)
      notifySwitchRequest( Address, LnPacket->srq.sw2 & OPC_SW_REQ_OUT, LnPacket->srq.sw2 & OPC_SW_REQ_DIR ) ;
    break ;

  case OPC_SW_REP:
    if(notifySwitchReport)
      notifySwitchReport( Address, LnPacket->srp.sn2 & OPC_SW_REP_HI, LnPacket->srp.sn2 & OPC_SW_REP_SW ) ;
    break ;

  case OPC_SW_STATE:
    Direction = LnPacket->srq.sw2 & OPC_SW_REQ_DIR ;
    Output = LnPacket->srq.sw2 & OPC_SW_REQ_OUT ;

    if(notifySwitchState)
      notifySwitchState( Address, Output, Direction ) ;
    break;

  case OPC_SW_ACK:
    break ;

  case OPC_LONG_ACK:
    if( LnPacket->lack.opcode == (OPC_SW_STATE & 0x7F ) )
    {
      Direction = LnPacket->lack.ack1 & 0x01 ;
    }
    else
      ConsumedFlag = 0 ;
    break;

  default:
    ConsumedFlag = 0 ;
  }

  return ConsumedFlag ;
}

void LocoNetClass::requestSwitch( uint16_t Address, uint8_t Output, uint8_t Direction )
{
  uint8_t AddrH = (--Address >> 7) & 0x0F ;
  uint8_t AddrL = Address & 0x7F ;

  if( Output )
    AddrH |= OPC_SW_REQ_OUT ;

  if( Direction )
    AddrH |= OPC_SW_REQ_DIR ;

  send( OPC_SW_REQ, AddrL, AddrH ) ;
}

void LocoNetClass::reportSwitch( uint16_t Address )
{
  Address -= 1;
  send( OPC_SW_STATE, (Address & 0x7F), ((Address >> 7) & 0x0F) ) ;
}

LocoNetClass LocoNet = LocoNetClass();

// LocoNet Throttle Support


// To make it easier to handle the Speed steps 0 = Stop, 1 = Em Stop and 2 -127
// normal speed steps we will swap speed steps 0 and 1 so that the normal
// range for speed steps is Stop = 1 2-127 is normal as before and now 0 = EmStop
static uint8_t SwapSpeedZeroAndEmStop( uint8_t Speed )
{
  if( Speed == 0 )
    return 1 ;

  if( Speed == 1 )
    return 0 ;

  return Speed ;
}

void LocoNetThrottleClass::updateAddress(uint16_t Address, uint8_t ForceNotify )
{
  if( ForceNotify || myAddress != Address )
  {
    myAddress = Address ;
    if(notifyThrottleAddress)
		notifyThrottleAddress( myUserData, myState, Address, mySlot ) ;
  }
}

void LocoNetThrottleClass::updateSpeed(uint8_t Speed, uint8_t ForceNotify )
{
  if( ForceNotify || mySpeed != Speed )
  {
    mySpeed = Speed ;
	if(notifyThrottleSpeed)
		notifyThrottleSpeed( myUserData, myState, SwapSpeedZeroAndEmStop( Speed ) ) ;
  }
}

void LocoNetThrottleClass::updateState(TH_STATE State, uint8_t ForceNotify )
{
  TH_STATE  PrevState ;

  if( ForceNotify || myState != State )
  {
    PrevState = myState ;
    myState = State ;
	if(notifyThrottleState)
		notifyThrottleState( myUserData, PrevState, State ) ;
  }
}

void LocoNetThrottleClass::updateStatus1(uint8_t Status, uint8_t ForceNotify )
{
  register uint8_t Mask ;	// Temporary uint8_t Variable for bitwise AND to force
  // the compiler to only do 8 bit operations not 16

  if( ForceNotify || myStatus1 != Status )
  {
    myStatus1 = Status ;
	if(notifyThrottleSlotStatus)
		notifyThrottleSlotStatus( myUserData, Status ) ;

    Mask = LOCO_IN_USE ;
    updateState( ( ( Status & Mask ) == Mask ) ? TH_ST_IN_USE : TH_ST_FREE, ForceNotify ) ;
  }
}

void LocoNetThrottleClass::updateDirectionAndFunctions(uint8_t DirFunc0to4, uint8_t ForceNotify )
{
  uint8_t Diffs ;
  uint8_t Function ;
  uint8_t Mask ;

  if( ForceNotify || myDirFunc0to4 != DirFunc0to4 )
  {
    Diffs = myDirFunc0to4 ^ DirFunc0to4 ;
    myDirFunc0to4 = DirFunc0to4 ;

    // Check Functions 1-4
    for( Function = 1, Mask = 1; Function <= 4; Function++ )
    {
      if( notifyThrottleFunction && (ForceNotify || Diffs & Mask ))
        notifyThrottleFunction( myUserData, Function, DirFunc0to4 & Mask ) ;

      Mask <<= 1 ;
    }

    // Check Functions 0
    if( notifyThrottleFunction && ( ForceNotify || Diffs & DIRF_F0 ))
      notifyThrottleFunction( myUserData, 0, DirFunc0to4 & (uint8_t)DIRF_F0 ) ;

    // Check Direction
    if( notifyThrottleDirection && (ForceNotify || Diffs & DIRF_DIR ))
      notifyThrottleDirection( myUserData, myState, DirFunc0to4 & (uint8_t)DIRF_DIR ) ;
  }
}

void LocoNetThrottleClass::updateFunctions5to8(uint8_t Func5to8, uint8_t ForceNotify )
{
  uint8_t Diffs ;
  uint8_t Function ;
  uint8_t Mask ;

  if( ForceNotify || myFunc5to8 != Func5to8 )
  {
    Diffs = myFunc5to8 ^ Func5to8 ;
    myFunc5to8 = Func5to8 ;

    // Check Functions 5-8
    for( Function = 5, Mask = 1; Function <= 8; Function++ )
    {
      if( notifyThrottleFunction && (ForceNotify || Diffs & Mask ))
        notifyThrottleFunction( myUserData, Function, Func5to8 & Mask ) ;

      Mask <<= 1 ;
    }
  }
}

#define SLOT_REFRESH_TICKS   		600   // 600 * 100ms = 60 seconds between speed refresh

void LocoNetThrottleClass::process100msActions(void)
{
  if( myState == TH_ST_IN_USE )
  {
	myTicksSinceLastAction++ ;

	if( ( myDeferredSpeed ) || ( myTicksSinceLastAction > SLOT_REFRESH_TICKS ) )
	{
	  LocoNet.send( OPC_LOCO_SPD, mySlot, ( myDeferredSpeed ) ? myDeferredSpeed : mySpeed ) ;

	  if( myDeferredSpeed )
		myDeferredSpeed = 0 ;

	  myTicksSinceLastAction = 0 ;
	}
  }
}

void LocoNetThrottleClass::init(uint8_t UserData, uint8_t Options, uint16_t ThrottleId )
{
  myState = TH_ST_FREE ;
  myThrottleId = ThrottleId ;
  myDeferredSpeed = 0 ;
  myUserData = UserData ;
  myOptions = Options ;
}

void LocoNetThrottleClass::processMessage(lnMsg *LnPacket )
{
  uint8_t  Data2 ;
  uint16_t  SlotAddress ;

  // Update our copy of slot information if applicable
  if( LnPacket->sd.command == OPC_SL_RD_DATA )
  {
    SlotAddress = (uint16_t) (( LnPacket->sd.adr2 << 7 ) + LnPacket->sd.adr ) ;

    if( mySlot == LnPacket->sd.slot )
    {
      // Make sure that the slot address matches even though we have the right slot number
      // as it is possible that another throttle got in before us and took our slot.
      if( myAddress == SlotAddress )
      {
        if(	( myState == TH_ST_SLOT_RESUME ) &&
          ( myThrottleId != (uint16_t)( ( LnPacket->sd.id2 << 7 ) + LnPacket->sd.id1 ) ) )
        {
          updateState( TH_ST_FREE, 1 ) ;
          if(notifyThrottleError)
			notifyThrottleError( myUserData, TH_ER_NO_LOCO ) ;
        }

        updateState( TH_ST_IN_USE, 1 ) ;
        updateAddress( SlotAddress, 1 ) ;
        updateSpeed( LnPacket->sd.spd, 1 ) ;
        updateDirectionAndFunctions( LnPacket->sd.dirf, 1 ) ;
        updateFunctions5to8( LnPacket->sd.snd, 1 ) ;
        updateStatus1( LnPacket->sd.stat, 1 ) ;

        // We need to force a State update to cause a display refresh once all data is known
        updateState( TH_ST_IN_USE, 1 ) ;

        // Now Write our own Throttle Id to the slot and write it back to the command station
        LnPacket->sd.command = OPC_WR_SL_DATA ;
        LnPacket->sd.id1 = (uint8_t) ( myThrottleId & 0x7F ) ;
        LnPacket->sd.id2 = (uint8_t) ( myThrottleId >> 7 );
        // jmg Loconet.Send( LnPacket ) ;
      }
      // Ok another throttle did a NULL MOVE with the same slot before we did
      // so we have to try again
      else if( myState == TH_ST_SLOT_MOVE )
      {
        updateState( TH_ST_SELECT, 1 ) ;
        LocoNet.send( OPC_LOCO_ADR, (uint8_t) ( myAddress >> 7 ), (uint8_t) ( myAddress & 0x7F ) ) ;
      }
    }
    // Slot data is not for one of our slots so check if we have requested a new addres
    else
    {
      if( myAddress == SlotAddress )
      {
        if( ( myState == TH_ST_SELECT ) || ( myState == TH_ST_DISPATCH ) )
        {
          if( ( LnPacket->sd.stat & STAT1_SL_CONUP ) == 0 &&
            ( LnPacket->sd.stat & LOCO_IN_USE ) != LOCO_IN_USE )
          {
            if( myState == TH_ST_SELECT )
            {
              updateState( TH_ST_SLOT_MOVE, 1 ) ;
              mySlot = LnPacket->sd.slot ;
              Data2 = LnPacket->sd.slot ;
            }
            else
            {
              updateState( TH_ST_FREE, 1 ) ;
              Data2 = 0 ;
            }

            LocoNet.send( OPC_MOVE_SLOTS, LnPacket->sd.slot, Data2 ) ;
          }
          else
          {
            if(notifyThrottleError)
				notifyThrottleError( myUserData, TH_ER_SLOT_IN_USE ) ;
            updateState( TH_ST_FREE, 1 ) ;
          }
        }
        else
        {
          if( myState == TH_ST_SLOT_FREE )
          {
            LocoNet.send( OPC_SLOT_STAT1, LnPacket->sd.slot, (uint8_t) ( myStatus1 & ~STAT1_SL_BUSY ) ) ;
            mySlot = 0xFF ;
            updateState( TH_ST_FREE, 1 ) ;
          }
        }
      }

      if( myState == TH_ST_ACQUIRE )
      {
        mySlot = LnPacket->sd.slot ;
        updateState( TH_ST_IN_USE, 1 ) ;

        updateAddress( SlotAddress, 1 ) ;
        updateSpeed( LnPacket->sd.spd, 1 ) ;
        updateDirectionAndFunctions( LnPacket->sd.dirf, 1 ) ;
        updateStatus1( LnPacket->sd.stat, 1 ) ;
      }
    }
  }

  else if( ( ( LnPacket->sd.command >= OPC_LOCO_SPD ) && ( LnPacket->sd.command <= OPC_LOCO_SND ) ) ||
    ( LnPacket->sd.command == OPC_SLOT_STAT1 ) )
  {
    if( mySlot == LnPacket->ld.slot )
    {
      if( LnPacket->ld.command == OPC_LOCO_SPD )
        updateSpeed( LnPacket->ld.data, 0 ) ;

      else if( LnPacket->ld.command == OPC_LOCO_DIRF )
        updateDirectionAndFunctions( LnPacket->ld.data, 0 ) ;

      else if( LnPacket->ld.command == OPC_LOCO_SND )
        updateFunctions5to8( LnPacket->ld.data, 0 ) ;

      else if( LnPacket->ld.command == OPC_SLOT_STAT1 )
        updateStatus1( LnPacket->ld.data, 0 ) ;
    }
  }
  else if( LnPacket->lack.command == OPC_LONG_ACK )
  {
    if( ( myState >= TH_ST_ACQUIRE ) && ( myState <= TH_ST_SLOT_MOVE ) )
    {
      if( LnPacket->lack.opcode == ( OPC_MOVE_SLOTS & 0x7F ) )
        if(notifyThrottleError)
			notifyThrottleError( myUserData, TH_ER_NO_LOCO ) ;

      if( LnPacket->lack.opcode == ( OPC_LOCO_ADR & 0x7F ) )
        if( notifyThrottleError)
			notifyThrottleError( myUserData, TH_ER_NO_SLOTS ) ;

      updateState( TH_ST_FREE, 1 ) ;
    }
  }
}

uint16_t LocoNetThrottleClass::getAddress(void)
{
  return myAddress ;
}

TH_ERROR LocoNetThrottleClass::setAddress(uint16_t Address )
{
  if( myState == TH_ST_FREE )
  {
    updateAddress( Address, 1 ) ;
    updateState( TH_ST_SELECT, 1 ) ;

    LocoNet.send( OPC_LOCO_ADR, (uint8_t) ( Address >> 7 ), (uint8_t) ( Address & 0x7F ) ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_BUSY ) ;
  return TH_ER_BUSY ;
}

TH_ERROR LocoNetThrottleClass::resumeAddress(uint16_t Address, uint8_t LastSlot )
{
  if( myState == TH_ST_FREE )
  {
    mySlot = LastSlot ;
    updateAddress( Address, 1 ) ;
    updateState( TH_ST_SLOT_RESUME, 1 ) ;

    LocoNet.send( OPC_RQ_SL_DATA, LastSlot, 0 ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_BUSY ) ;
  return TH_ER_BUSY ;
}

TH_ERROR LocoNetThrottleClass::freeAddress(uint16_t Address )
{
  if( myState == TH_ST_FREE )
  {
    updateAddress( Address, 1 ) ;
    updateState( TH_ST_SLOT_FREE, 1 ) ;

    LocoNet.send( OPC_LOCO_ADR, (uint8_t) ( Address >> 7 ), (uint8_t) ( Address & 0x7F ) ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_BUSY ) ;
  return TH_ER_BUSY ;
}


TH_ERROR LocoNetThrottleClass::dispatchAddress(uint16_t Address )
{
  if( myState == TH_ST_FREE)
  {
    updateAddress( Address, 1 ) ;
    updateState( TH_ST_DISPATCH, 1 ) ;

    LocoNet.send( OPC_LOCO_ADR, (uint8_t) ( Address >> 7 ), (uint8_t) ( Address & 0x7F ) ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_BUSY ) ;
  return TH_ER_BUSY ;
}

TH_ERROR LocoNetThrottleClass::acquireAddress(void)
{
  if( myState == TH_ST_FREE )
  {
    updateState( TH_ST_ACQUIRE, 1 ) ;

    LocoNet.send( OPC_MOVE_SLOTS, 0, 0 ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_BUSY ) ;
  return TH_ER_BUSY ;
}

void LocoNetThrottleClass::releaseAddress(void)
{
  if( myState == TH_ST_IN_USE )
  {
    LocoNet.send( OPC_SLOT_STAT1, mySlot, (uint8_t) ( myStatus1 & ~STAT1_SL_BUSY ) ) ;
  }

  mySlot = 0xFF ;
  updateState( TH_ST_FREE, 1 ) ;
}

uint8_t LocoNetThrottleClass::getSpeed(void)
{
  return SwapSpeedZeroAndEmStop( mySpeed ) ;
}

TH_ERROR LocoNetThrottleClass::setSpeed(uint8_t Speed )
{
  if( myState == TH_ST_IN_USE )
  {
    Speed = SwapSpeedZeroAndEmStop( Speed ) ;

    if( mySpeed != Speed )
    {
      // Always defer any speed other than stop or em stop
      if( (myOptions & TH_OP_DEFERRED_SPEED) &&
        ( ( Speed > 1 ) || (myTicksSinceLastAction == 0 ) ) )
        myDeferredSpeed = Speed ;
      else
      {
        LocoNet.send( OPC_LOCO_SPD, mySlot, Speed ) ;
        myTicksSinceLastAction = 0 ;
        myDeferredSpeed = 0 ;
      }
    }
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_NOT_SELECTED ) ;
  return TH_ER_NOT_SELECTED ;
}

uint8_t LocoNetThrottleClass::getDirection(void)
{
  return myDirFunc0to4 & (uint8_t)DIRF_DIR ;
}

TH_ERROR LocoNetThrottleClass::setDirection(uint8_t Direction )
{
  if( myState == TH_ST_IN_USE )
  {
    LocoNet.send( OPC_LOCO_DIRF, mySlot,
    ( Direction ) ? (uint8_t) ( myDirFunc0to4 | DIRF_DIR ) : (uint8_t) ( myDirFunc0to4 & ~DIRF_DIR ) ) ;

    myTicksSinceLastAction = 0 ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_NOT_SELECTED ) ;
  return TH_ER_NOT_SELECTED ;
}

uint8_t LocoNetThrottleClass::getFunction(uint8_t Function )
{
  uint8_t Mask ;

  if( Function <= 4 )
  {
    Mask = (uint8_t) (1 << ((Function) ? Function - 1 : 4 )) ;
    return myDirFunc0to4 & Mask ;
  }

  Mask = (uint8_t) (1 << (Function - 5)) ;
  return myFunc5to8 & Mask ;
}

TH_ERROR LocoNetThrottleClass::setFunction(uint8_t Function, uint8_t Value )
{
  uint8_t Mask ;
  uint8_t OpCode ;
  uint8_t Data ;

  if( myState == TH_ST_IN_USE )
  {
    if( Function <= 4 )
    {
      OpCode = OPC_LOCO_DIRF ;
      Data = myDirFunc0to4 ;
      Mask = (uint8_t)(1 << ((Function) ? Function - 1 : 4 )) ;
    }
    else
    {
      OpCode = OPC_LOCO_SND ;
      Data = myFunc5to8 ;
      Mask = (uint8_t)(1 << (Function - 5)) ;
    }

    if( Value )
      Data |= Mask ;
    else
      Data &= (uint8_t)~Mask ;

    LocoNet.send( OpCode, mySlot, Data ) ;

    myTicksSinceLastAction = 0 ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_NOT_SELECTED ) ;
  return TH_ER_NOT_SELECTED ;
}

TH_ERROR LocoNetThrottleClass::setDirFunc0to4Direct(uint8_t Value )
{
  if( myState == TH_ST_IN_USE )
  {
    LocoNet.send( OPC_LOCO_DIRF, mySlot, Value & 0x7F ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_NOT_SELECTED ) ;
  return TH_ER_NOT_SELECTED ;
}

TH_ERROR LocoNetThrottleClass::setFunc5to8Direct(uint8_t Value )
{
  if( myState == TH_ST_IN_USE )
  {
    LocoNet.send( OPC_LOCO_SND, mySlot, Value & 0x7F ) ;
    return TH_ER_OK ;
  }

  if(notifyThrottleError)
	notifyThrottleError( myUserData, TH_ER_NOT_SELECTED ) ;
  return TH_ER_NOT_SELECTED ;
}

TH_STATE LocoNetThrottleClass::getState(void)
{
  return myState ;
}


const char *LocoNetThrottleClass::getStateStr( TH_STATE State )
{
  switch( State )
  {
  case TH_ST_FREE:
    return "Free" ;

  case TH_ST_ACQUIRE:
    return "Acquire" ;

  case TH_ST_SELECT:
    return "Select" ;

  case TH_ST_DISPATCH:
    return "Dispatch" ;

  case TH_ST_SLOT_MOVE:
    return "Slot Move" ;

  case TH_ST_SLOT_FREE:
    return "Slot Free" ;

  case TH_ST_IN_USE:
    return "In Use" ;

  default:
    return "Unknown" ;
  }
}

const char *LocoNetThrottleClass::getErrorStr( TH_ERROR Error )
{
  switch( Error )
  {
  case TH_ER_OK:

    return "Ok" ;

  case TH_ER_SLOT_IN_USE:

    return "In Use" ;

  case TH_ER_BUSY:

    return "Busy" ;

  case TH_ER_NOT_SELECTED:

    return "Not Sel" ;

  case TH_ER_NO_LOCO:

    return "No Loco" ;

  case TH_ER_NO_SLOTS:

    return "No Free Slots" ;

  default:

    return "Unknown" ;
  }
}

#define FC_FLAG_DCS100_COMPATIBLE_SPEED	0x01
#define FC_FLAG_MINUTE_ROLLOVER_SYNC	0x02
#define FC_FLAG_NOTIFY_FRAC_MINS_TICK	0x04
#define FC_FRAC_MIN_BASE   				0x3FFF
#define FC_FRAC_RESET_HIGH	 			0x78
#define FC_FRAC_RESET_LOW 	 			0x6D
#define FC_TIMER_TICKS         			65        // 65ms ticks
#define FC_TIMER_TICKS_REQ	  			250        // 250ms waiting for Response to FC Req

void LocoNetFastClockClass::init(uint8_t DCS100CompatibleSpeed, uint8_t CorrectDCS100Clock, uint8_t NotifyFracMin)
{
  fcState = FC_ST_IDLE ;
  
  fcFlags = 0;
  if(DCS100CompatibleSpeed)
	fcFlags |= FC_FLAG_DCS100_COMPATIBLE_SPEED ;

  if(CorrectDCS100Clock)
	fcFlags |= FC_FLAG_MINUTE_ROLLOVER_SYNC ;

  if(NotifyFracMin)
	fcFlags |= FC_FLAG_NOTIFY_FRAC_MINS_TICK ;
}

void LocoNetFastClockClass::poll(void)
{
  LocoNet.send( OPC_RQ_SL_DATA, FC_SLOT, 0 ) ;
}

void LocoNetFastClockClass::doNotify( uint8_t Sync )
{
  if(notifyFastClock)
	notifyFastClock(fcSlotData.clk_rate, fcSlotData.days,
	  (fcSlotData.hours_24 >= (128-24)) ? fcSlotData.hours_24 - (128-24) : fcSlotData.hours_24 % 24 ,
	  fcSlotData.mins_60 - (127-60 ), Sync ) ;
}

void LocoNetFastClockClass::processMessage( lnMsg *LnPacket )
{
  if( ( LnPacket->fc.slot == FC_SLOT ) && ( ( LnPacket->fc.command == OPC_WR_SL_DATA ) || ( LnPacket->fc.command == OPC_SL_RD_DATA ) ) )
  {
    if( LnPacket->fc.clk_cntrl & 0x40 )
    {
      if( fcState >= FC_ST_REQ_TIME )
      {
		memcpy( &fcSlotData, &LnPacket->fc, sizeof( fastClockMsg ) ) ; 
				
        doNotify( 1 ) ;

		if( fcFlags & FC_FLAG_NOTIFY_FRAC_MINS_TICK )
		  notifyFastClockFracMins( FC_FRAC_MIN_BASE - ( ( fcSlotData.frac_minsh << 7 ) + fcSlotData.frac_minsl ) );

        fcState = FC_ST_READY ;
      }
    }
    else
      fcState = FC_ST_DISABLED ;
  }
}

void LocoNetFastClockClass::process66msActions(void)
{
		// If we are all initialised and ready then increment accumulators
  if( fcState == FC_ST_READY )
  {
    fcSlotData.frac_minsl +=  fcSlotData.clk_rate ;
    if( fcSlotData.frac_minsl & 0x80 )
    {
      fcSlotData.frac_minsl &= ~0x80 ;

      fcSlotData.frac_minsh++ ;
      if( fcSlotData.frac_minsh & 0x80 )
      {
					// For the next cycle prime the fraction of a minute accumulators
        fcSlotData.frac_minsl = FC_FRAC_RESET_LOW ;
				
					// If we are in FC_FLAG_DCS100_COMPATIBLE_SPEED mode we need to run faster
					// by reducong the FRAC_MINS duration count by 128
        fcSlotData.frac_minsh = FC_FRAC_RESET_HIGH + (fcFlags & FC_FLAG_DCS100_COMPATIBLE_SPEED) ;

        fcSlotData.mins_60++;
        if( fcSlotData.mins_60 >= 0x7F )
        {
          fcSlotData.mins_60 = 127 - 60 ;

          fcSlotData.hours_24++ ;
          if( fcSlotData.hours_24 & 0x80 )
          {
            fcSlotData.hours_24 = 128 - 24 ;

            fcSlotData.days++;
          }
        }
				
			// We either send a message out onto the LocoNet to change the time,
			// which we will also see and act on or just notify our user
			// function that our internal time has changed.
		if( fcFlags & FC_FLAG_MINUTE_ROLLOVER_SYNC )
		{
			fcSlotData.command = OPC_WR_SL_DATA ;
			LocoNet.send((lnMsg*)&fcSlotData) ;
		}
		else
			doNotify( 0 ) ;
      }
    }

	if( notifyFastClockFracMins && (fcFlags & FC_FLAG_NOTIFY_FRAC_MINS_TICK ))
	  notifyFastClockFracMins( FC_FRAC_MIN_BASE - ( ( fcSlotData.frac_minsh << 7 ) + fcSlotData.frac_minsl ) ) ;
  }

  if( fcState == FC_ST_IDLE )
  {
    LocoNet.send( OPC_RQ_SL_DATA, FC_SLOT, 0 ) ;
    fcState = FC_ST_REQ_TIME ;
  }
}
