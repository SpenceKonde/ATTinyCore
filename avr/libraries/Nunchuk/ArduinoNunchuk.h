/*
 * ArduinoNunchuk.h - Improved Wii Nunchuk library for Arduino
 *
 * Copyright 2011-2013 Gabriel Bianconi, http://www.gabrielbianconi.com/
 *
 * Project URL: http://www.gabrielbianconi.com/projects/arduinonunchuk/
 *
 * Based on the following resources:
 *   http://www.windmeadow.com/node/42
 *   http://todbot.com/blog/2008/02/18/wiichuck-wii-nunchuck-adapter-available/
 *   http://wiibrew.org/wiki/Wiimote/Extension_Controllers
 *
 */

#ifndef ArduinoNunchuk_H
#define ArduinoNunchuk_H

#include <Arduino.h>

class ArduinoNunchuk
{
  public:
    int analogX;
    int analogY;
    int accelX;
    int accelY;
    int accelZ;
    int zButton;
    int cButton;

    void init();
    void update();

  private:
    void _sendByte(byte data, byte location);
};

#endif
