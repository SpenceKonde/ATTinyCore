#ifndef LOCONET_INCLUDED
#define LOCONET_INCLUDED

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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "utility/ln_buf.h"

typedef enum
{
	LN_CD_BACKOFF = 0,
	LN_PRIO_BACKOFF,
	LN_NETWORK_BUSY,
	LN_DONE,
	LN_COLLISION,
	LN_UNKNOWN_ERROR,
	LN_RETRY_ERROR
} LN_STATUS ;

// CD Backoff starts after the Stop Bit (Bit 9) and has a minimum or 20 Bit Times
// but initially starts with an additional 20 Bit Times 
#define   LN_CARRIER_TICKS      20  // carrier detect backoff - all devices have to wait this
#define   LN_MASTER_DELAY        6  // non master devices have to wait this additionally
#define   LN_INITIAL_PRIO_DELAY 20  // initial attempt adds priority delay
#define   LN_BACKOFF_MIN      (LN_CARRIER_TICKS + LN_MASTER_DELAY)      // not going below this
#define   LN_BACKOFF_INITIAL  (LN_BACKOFF_MIN + LN_INITIAL_PRIO_DELAY)  // for the first normal tx attempt
#define   LN_BACKOFF_MAX      (LN_BACKOFF_INITIAL + 10)                 // lower priority is not supported

class LocoNetClass
{
  private:
    LnBuf   LnBuffer ;
	void 		setTxPin(uint8_t txPin);

  public:
    LocoNetClass();
    void        init(void);
    void        init(uint8_t txPin);
    lnMsg*      receive(void);
    LN_STATUS   send(lnMsg *TxPacket);
    LN_STATUS   send(lnMsg *TxPacket, uint8_t PrioDelay);
    LN_STATUS   send( uint8_t OpCode, uint8_t Data1, uint8_t Data2 );
    LN_STATUS   send( uint8_t OpCode, uint8_t Data1, uint8_t Data2, uint8_t PrioDelay );
    
    uint8_t processSwitchSensorMessage( lnMsg *LnPacket ) ;

    void requestSwitch( uint16_t Address, uint8_t Output, uint8_t Direction ) ;
    void reportSwitch( uint16_t Address ) ;
};

extern LocoNetClass LocoNet;

typedef enum
{
  TH_ST_FREE   = 0,
  TH_ST_ACQUIRE,
  TH_ST_SELECT,
  TH_ST_DISPATCH,
  TH_ST_SLOT_MOVE,
  TH_ST_SLOT_FREE,
  TH_ST_SLOT_RESUME,
  TH_ST_IN_USE
} TH_STATE ;

typedef enum
{
  TH_ER_OK = 0,
  TH_ER_SLOT_IN_USE,
  TH_ER_BUSY,
  TH_ER_NOT_SELECTED,
  TH_ER_NO_LOCO,
  TH_ER_NO_SLOTS
} TH_ERROR ;

#define TH_OP_DEFERRED_SPEED 0x01

class LocoNetThrottleClass
{
  private:
	TH_STATE	myState ;         // State of throttle
	uint16_t	myTicksSinceLastAction ;
	uint16_t	myThrottleId ;		// Id of throttle
	uint8_t		mySlot ;          // Master Slot index
	uint16_t	myAddress ;       // Decoder Address
	uint8_t		mySpeed ;         // Loco Speed
	uint8_t		myDeferredSpeed ; // Deferred Loco Speed setting
	uint8_t		myStatus1 ;       // Stat1
	uint8_t		myDirFunc0to4 ;   // Direction
	uint8_t		myFunc5to8 ;       // Direction
	uint8_t		myUserData ;
	uint8_t		myOptions ;
	uint32_t 	myLastTimerMillis;
	
	void updateAddress(uint16_t Address, uint8_t ForceNotify );
	void updateSpeed(uint8_t Speed, uint8_t ForceNotify );
	void updateState(TH_STATE State, uint8_t ForceNotify );
	void updateStatus1(uint8_t Status, uint8_t ForceNotify );
	void updateDirectionAndFunctions(uint8_t DirFunc0to4, uint8_t ForceNotify );
	void updateFunctions5to8(uint8_t Func5to8, uint8_t ForceNotify );
  
  public:
	void init(uint8_t UserData, uint8_t Options, uint16_t ThrottleId ) ;

	void processMessage(lnMsg *LnPacket ) ;
	void process100msActions(void);

	uint16_t getAddress(void) ;
	TH_ERROR setAddress(uint16_t Address) ;
	TH_ERROR resumeAddress(uint16_t Address, uint8_t LastSlot) ;
	TH_ERROR dispatchAddress(uint16_t Address) ;
	TH_ERROR acquireAddress(void) ;
	void releaseAddress(void) ;
	TH_ERROR freeAddress(uint16_t Address) ;

	uint8_t getSpeed(void) ;
	TH_ERROR setSpeed(uint8_t Speed) ;

	uint8_t getDirection(void) ;
	TH_ERROR setDirection(uint8_t Direction) ;

	uint8_t getFunction(uint8_t Function) ;
	TH_ERROR setFunction(uint8_t Function, uint8_t Value) ;
	TH_ERROR setDirFunc0to4Direct(uint8_t Value) ;
	TH_ERROR setFunc5to8Direct(uint8_t Value) ;

