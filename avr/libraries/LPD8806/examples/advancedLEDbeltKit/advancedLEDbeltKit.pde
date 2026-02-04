// Example to control LPD8806-based RGB LED Modules in a strip; originally
// intended for the Adafruit Digital Programmable LED Belt Kit.
// REQUIRES TIMER1 LIBRARY: http://www.arduino.cc/playground/Code/Timer1
// ALSO REQUIRES LPD8806 LIBRARY, which should be included with this code.

// I'm generally not fond of canned animation patterns.  Wanting something
// more nuanced than the usual 8-bit beep-beep-boop-boop pixelly animation,
// this program smoothly cycles through a set of procedural animated effects
// and transitions -- it's like a Video Toaster for your waist!  Some of the
// coding techniques may be a bit obtuse (e.g. function arrays), so novice
// programmers may have an easier time starting out with the 'strandtest'
// program also included with the LPD8806 library.

#include <avr/pgmspace.h>
#include "SPI.h"
#include "LPD8806.h"
#include "TimerOne.h"


#if defined(USB_SERIAL) || defined(USB_SERIAL_ADAFRUIT)
// this is for teensyduino support
int dataPin = 2;
int clockPin = 1;
#else 
// these are the pins we use for the LED belt kit using
// the Leonardo pinouts
int dataPin = 16;
int clockPin = 15;
#endif


// Declare the number of pixels in strand; 32 = 32 pixels in a row.  The
// LED strips have 32 LEDs per meter, but you can extend or cut the strip.
const int numPixels = 32;
// 'const' makes subsequent array declarations possible, otherwise there
// would be a pile of malloc() calls later.

// Instantiate LED strip; arguments are the total number of pixels in strip,
// the data pin number and clock pin number:
LPD8806 strip = LPD8806(numPixels, dataPin, clockPin);

// You can also use hardware SPI for ultra-fast writes by omitting the data
// and clock pin arguments.  This is faster, but the data and clock are then
// fixed to very specific pin numbers: on Arduino 168/328, data = pin 11,
// clock = pin 13.  On Mega, data = pin 51, clock = pin 52.
//LPD8806 strip = LPD8806(numPixels);

// Principle of operation: at any given time, the LEDs depict an image or
// animation effect (referred to as the "back" image throughout this code).
// Periodically, a transition to a new image or animation effect (referred
// to as the "front" image) occurs.  During this transition, a third buffer
// (the "alpha channel") determines how the front and back images are
// combined; it represents the opacity of the front image.  When the
// transition completes, the "front" then becomes the "back," a new front
// is chosen, and the process repeats.
byte imgData[2][numPixels * 3], // Data for 2 strips worth of imagery
     alphaMask[numPixels],      // Alpha channel for compositing images
     backImgIdx = 0,            // Index of 'back' image (always 0 or 1)
     fxIdx[3];                  // Effect # for back & front images + alpha
int  fxVars[3][50],             // Effect instance variables (explained later)
     tCounter   = -1,           // Countdown to next transition
     transitionTime;            // Duration (in frames) of current transition

// function prototypes, leave these be :)
void renderEffect00(byte idx);
void renderEffect01(byte idx);
void renderEffect02(byte idx);
void renderEffect03(byte idx);
void renderAlpha00(void);
void renderAlpha01(void);
void renderAlpha02(void);
void renderAlpha03(void);
void callback();
byte gamma(byte x);
long hsv2rgb(long h, byte s, byte v);
char fixSin(int angle);
char fixCos(int angle);

// List of image effect and alpha channel rendering functions; the code for
// each of these appears later in this file.  Just a few to start with...
// simply append new ones to the appropriate list here:
void (*renderEffect[])(byte) = {
  renderEffect00,
  renderEffect01,
  renderEffect02,
  renderEffect03 },
(*renderAlpha[])(void)  = {
  renderAlpha00,
  renderAlpha01,
  renderAlpha02 };

// ---------------------------------------------------------------------------

