#include <TinyWireM.h>
#define expander 0x20

byte expanderStatus = B11111111; //all off

void setup()
{
    TinyWireM.begin();
}

void loop()
{
    expanderWrite(0,HIGH);
    delay(1000);
    expanderWrite(0,LOW);
    delay(1000);
}


void expanderWrite(byte pinNumber, boolean state){
  if(state == HIGH)
    expanderStatus &= ~(1 << pinNumber);
  else
    expanderStatus |= (1 << pinNumber); 
    
  expanderWrite(expanderStatus);
}

void expanderWrite(byte _data ) {
  TinyWireM.beginTransmission(expander);
  TinyWireM.send(_data);
  TinyWireM.endTransmission();
}
