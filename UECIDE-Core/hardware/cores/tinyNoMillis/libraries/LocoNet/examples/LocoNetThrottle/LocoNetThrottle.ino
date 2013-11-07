/****************************************************************************
    Copyright (C) 2002,2003,2004 Alex Shepherd

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

 Title :   LocoNet Infrared TV/VCR Remote controlled Throttle
 Author:   Alex Shepherd <kiwi64ajs@sourceforge.net>
 Date:     4-Dec-2004
 Software: AVR-GCC
 Target:   AtMega8

 DESCRIPTION
    This project is a LocoNet throttle which gets it commands from a TV/VCR
	Infrared remote control. Initially this will only work with the Sony SIRCS
	remotes as this is what I have several of but support for others will
	probably be added.
	
*****************************************************************************/

#include <LocoNet.h>
#include <BasicTerm.h>
#include <ctype.h>

// Address Recall Stack Length
#define RECALL_STACK_LEN  6
#define AJS_RECALL_STACK  { 38, 44, 53, 99, 191, 192 }

typedef struct
{
	word	RecallStack[ RECALL_STACK_LEN ] ;
	word	LastAddress ;
	byte	LastSlot ;
} TH_EE_CONFIG ;

TH_EE_CONFIG		eeConfig __attribute__((section(".eeprom")))
	= { AJS_RECALL_STACK, 0, 0xFF };
	
LocoNetThrottleClass  Throttle ;
word                  LocoAddr ;
byte                  RecallIndex ;
LnBuf                 LnRxBuffer ;
lnMsg                 *RxPacket ;
byte                  i ;
byte                  tByte ;
uint32_t              LastThrottleTimerTick;

BasicTerm Term(&Serial);

void DrawStaticText(void)
{
  Term.show_cursor(0);
  Term.cls();
  Term.position(0,0);
  Term.println(F("LocoNet Throttle Library Demonstration Version: 1"));
  Term.println();
  Term.println(F("Address    :"));
  Term.println();
  Term.println(F("Speed      :"));
  Term.println();
  Term.println(F("Direction  :"));
  Term.println();
  Term.println(F("Functions  : 0 1 2 3 4 5 6 7 8"));
//                        1         2         3             
//              0123456789012345678901234567890  
  Term.println();
  Term.println("Status     :");
  Term.println();
  Term.println("Error      :");
  Term.println();
  Term.println("Last Key   :");
  Term.println();
  Term.println(F("Keys: A    - Enter Loco Address, <BS> to delete, <ENTER> to select"));
  Term.println(F("Keys: Q    - Release Loco Address"));
  Term.println(F("Keys: [    - Reduce Speed"));
  Term.println(F("Keys: ]    - Increase Speed"));
  Term.println(F("Keys: F    - Forward"));
  Term.println(F("Keys: R    - Reverse"));
  Term.println(F("Keys: T    - Toggle Direction"));
  Term.println(F("Keys: <SP> - Stop"));
  Term.println(F("Keys: 0..8 - Toggle Functions 0..8"));
}

void notifyThrottleAddress( uint8_t UserData, TH_STATE State, uint16_t Address, uint8_t Slot )
{
  Term.position(2,13);
  Term.print(Address);
  Term.print("     "); // Erase any extra chars
};

void notifyThrottleSpeed( uint8_t UserData, TH_STATE State, uint8_t Speed )
{
  Term.position(4,13);
  Term.print(Speed);
  Term.print("     "); // Erase any extra chars
};

void notifyThrottleDirection( uint8_t UserData, TH_STATE State, uint8_t Direction )
{
  Term.position(6,13);
  Term.print(Direction ? "Reverse" : "Forward");
};

void notifyThrottleFunction( uint8_t UserData, uint8_t Function, uint8_t Value )
{
  Term.position(8,13 + (Function * 2) );
  if(Value)
    Term.set_attribute(BT_BOLD);
    
  Term.print(Function, DEC);

  if(Value)
    Term.set_attribute(BT_NORMAL);
};

void notifyThrottleSlotStatus( uint8_t UserData, uint8_t Status ){};

void notifyThrottleState( uint8_t UserData, TH_STATE PrevState, TH_STATE State )
{
  Term.position(10, 13);
  Term.print(State, DEC);
  Term.print(' ');
  Term.print(Throttle.getStateStr(State));
  Term.print("                   ");
}

void notifyThrottleError( uint8_t UserData, TH_ERROR Error )
{
  Term.position(12, 13);
  Term.print(Error, DEC);
  Term.print(' ');
  Term.print(Throttle.getErrorStr(Error));
  Term.print("                   ");
}

void setup()
{
  // First initialize the LocoNet interface
  LocoNet.init();

  // Configure the serial port for 57600 baud
  Serial.begin(115200);
  Term.init();
  DrawStaticText();
  
  Throttle.init(0, 0, 9999);
}

boolean isTime(unsigned long *timeMark, unsigned long timeInterval)
{
    unsigned long timeNow = millis();
    if ( timeNow - *timeMark >= timeInterval) {
        *timeMark = timeNow;
        return true;
    }    
    return false;
}

void loop()
{  
  // Check for any received LocoNet packets
  RxPacket = LocoNet.receive() ;
  if( RxPacket )
  {
    digitalWrite(13, LOW);
    
    if( !LocoNet.processSwitchSensorMessage(RxPacket) )
      Throttle.processMessage(RxPacket) ; 
  }
  
  if( Serial.available())
  {
    int16_t inChar = toupper(Serial.read());
    Term.position(14,13);
    Term.print(inChar);
    switch(inChar){
      case 'A': Throttle.setAddress(1234);
                break;
      case 'X': DrawStaticText();
                Throttle.freeAddress(1234);
                break;
      case 'Q': DrawStaticText();
                Throttle.releaseAddress(); 
                break;
      case 'F': Throttle.setDirection(0); 
                break;
      case 'R': Throttle.setDirection(1); 
                break;
      case 'T': Throttle.setDirection(!Throttle.getDirection());
                break;
      case '[': if(Throttle.getSpeed() > 0 )
                  Throttle.setSpeed(Throttle.getSpeed() - 1);
                break;
      case ']': if(Throttle.getSpeed() < 127 )
                  Throttle.setSpeed(Throttle.getSpeed() + 1);
                break;
      case ' ': Throttle.setSpeed(0); break;
      default:  if( (inChar >= '0') && (inChar <= '8'))
                  Throttle.setFunction( inChar - '0', !Throttle.getFunction(inChar - '0'));
                break;
    }
  }
  
  if(isTime(&LastThrottleTimerTick, 100))
  {
    Throttle.process100msActions() ; 
    digitalWrite(13, HIGH);
  }
}