void setup() {
  // Start up the LED strip.  Note that strip.show() is NOT called here --
  // the callback function will be invoked immediately when attached, and
  // the first thing the calback does is update the strip.
  strip.begin();

  // Initialize random number generator from a floating analog input.
  randomSeed(analogRead(0));
  memset(imgData, 0, sizeof(imgData)); // Clear image data
  fxVars[backImgIdx][0] = 1;           // Mark back image as initialized

  // Timer1 is used so the strip will update at a known fixed frame rate.
  // Each effect rendering function varies in processing complexity, so
  // the timer allows smooth transitions between effects (otherwise the
  // effects and transitions would jump around in speed...not attractive).
  Timer1.initialize();
  Timer1.attachInterrupt(callback, 1000000 / 60); // 60 frames/second
}

void loop() {
  // Do nothing.  All the work happens in the callback() function below,
  // but we still need loop() here to keep the compiler happy.
}

// Timer1 interrupt handler.  Called at equal intervals; 60 Hz by default.
void callback() {
  // Very first thing here is to issue the strip data generated from the
  // *previous* callback.  It's done this way on purpose because show() is
  // roughly constant-time, so the refresh will always occur on a uniform
  // beat with respect to the Timer1 interrupt.  The various effects
  // rendering and compositing code is not constant-time, and that
  // unevenness would be apparent if show() were called at the end.
  strip.show();

  byte frontImgIdx = 1 - backImgIdx,
       *backPtr    = &imgData[backImgIdx][0],
       r, g, b;
  int  i;

  // Always render back image based on current effect index:
  (*renderEffect[fxIdx[backImgIdx]])(backImgIdx);

  // Front render and composite only happen during transitions...
  if(tCounter > 0) {
    // Transition in progress
    byte *frontPtr = &imgData[frontImgIdx][0];
    int  alpha, inv;

    // Render front image and alpha mask based on current effect indices...
    (*renderEffect[fxIdx[frontImgIdx]])(frontImgIdx);
    (*renderAlpha[fxIdx[2]])();

    // ...then composite front over back:
    for(i=0; i<numPixels; i++) {
      alpha = alphaMask[i] + 1; // 1-256 (allows shift rather than divide)
      inv   = 257 - alpha;      // 1-256 (ditto)
      // r, g, b are placed in variables (rather than directly in the
      // setPixelColor parameter list) because of the postincrement pointer
      // operations -- C/C++ leaves parameter evaluation order up to the
      // implementation; left-to-right order isn't guaranteed.
      r = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      g = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      b = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      strip.setPixelColor(i, r, g, b);
    }
  } else {
    // No transition in progress; just show back image
    for(i=0; i<numPixels; i++) {
      // See note above re: r, g, b vars.
      r = gamma(*backPtr++);
      g = gamma(*backPtr++);
      b = gamma(*backPtr++);
      strip.setPixelColor(i, r, g, b);
    }
  }

  // Count up to next transition (or end of current one):
  tCounter++;
  if(tCounter == 0) { // Transition start
    // Randomly pick next image effect and alpha effect indices:
    fxIdx[frontImgIdx] = random((sizeof(renderEffect) / sizeof(renderEffect[0])));
    fxIdx[2]           = random((sizeof(renderAlpha)  / sizeof(renderAlpha[0])));
    transitionTime     = random(30, 181); // 0.5 to 3 second transitions
    fxVars[frontImgIdx][0] = 0; // Effect not yet initialized
    fxVars[2][0]           = 0; // Transition not yet initialized
  } else if(tCounter >= transitionTime) { // End transition
    fxIdx[backImgIdx] = fxIdx[frontImgIdx]; // Move front effect index to back
    backImgIdx        = 1 - backImgIdx;     // Invert back index
    tCounter          = -120 - random(240); // Hold image 2 to 6 seconds
  }
}

// ---------------------------------------------------------------------------
// Image effect rendering functions.  Each effect is generated parametrically
// (that is, from a set of numbers, usually randomly seeded).  Because both
// back and front images may be rendering the same effect at the same time
// (but with different parameters), a distinct block of parameter memory is
// required for each image.  The 'fxVars' array is a two-dimensional array
// of integers, where the major axis is either 0 or 1 to represent the two
// images, while the minor axis holds 50 elements -- this is working scratch
// space for the effect code to preserve its "state."  The meaning of each
// element is generally unique to each rendering effect, but the first element
// is most often used as a flag indicating whether the effect parameters have
// been initialized yet.  When the back/front image indexes swap at the end of
// each transition, the corresponding set of fxVars, being keyed to the same
// indexes, are automatically carried with them.

