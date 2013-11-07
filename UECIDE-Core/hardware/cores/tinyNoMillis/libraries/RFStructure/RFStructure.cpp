#include "RFStructure.h"

const byte RFStructure::pins[5] = {9,8,7,5,4};
const byte RFStructure::analogPins[2] = {6,5};

byte RFStructure::checksum () {
  byte checksum = 0;
  for (byte index = 0; index < frame.length-1; index++){
    checksum += frame.data[index].__private_byte_;
  }
  return checksum;
}

RFPacket RFStructure::parity (RFPacket packet) { //using even parity
  byte data = packet.ParityChecker.data<<1; //data starts at bit 1 in the bit field, so this helps the compiler optimise its extraction
  byte parity = 1;
  while (data) {
    data >>= 1; //this should be after parity+=...;, but as we shifted data up by one, we do this first to counteract that.
    parity += (data & 1);
  }
  packet.ParityChecker.parity = parity;
  return packet;
}

const char* RFStructure::writeableFrame() {
  return (const char*)frame.data;
}

boolean RFStructure::reassembleFrame(byte packetIn){
  RFPacket packet;
  packet.__private_byte_ = packetIn;
  byte status = 0;
  switch (packet.PacketIDCheck.ID){
    case IdentifierPacket:
      if (frame.index != 0) {
        goto packetError; //packet was recieved at wrong time, so drop the current frame.
      }
      if (packet.IdentifierPacket.channel != channel) {
        goto packetError; //packet not meant for this channel
      }
      break;
    case ChecksumPacket:
      if (frame.index != frame.length-1) {
        goto packetError; //packet was recieved at wrong time, so drop the current frame.
      }
      if (packet.ChecksumPacket.checksum != (checksum()&0x3F)){
        goto packetError; //checkusm failed, so abandon this packet.
      }
      status = 1; //packet completed successfully.
      break;
    case DataPacket:
      if (packet.ParityChecker.parity != parity(packet).ParityChecker.parity) {
        goto packetError; //parity check failed, so abandon this packet
      }
      if ((frame.index == 0) || (frame.index > frame.payload + 1)) {
        goto packetError; //packet was recieved at wrong time, so drop the current frame.
      } else if (frame.index == 1) {
        //header
        frame.payload = packet.HeaderPacket.length;
        frame.length = packet.HeaderPacket.length + 3;
      }/* else if (frame.index == 2){
        //digital
        
      } else {
        //analog
        
      }*/ //for the time being, analog and digital are not handled here. Just accept them if parity is passed.
      break;
    default:
      //invalid packet, so ignore. This accounts mainly for noise induced packets which are usually 0x00 or 0x01 and so must be filtered out, but are not a frame error... yet.
      return status;
  }
  
  frame.data[frame.index] = packet;
  frame.index++;
  if (status) {
    frame.index = 0; //ready for next time.
  }
  return status;

packetError:
  frame.clear();
  return status;
}

void RFStructure::makeFrame(RFModes mode) {
  if (mode == OfflineMode) {
    frame.clear(); //don't send anything.
    return;
  }
  RFPacket packet;
  byte analog[2];
  for (byte i = 0; i < 2; i++){
    analog[i] = (analogRead(analogPins[i])/10); //read analog pin, reducing to 6bit. Max value will be ~620 due to non rail-rail op-amp.
    analog[i] &= 0x3F; //6bit analog values sent, so fix to 6 bits.
  }
  //PayloadLengths payload =DigitalOnlyPayload;//= payloadLength(mode); 
  frame.payload = 0;
  switch ((byte)mode) {
    case DoubleAnalogMode:
    case DirectionalAnalogMode:
    case DoubleServoMode:
    case DirectionalServoMode:
      frame.payload++;
      packet.AnalogPacket.clear();
      //packet.AnalogPacket.ID = AnalogPacket;
      packet.AnalogPacket.value = analog[0]; //lower 5 bits get sent in analog packet
      frame.data[4] = parity(packet);
    case SingleServoMode:
    case SingleAnalogMode:
      frame.payload++;
      packet.AnalogPacket.clear();
      //packet.AnalogPacket.ID = AnalogPacket;
      packet.AnalogPacket.value = analog[1];
      frame.data[3] = parity(packet);
    case DigitalMode:
      frame.payload++;
      packet.DigitalPacket.clear();
      //packet.DigitalPacket.ID = DigitalPacket;
      if(digitalRead(pins[0])&1){
         packet.DigitalPacket.d0 = 1;
      }
      if(digitalRead(pins[1])&1){
         packet.DigitalPacket.d1 = 1;
      }
      if(digitalRead(pins[2])&1){
         packet.DigitalPacket.d2 = 1;
      }
      if(analog[0]&0x20){
         packet.DigitalPacket.d3 = 1; //upper most bit gets sent in digital packet. In digitalMode, analogRead>50% will be a digital 1.
      }
      if(analog[1]&0x20){
         packet.DigitalPacket.d4 = 1;
      }
      frame.data[2] = parity(packet);
      
      //packet.clear();
      packet.HeaderPacket.clear();//.ID = HeaderPacket;
      packet.HeaderPacket.mode = mode;
      packet.HeaderPacket.length = frame.payload;
      frame.data[1] = parity(packet);
      
      //packet.clear();
      packet.IdentifierPacket.clear();//ID = IdentifierPacket;
      packet.IdentifierPacket.channel = channel;
      frame.data[0] = packet;
  }
  frame.length = frame.payload + 3;
  
  packet.ChecksumPacket.clear();
  //packet.ChecksumPacket.ID = ChecksumPacket;
  packet.ChecksumPacket.checksum = checksum();
  frame.data[frame.length-1] = packet;
  
  packet.clear();
  frame.data[frame.length] = packet; //null termination.
}
