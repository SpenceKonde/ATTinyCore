#include <TinyWireM.h>    
 
#define disk1 0x50    //Address of 24LC256 eeprom chip
int returned = 0; 
void setup(void)
{
  //Serial.begin(9600);
  TinyWireM.begin();  
 
  unsigned int address = 0;
  pinMode(5, OUTPUT);
 
  writeEEPROM(disk1, address, 5);
  returned = readEEPROM(disk1, address);
  
  while(returned>0){
   digitalWrite(5,HIGH);
   delay(500);
   digitalWrite(5,LOW);
   delay(500);
   returned--; 
  }

}
 
void loop(){}
 
void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) 
{
  TinyWireM.beginTransmission(deviceaddress);
  TinyWireM.send((int)(eeaddress >> 8));   // MSB
  TinyWireM.send((int)(eeaddress & 0xFF)); // LSB
  TinyWireM.send(data);
  TinyWireM.endTransmission();
 
  delay(5);
}
 
byte readEEPROM(int deviceaddress, unsigned int eeaddress ) 
{
  byte rdata = 0xFF;
 
  TinyWireM.beginTransmission(deviceaddress);
  TinyWireM.send((int)(eeaddress >> 8));   // MSB
  TinyWireM.send((int)(eeaddress & 0xFF)); // LSB
  TinyWireM.endTransmission();
 
  TinyWireM.requestFrom(deviceaddress,1);
 
  if (TinyWireM.available()) rdata = TinyWireM.receive();
 
  return rdata;
}