// Simplest rendering effect: fill entire image with solid color
void renderEffect00(byte idx) {
  // Only needs to be rendered once, when effect is initialized:
  if(fxVars[idx][0] == 0) {
    byte *ptr = &imgData[idx][0],
      r = random(256), g = random(256), b = random(256);
    for(int i=0; i<numPixels; i++) {
      *ptr++ = r; *ptr++ = g; *ptr++ = b;
    }
    fxVars[idx][0] = 1; // Effect initialized
  }
}

// Rainbow effect (1 or more full loops of color wheel at 100% saturation).
// Not a big fan of this pattern (it's way overused with LED stuff), but it's
// practically part of the Geneva Convention by now.
void renderEffect01(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    // Number of repetitions (complete loops around color wheel); any
    // more than 4 per meter just looks too chaotic and un-rainbow-like.
    // Store as hue 'distance' around complete belt:
    fxVars[idx][1] = (1 + random(4 * ((numPixels + 31) / 32))) * 1536;
    // Frame-to-frame hue increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][2] = 4 + random(fxVars[idx][1]) / numPixels;
    // Reverse speed and hue shift direction half the time.
    if(random(2) == 0) fxVars[idx][1] = -fxVars[idx][1];
    if(random(2) == 0) fxVars[idx][2] = -fxVars[idx][2];
    fxVars[idx][3] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[idx][0];
  long color, i;
  for(i=0; i<numPixels; i++) {
    color = hsv2rgb(fxVars[idx][3] + fxVars[idx][1] * i / numPixels,
      255, 255);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][3] += fxVars[idx][2];
}

// Sine wave chase effect
void renderEffect02(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = random(1536); // Random hue
    // Number of repetitions (complete loops around color wheel);
    // any more than 4 per meter just looks too chaotic.
    // Store as distance around complete belt in half-degree units:
    fxVars[idx][2] = (1 + random(4 * ((numPixels + 31) / 32))) * 720;
    // Frame-to-frame increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][3] = 4 + random(fxVars[idx][1]) / numPixels;
    // Reverse direction half the time.
    if(random(2) == 0) fxVars[idx][3] = -fxVars[idx][3];
    fxVars[idx][4] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[idx][0];
  int  foo;
  long color, i;
  for(long i=0; i<numPixels; i++) {
    foo = fixSin(fxVars[idx][4] + fxVars[idx][2] * i / numPixels);
    // Peaks of sine wave are white, troughs are black, mid-range
    // values are pure hue (100% saturated).
    color = (foo >= 0) ?
       hsv2rgb(fxVars[idx][1], 254 - (foo * 2), 255) :
       hsv2rgb(fxVars[idx][1], 255, 254 + foo * 2);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][4] += fxVars[idx][3];
}

// Data for American-flag-like colors (20 pixels representing
// blue field, stars and stripes).  This gets "stretched" as needed
// to the full LED strip length in the flag effect code, below.
// Can change this data to the colors of your own national flag,
// favorite sports team colors, etc.  OK to change number of elements.
#define C_RED   160,   0,   0
#define C_WHITE 255, 255, 255
#define C_BLUE    0,   0, 100
PROGMEM prog_uchar flagTable[]  = {
  C_BLUE , C_WHITE, C_BLUE , C_WHITE, C_BLUE , C_WHITE, C_BLUE,
  C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED ,
  C_WHITE, C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED };

