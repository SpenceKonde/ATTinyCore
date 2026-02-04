#include <DigiUSB.h>

void setup() {
  DigiUSB.begin();
}

void get_input() {
  // when there are no characters to read
  while (1==1) {
    if(DigiUSB.available()){
      //something to read
      DigiUSB.read();
      break;
    }
    // refresh the usb port
    DigiUSB.refresh();
    delay(10);

  }

}


void loop() {
  DigiUSB.refresh();
  //print output
  float value = analogRead(1); //This is Pin3
  if(value>1020)
    value = 255;
  else if(value<2)
    value = 0;
  else
    value = value/4;
   //send value
  
  value = round(byte(value));
  DigiUSB.write(value);
  
  //wait for response
  get_input();

}