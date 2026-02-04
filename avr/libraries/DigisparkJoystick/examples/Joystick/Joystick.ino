//DigiJoystick test and usage documentation

#include "DigiJoystick.h"

void setup() {
  // Do nothing? It seems as if the USB hardware is ready to go on reset
}


void loop() {
  // If not using plentiful DigiJoystick.delay() calls, make sure to
  //DigiJoystick.update(); // call this at least every 50ms
  // calling more often than that is fine
  // this will actually only send the data every once in a while unless the data is different
  
  // you can set the values from a raw byte array with:
  // char myBuf[8] = {
  //   x, y, xrot, yrot, zrot, slider,
  //   buttonLowByte, buttonHighByte
  // };
  // DigiJoystick.setValues(myBuf);
  
  // Or we can also set values like this:
  DigiJoystick.setX((byte) (millis() / 100)); // scroll X left to right repeatedly
  DigiJoystick.setY((byte) 0x30);
  DigiJoystick.setXROT((byte) 0x60);
  DigiJoystick.setYROT((byte) 0x90);
  DigiJoystick.setZROT((byte) 0xB0);
  DigiJoystick.setSLIDER((byte) 0xF0);
  
  // it's best to use DigiJoystick.delay() because it knows how to talk to
  // the connected computer - otherwise the USB link can crash with the 
  // regular arduino delay() function
  DigiJoystick.delay(50); // wait 50 milliseconds
  
  // we can also set buttons like this (lowByte, highByte)
  //DigiJoystick.setButtons(0x00, 0x00);
}