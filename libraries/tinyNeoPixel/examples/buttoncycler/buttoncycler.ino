// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#if (F_CPU>7370000) //neopixel library required 7.37MHz minimum clock speed; this line is used to skip this sketch in internal testing. It is not needed in your sketches.

#include <tinyNeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB);

int delayval = 500; // delay for half a second

void setup() {

  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {

  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

  for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).

  }
}
#else //neopixel library required 7.37MHz minimum clock speed; these and following lines are used to skip this sketch in internal testing. It is not needed in your sketches.
#warning "Neopixel control requires F_CPU > 7.37MHz"
void setup() {}
void loop() {}
#endif
