#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
 
//THESE MUST BE SET! CHANGE ADDRESS AS YOU UPLOAD TO EACH NODE!
#define TOTAL_NODES 3 //TOTAL NUMBER OF NODES IN MESH
#define NODE_ADDRESS 0 //the sero indexed address of this node - nodes must be numbered 0 thru TOTAL_NODES-1 //also note 255 is broadcast address
 
//hold the seuqnce numbers to avoid repeating the same messages
uint8_t  sequence[TOTAL_NODES];
 
//setup radio module
RF24 radio(9,12);
 
//message format - adjust data as needed  - the rest is madatory for this mesh to work
struct 
{
 char toAddress;
 char fromAddress;
 char fromSequence;
 long data;
} msg;
 
//don't change! read is same for all nodes, write is automatically calculated here
const uint64_t writePipe = 0xF0F0F0F00LL + NODE_ADDRESS;
const uint64_t readPipe = 0xF0F0F0F0D2LL;
 
 
void setup() {
  // put your setup code here, to run once:
  radio.begin();
  radio.setDataRate(RF24_250KBPS); //lowest speed = most range
  radio.setAutoAck(false); //this is a mesh so we don't want ACKs!
  radio.setRetries(15, 15);
  radio.setPayloadSize(sizeof(msg));
  radio.openWritingPipe(writePipe);
  radio.openReadingPipe(1, readPipe);
  radio.startListening();
}
 
long now = 0;
 
void loop() {
 
 
 
  if(readAndRepeat()){ //will repeat messages as needed and return false unless there is packet for this node - CALL FREQUENTLY!
    //if this does not return false then we have a packet for THIS node!
    //msg.fromAddress is the node that sent it
    //msg.data is the data itself
    //Do something with it!
    //For example display packets coming to this node on a LCD:
    /*
    NOTE: TO USE THIS ADD THE LCD INCLUDES AND SETUP ROUTINE FROM THE DigisparkLCD example  
    lcd.clear();
    lcd.print("From: ");
    lcd.println(msg.fromAddress);
    lcd.print("Value: ");
    lcd.println(msg.data);
    */
  }
 
  if(millis() - now > 10000){ //send a packet from this node to the mesh every 10 seconds but wait in a non-blocking way so that we can still run this loop and repeat things
    now = millis(); //set now to millis so we wait another 10 seconds before sending again
    //sendToMesh(To_Address,Data_To_Send);
    //sendToMesh(0, analogRead(A5)); //send to node 0 the analog read value of pin 5 - could also send a temp sensor value, etc ,etc 
    //sendToMesh(255, analogRead(A5)); //send to all nodes (255 is the broadcast address) the analog read value of pin 5
  }
 
}
 
 
void sendToMesh(uint8_t toAddress, long data){
  if(sequence[NODE_ADDRESS]<255)
    sequence[NODE_ADDRESS]++; //increment sequence count for this device
  else
    sequence[NODE_ADDRESS] = 0; //set to zero if last was 255 so we don't overflow - logic for read is built to handle this too
 
  msg.toAddress = toAddress; //set the address of the destination node
  msg.fromAddress = NODE_ADDRESS; //set the from as this node - of course
  msg.fromSequence = sequence[NODE_ADDRESS]; //set it to the sequence number we just implemented which should be greater than any the nodes have received 
  radio.stopListening(); //turn of recv so we can transmit
  radio.write(&msg, sizeof(msg));
  radio.startListening(); //turn recv back on
}
 
bool readAndRepeat(){
  if(radio.read(&msg, sizeof(msg))){ //if we have incoming data
    if(msg.fromAddress!=NODE_ADDRESS){ //make sure this node didn't send it
      if(sequence[msg.fromAddress] < msg.fromSequence || (sequence[msg.fromAddress] == 255 && msg.fromSequence == 0)){ //make sure we haven't already repeated it or received it
          //increment sequence for that address so we don't repeat it twice from this node or receive it twice
          sequence[msg.fromAddress] = msg.fromSequence;
          if(msg.toAddress==NODE_ADDRESS){ //is it for this node? if so return true so we can use it!
            return true;
          }
          //otherwise repeat it - send it back out
          radio.write(&msg, sizeof(msg));
          if(msg.toAddress == 255){ //it was a broadcast so return true so we do something with it
            return true;
          }
      }
    }
  }
  return false;
}