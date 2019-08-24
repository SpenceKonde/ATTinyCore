// NeoPixel simple sketch (c) 2013 Shae Erisson, adapted to tinyNeoPixel library by Spence Konde 2019.
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#if (F_CPU>7370000) //neopixel library required 7.37MHz minimum clock speed; this line is used to skip this sketch in internal testing. It is not needed in your sketches.

#include <tinyNeoPixel_Static.h>


// Which pin on the Arduino is connected to the NeoPixels?
// This example uses pin 3, which is on PORTA on all megaTinyCore boards, so default menu option will work.
#define PIN            3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixels);

int delayval = 500; // delay for half a second

void setup() {
  pinMode(PIN,OUTPUT);
  //with tinyNeoPixel_Static, you need to set pinMode yourself. This means you can eliminate pinMode()
  //and replace with direct port writes to save a couple hundred bytes in sketch size (note that this
  //savings is only present when you eliminate *all* references to pinMode).
  //leds.begin() not needed on tinyNeoPixel
}

void loop() {

  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

  for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    leds.setPixelColor(i, leds.Color(0,150,0)); // Moderately bright green color.

    leds.show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  //with tinyNeoPixel_Static, since we have the pixel array, we can also directly manipulate it - this sacrifices the correction for the pixel order, and the clarity of setColor to save a tiny amount of flash and time.
  for(int i=0;i<(NUMPIXELS*3);i++) {
    pixels[i]=150; //set byte i of array (this is channel (i%3) of led (i/3) (respectively, i%4 and i/4 for RGBW leds)
    leds.show(); //show
    delay(delayval); //delay for a period of time
    pixels[i]=0; //turn off the above pixel
    //result is that each pixel will cycle through each of the primary colors (green, red, blue for most LEDs) in turn, and only one LED will be on at a time.
  }
}
#else //neopixel library required 7.37MHz minimum clock speed; these and following lines are used to skip this sketch in internal testing. It is not needed in your sketches.
#warning "Neopixel control requires F_CPU > 7.37MHz"
void setup() {}
void loop() {}
#endif
