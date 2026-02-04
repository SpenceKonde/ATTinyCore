//DigiJoystick Nunchuck Demo

#include <DigiJoystick.h>
#include <ArduinoNunchuk.h>
#include <TinyWireM.h>

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup() {
  nunchuk.init();
}


void loop() {
  nunchuk.update();
  DigiJoystick.setX((byte) nunchuk.analogX); // scroll X left to right repeatedly
  DigiJoystick.setY((byte) nunchuk.analogY);
  DigiJoystick.setXROT((byte) map(nunchuk.accelX,255,700,0,255));
  DigiJoystick.setYROT((byte) map(nunchuk.accelY,255,850,0,255));
  DigiJoystick.setZROT((byte) map(nunchuk.accelZ,255,750,0,255));
  int buttonByte = 0;
  bitWrite(buttonByte, 0, nunchuk.zButton);
  bitWrite(buttonByte, 1, nunchuk.cButton);
  DigiJoystick.setButtons((byte) buttonByte, (byte) 0);
  DigiJoystick.delay(10); 
  
  
}