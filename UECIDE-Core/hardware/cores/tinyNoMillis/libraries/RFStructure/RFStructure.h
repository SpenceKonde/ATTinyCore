
#include "Arduino.h"

#ifndef RFStructure_h_
#define RFStructure_h_


typedef enum {
  InvalidPacket = 0,
  ChecksumPacket = 1,
  HeaderPacket = 2,
  DataPacket = 2,
  DigitalPacket = 2,
  AnalogPacket = 2,
  IdentifierPacket = 3
} PacketID;

typedef enum {
  RFModes_MIN = 0,
  DigitalMode = RFModes_MIN,
  SingleAnalogMode = 1,
  DoubleAnalogMode = 2,
  DirectionalAnalogMode = 3,
  OfflineMode = 4,
  SingleServoMode = 5,
  DoubleServoMode = 6,
  DirectionalServoMode = 7,
  RFModes_MAX = DirectionalServoMode
} RFModes;

// Overload the ControlType++ operator
inline RFModes operator++(RFModes& mode, int){
  const int i = static_cast<int>(mode)+1;
  if (i > RFModes_MAX){
    mode = RFModes_MIN; //incremented beyond range, so loop back to beginning.
  } else {
    mode = static_cast<RFModes>(i);
  }
  return mode;
}

#define MAXFRAME (1+1+3+1)

typedef struct{
  byte :6;
  PacketID ID:2;
} PacketIDCheckStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte checksum:6;
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = ChecksumPacket;
  }
} ChecksumPacketStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte parity:1;
    byte data:5;
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = DataPacket;
  }
} DataPacketStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte parity:1;
    byte value:5;
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = AnalogPacket;
  }
} AnalogPacketStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte parity:1;
    byte d0:1;
    byte d1:1;
    byte d2:1;
    byte d3:1;
    byte d4:1;
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = DigitalPacket;
  }
} DigitalPacketStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte parity:1;
    byte length:2; //frame payload length
    RFModes mode:3; //data format
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = HeaderPacket;
  }
} HeaderPacketStruct;

typedef union{
  byte __private_byte_;
  struct{
    byte channel:6;
    PacketID ID:2;
  };
  inline void clear(){
    __private_byte_ = 0;
    ID = IdentifierPacket;
  }
} IdentifierPacketStruct;


typedef union {
  byte __private_byte_;
  PacketIDCheckStruct PacketIDCheck;
  ChecksumPacketStruct ChecksumPacket;
  DataPacketStruct DataPacket;
  AnalogPacketStruct AnalogPacket;
  DigitalPacketStruct DigitalPacket;
  HeaderPacketStruct HeaderPacket;
  IdentifierPacketStruct IdentifierPacket;
  struct{
    byte parity:1;
    byte data:5;
    byte :2;
  } ParityChecker;
  inline void clear(){
    __private_byte_ = 0;
  }
} RFPacket;

typedef struct {
  RFPacket data[MAXFRAME+1];
  byte length;
  byte payload;
  byte index;
  inline void clear() {
    memset(this, 0,sizeof(*this)); //clear frame.
  }
} RFFrame;

class RFStructure {
  public:  
    RFFrame frame;
    
    
    const char* writeableFrame();
    
    boolean reassembleFrame(byte packetIn);
    void makeFrame(RFModes mode);
    byte checksum ();
    static RFPacket parity (RFPacket packet) __attribute__((noinline));
    
    static const byte pins[5];
    static const byte analogPins[2];
    byte channel;
};

#endif
