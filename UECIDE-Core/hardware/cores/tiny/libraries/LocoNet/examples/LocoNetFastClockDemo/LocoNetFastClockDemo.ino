#include <LocoNet.h>

// LocoNet FastClock Demo

LocoNetFastClockClass  FastClock ;

static   lnMsg    *LnPacket;
unsigned long     LastFastClockTick;

boolean isTime(unsigned long *timeMark, unsigned long timeInterval)
{
    unsigned long timeNow = millis();
    if ( timeNow - *timeMark >= timeInterval) {
        *timeMark += timeInterval;
        return true;
    }    
    return false;
}

void setup()
{
  Serial.begin(57600);
  Serial.println("LocoNet Fast Clock Demo");
  
  // Initialize the LocoNet interface
  LocoNet.init();

  // Initialize the Fast Clock
  FastClock.init(0, 0, 1);

  // Poll the Current Time from the Command Station
  FastClock.poll();
}

void loop()
{  
    // Check for any received LocoNet packets
  LnPacket = LocoNet.receive() ;
  if( LnPacket )
  {
//    Serial.print('|');
    FastClock.processMessage(LnPacket);
  }
  
  if(isTime(&LastFastClockTick, 67))
    FastClock.process66msActions() ; 
}

void notifyFastClock( uint8_t Rate, uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Sync )
{
  Serial.print("Rate: "); Serial.print(Rate);
  Serial.print(" Day: "); Serial.print(Day);
  Serial.print(" Hour: "); Serial.print(Hour);
  Serial.print(" Min: "); Serial.print(Minute);
  Serial.print(" Sunc: "); Serial.println(Sync);
}

void notifyFastClockFracMins( uint16_t FracMins )
{
//  Serial.print('.');
}