// Wavy flag effect
void renderEffect03(byte idx) {
  long i, sum, s, x;
  int  idx1, idx2, a, b;
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = 720 + random(720); // Wavyness
    fxVars[idx][2] = 4 + random(10);    // Wave speed
    fxVars[idx][3] = 200 + random(200); // Wave 'puckeryness'
    fxVars[idx][4] = 0;                 // Current  position
    fxVars[idx][0] = 1;                 // Effect initialized
  }
  for(sum=0, i=0; i<numPixels-1; i++) {
    sum += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  byte *ptr = &imgData[idx][0];
  for(s=0, i=0; i<numPixels; i++) {
    x = 256L * ((sizeof(flagTable) / 3) - 1) * s / sum;
    idx1 =  (x >> 8)      * 3;
    idx2 = ((x >> 8) + 1) * 3;
    b    = (x & 255) + 1;
    a    = 257 - b;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1    ]) * a) +
              (pgm_read_byte(&flagTable[idx2    ]) * b)) >> 8;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1 + 1]) * a) +
              (pgm_read_byte(&flagTable[idx2 + 1]) * b)) >> 8;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1 + 2]) * a) +
              (pgm_read_byte(&flagTable[idx2 + 2]) * b)) >> 8;
    s += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  fxVars[idx][4] += fxVars[idx][2];
  if(fxVars[idx][4] >= 720) fxVars[idx][4] -= 720;
}

// TO DO: Add more effects here...Larson scanner, etc.

// ---------------------------------------------------------------------------
// Alpha channel effect rendering functions.  Like the image rendering
// effects, these are typically parametrically-generated...but unlike the
// images, there is only one alpha renderer "in flight" at any given time.
// So it would be okay to use local static variables for storing state
// information...but, given that there could end up being many more render
// functions here, and not wanting to use up all the RAM for static vars
// for each, a third row of fxVars is used for this information.

// Simplest alpha effect: fade entire strip over duration of transition.
void renderAlpha00(void) {
  byte fade = 255L * tCounter / transitionTime;
  for(int i=0; i<numPixels; i++) alphaMask[i] = fade;
}

