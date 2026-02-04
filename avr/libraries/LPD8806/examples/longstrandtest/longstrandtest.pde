#include "LPD8806.h"
#include "SPI.h"

// Simple test for 160 (5 meters) of LPD8806-based RGB LED strip

/*****************************************************************************/

// Number of RGB LEDs in strand:
int nLEDs = 160;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);

void setup() {
  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();
}

void loop() {
  colorChase(strip.Color(127,  0,  0), 100); // Red
  colorChase(strip.Color(  0,127,  0), 100); // Green
  colorChase(strip.Color(  0,  0,127), 100); // Blue
  colorChase(strip.Color(127,127,127), 100); // White
}

// Chase one dot down the full strip.  Good for testing purposes.
void colorChase(uint32_t c, uint8_t wait) {
  int i;
  
  // Start by turning all pixels off:
  for(i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, 0);

  // Then display one pixel at a time:
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c); // Set new pixel 'on'
    strip.show();              // Refresh LED states
    strip.setPixelColor(i, 0); // Erase pixel, but don't refresh!
    delay(wait);
  }

  strip.show(); // Refresh to turn off last pixel
}

