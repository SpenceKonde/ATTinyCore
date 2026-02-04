/*
 * Digispark Nunchuck shield demo
 *
 * Uses arduinonunchuk - Copyright 2011-2013 Gabriel Bianconi, http://www.gabrielbianconi.com/ - http://www.gabrielbianconi.com/projects/arduinonunchuk/
 *
 */


#include <ArduinoNunchuk.h>
#include <DigiUSB.h>
#include <TinyWireM.h>

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup()
{
  DigiUSB.begin();
  nunchuk.init();
}

void loop()
{
  nunchuk.update();

  DigiUSB.println(nunchuk.analogX, DEC);

  DigiUSB.println(nunchuk.analogY, DEC);

  DigiUSB.println(nunchuk.accelX, DEC);

  DigiUSB.println(nunchuk.accelY, DEC);

  DigiUSB.println(nunchuk.accelZ, DEC);

  DigiUSB.println(nunchuk.zButton, DEC);

  DigiUSB.println(nunchuk.cButton, DEC);
  
  DigiUSB.delay(250);
}