// Straight left-to-right or right-to-left wipe
void renderAlpha01(void) {
  long x, y, b;
  if(fxVars[2][0] == 0) {
    fxVars[2][1] = random(1, numPixels); // run, in pixels
    fxVars[2][2] = (random(2) == 0) ? 255 : -255; // rise
    fxVars[2][0] = 1; // Transition initialized
  }

  b = (fxVars[2][2] > 0) ?
    (255L + (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime - (numPixels * fxVars[2][2] / fxVars[2][1]) :
    (255L - (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime;
  for(x=0; x<numPixels; x++) {
    y = x * fxVars[2][2] / fxVars[2][1] + b; // y=mx+b, fixed-point style
    if(y < 0)         alphaMask[x] = 0;
    else if(y >= 255) alphaMask[x] = 255;
    else              alphaMask[x] = (byte)y;
  }
}

// Dither reveal between images
void renderAlpha02(void) {
  long fade;
  int  i, bit, reverse, hiWord;

  if(fxVars[2][0] == 0) {
    // Determine most significant bit needed to represent pixel count.
    int hiBit, n = (numPixels - 1) >> 1;
    for(hiBit=1; n; n >>=1) hiBit <<= 1;
    fxVars[2][1] = hiBit;
    fxVars[2][0] = 1; // Transition initialized
  }

  for(i=0; i<numPixels; i++) {
    // Reverse the bits in i for ordered dither:
    for(reverse=0, bit=1; bit <= fxVars[2][1]; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    fade   = 256L * numPixels * tCounter / transitionTime;
    hiWord = (fade >> 8);
    if(reverse == hiWord)     alphaMask[i] = (fade & 255); // Remainder
    else if(reverse < hiWord) alphaMask[i] = 255;
    else                      alphaMask[i] = 0;
  }
}

// TO DO: Add more transitions here...triangle wave reveal, etc.

// ---------------------------------------------------------------------------
// Assorted fixed-point utilities below this line.  Not real interesting.

// Gamma correction compensates for our eyes' nonlinear perception of
// intensity.  It's the LAST step before a pixel value is stored, and
// allows intermediate rendering/processing to occur in linear space.
// The table contains 256 elements (8 bit input), though the outputs are
// only 7 bits (0 to 127).  This is normal and intentional by design: it
// allows all the rendering code to operate in the more familiar unsigned
// 8-bit colorspace (used in a lot of existing graphics code), and better
// preserves accuracy where repeated color blending operations occur.
// Only the final end product is converted to 7 bits, the native format
// for the LPD8806 LED driver.  Gamma correction and 7-bit decimation
// thus occur in a single operation.
PROGMEM prog_uchar gammaTable[]  = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
   11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
   16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
   23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
   30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
   40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
   50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
   62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
   76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
   92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};

// This function (which actually gets 'inlined' anywhere it's called)
// exists so that gammaTable can reside out of the way down here in the
// utility code...didn't want that huge table distracting or intimidating
// folks before even getting into the real substance of the program, and
// the compiler permits forward references to functions but not data.
inline byte gamma(byte x) {
  return pgm_read_byte(&gammaTable[x]);
}

// Fixed-point colorspace conversion: HSV (hue-saturation-value) to RGB.
// This is a bit like the 'Wheel' function from the original strandtest
// code on steroids.  The angular units for the hue parameter may seem a
// bit odd: there are 1536 increments around the full color wheel here --
// not degrees, radians, gradians or any other conventional unit I'm
// aware of.  These units make the conversion code simpler/faster, because
// the wheel can be divided into six sections of 256 values each, very
// easy to handle on an 8-bit microcontroller.  Math is math, and the
// rendering code elsehwere in this file was written to be aware of these
// units.  Saturation and value (brightness) range from 0 to 255.
long hsv2rgb(long h, byte s, byte v) {
  byte r, g, b, lo;
  int  s1;
  long v1;

  // Hue
  h %= 1536;           // -1535 to +1535
  if(h < 0) h += 1536; //     0 to +1535
  lo = h & 255;        // Low byte  = primary/secondary color mix
  switch(h >> 8) {     // High byte = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = s + 1;
  r = 255 - (((255 - r) * s1) >> 8);
  g = 255 - (((255 - g) * s1) >> 8);
  b = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) and 24-bit color concat merged: similar to above, add
  // 1 to allow shifts, and upgrade to long makes other conversions implicit.
  v1 = v + 1;
  return (((r * v1) & 0xff00) << 8) |
          ((g * v1) & 0xff00)       |
         ( (b * v1)           >> 8);
}

// The fixed-point sine and cosine functions use marginally more
// conventional units, equal to 1/2 degree (720 units around full circle),
// chosen because this gives a reasonable resolution for the given output
// range (-127 to +127).  Sine table intentionally contains 181 (not 180)
// elements: 0 to 180 *inclusive*.  This is normal.

PROGMEM prog_char sineTable[181]  = {
    0,  1,  2,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 15, 16, 17,
   18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31, 32, 33, 34,
   35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
   67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 77, 78, 79, 80, 81,
   82, 83, 83, 84, 85, 86, 87, 88, 88, 89, 90, 91, 92, 92, 93, 94,
   95, 95, 96, 97, 97, 98, 99,100,100,101,102,102,103,104,104,105,
  105,106,107,107,108,108,109,110,110,111,111,112,112,113,113,114,
  114,115,115,116,116,117,117,117,118,118,119,119,120,120,120,121,
  121,121,122,122,122,123,123,123,123,124,124,124,124,125,125,125,
  125,125,126,126,126,126,126,126,126,127,127,127,127,127,127,127,
  127,127,127,127,127
};

char fixSin(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
     pgm_read_byte(&sineTable[(angle <= 180) ?
       angle          : // Quadrant 1
      (360 - angle)]) : // Quadrant 2
    -pgm_read_byte(&sineTable[(angle <= 540) ?
      (angle - 360)   : // Quadrant 3
      (720 - angle)]) ; // Quadrant 4
}

char fixCos(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
    ((angle <= 180) ?  pgm_read_byte(&sineTable[180 - angle])  : // Quad 1
                      -pgm_read_byte(&sineTable[angle - 180])) : // Quad 2
    ((angle <= 540) ? -pgm_read_byte(&sineTable[540 - angle])  : // Quad 3
                       pgm_read_byte(&sineTable[angle - 540])) ; // Quad 4
}