	TH_STATE getState(void) ;
	const char *getStateStr( TH_STATE State );
	const char *getErrorStr( TH_ERROR Error );
};

/************************************************************************************
	The LocoNet fast clock in the Command Station is driven from a 65.535 ms
    time base. A normal minute takes approximately 915 x 65.535 ms ticks.

	The LocoNet fast clock values are stored in a special slot in the Command
	Station called the fast clock slot which is slot number 0x7B or 123
	
	Each of the fields in the slot are supposed to count up until the most significant bit
	is 0x80 and then rollover the appropriate values and reset however this behaviour
	does not seem to hold for all fields and so some corrction factors are needed

	An important part of syncing to the Fast Clock master is to interpret the current
	FRAC_MINS fields so that a Fast Clock Slave can sync to the part minute and then
	rollover it's accumulators in sync with the master. The FRAC_MINS counter is a
	14 bit counter that is stored in the two 7 bit FRAC_MINSL & FRAC_MINSH fields.
	It counts up the FRAC_MINSL field until it rolls over to 0x80 and then increments
	the FRAC_MINSH high field until it rolls over to 0x80 and then increments the minute,
	hour and day fields as appropriate and then resets the FRAC_MINS fields to 0x4000 - 915
	which is stored in each of the 7 bit fields.
 
	HOWEVER when the DCS100 resets FRAC_MINS fields to 0x4000 - 915, it then immediately
	rolls over a 128 count and so the minute is short by 915 - 128 65.535 ms ticks, so it
	runs too fast. To correct this problem the fast clock slot can be overwritten with
	corrected FRAC_MINS field values that the DCS100 will then increment correctly.

	This implementation of a LocoNet Fast Clock Slave has two features to correct these
	short commings:
	
	A) It has the option to reduce the FRAC_MINS count by 128 so that it keeps in step with
	the DCS100 Fast Clock which normally runs too fast. This is enabled by passing in the
	FC_FLAG_DCS100_COMPATIBLE_SPEED flag bit to the init() function. 
	
	B) It has the option to overwrite the LocoNet Fast Clock Master slot values with corrected
	FRAC_MINS fields imediately after it rolls-over the fast minute, to make the DCS100 not run
	too fast as it normally does.	
	
	There also seems to be problems with the hours field not rolling over correctly from 23
	back to 0 and so there is extra processing to work out the hours when it has rolled over
	to 0x80 or 0x00 by the time the bit 7 is cleared. This seems to cause the DT400 throttle
	problems as well and so when running in FC_FLAG_MINUTE_ROLLOVER_SYNC mode, this should
	be corrected. 

	The DT400 throttle display seems to decode the minutes incorrectly by 1 count and so we
	have to make the same interpretation here which is why there is a 127 and not a 128
    roll-over for the minutes. 
***********************************************************************************************/

typedef enum
{
  FC_ST_IDLE,
  FC_ST_REQ_TIME,
  FC_ST_READY,
  FC_ST_DISABLED,
} FC_STATE ;

class LocoNetFastClockClass
{
  private:
	FC_STATE 		fcState ;			// State of the Fast Clock Slave 
	uint8_t			fcFlags ;			// Storage of the option flags passed into initFastClock()
	fastClockMsg 	fcSlotData ;		// Primary storage for the Fast Clock slot data 
	uint8_t 		fcLastPeriod ;		// Period of last tick so we can alternate between
	
	void doNotify( uint8_t Sync );

  public:
	void init(uint8_t DCS100CompatibleSpeed, uint8_t CorrectDCS100Clock, uint8_t NotifyFracMin);
	void poll(void);
	void processMessage(lnMsg *LnPacket );
	void process66msActions(void);
};

#if defined (__cplusplus)
	extern "C" {
#endif

extern void notifySensor( uint16_t Address, uint8_t State ) __attribute__ ((weak));
extern void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));
extern void notifySwitchReport( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));
extern void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));

// Throttle notify Call-back functions
extern void notifyThrottleAddress( uint8_t UserData, TH_STATE State, uint16_t Address, uint8_t Slot ) __attribute__ ((weak));
extern void notifyThrottleSpeed( uint8_t UserData, TH_STATE State, uint8_t Speed ) __attribute__ ((weak));
extern void notifyThrottleDirection( uint8_t UserData, TH_STATE State, uint8_t Direction ) __attribute__ ((weak));
extern void notifyThrottleFunction( uint8_t UserData, uint8_t Function, uint8_t Value ) __attribute__ ((weak));
extern void notifyThrottleSlotStatus( uint8_t UserData, uint8_t Status ) __attribute__ ((weak));
extern void notifyThrottleError( uint8_t UserData, TH_ERROR Error ) __attribute__ ((weak));
extern void notifyThrottleState( uint8_t UserData, TH_STATE PrevState, TH_STATE State ) __attribute__ ((weak));

// FastClock notify Call-back functions
extern void notifyFastClock( uint8_t Rate, uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Sync ) __attribute__ ((weak));
extern void notifyFastClockFracMins( uint16_t FracMins ) __attribute__ ((weak));

#if defined (__cplusplus)
}
#endif

#endif
