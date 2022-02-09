/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 800 KHz bitstreams on 8, 10, 12, 16, and 20 MHz ATtiny
  MCUs used and ATTinyCore 1.30+ with LEDs wired for various color orders.

  The tinyAVR 0/1/2-series and other modernAVRs use the version that
  comes with megaTinyCore or DxCore, not this version - the instruction
  timing is different.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

  Modification was carried out between 2016 and 2021 by Spence Konde for
  ATTinyCore, and later megaTinyCore and DxCore. The latest modification
  eliminates the menu for the port - It uses ST instructions now, and
  while the assembly is uglier, the result is often smaller code that
  doesn't need a stupid menu option.

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "tinyNeoPixel.h"

// Constructor when length, pin and type are known at compile-time:
tinyNeoPixel::tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t) :
  begun(false), brightness(0), pixels(NULL), endTime(0) {
  updateType(t);
  updateLength(n);
  setPin(p);
}

/* via Michael Vogt/neophob: empty constructor is used when strand length
 * isn't known at compile-time; situations where program config might be
 * read from internal flash memory or an SD card, or arrive via serial
 * command.  If using this constructor, MUST follow up with updateType(),
 * updateLength(), etc. to establish the strand type, length and pin number!
 */
tinyNeoPixel::tinyNeoPixel() :
  begun(false), numLEDs(0), numBytes(0), pin(-1), brightness(0), pixels(NULL),
  rOffset(1), gOffset(0), bOffset(2), wOffset(1), endTime(0) {
}

tinyNeoPixel::~tinyNeoPixel() {
  if (pixels)   free(pixels);
  if (pin >= 0) pinMode(pin, INPUT);
}

void tinyNeoPixel::begin(void) {
  if (pin >= 0) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  begun = true;
}

void tinyNeoPixel::updateLength(uint16_t n) {
  if (pixels) free(pixels); // Free existing data (if any)

  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  if ((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
  } else {
    numLEDs = numBytes = 0;
  }
}

void tinyNeoPixel::updateType(neoPixelType t) {
  boolean oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;

  // If bytes-per-pixel has changed (and pixel data was previously
  // allocated), re-allocate to new size.  Will clear any data.
  if (pixels) {
    boolean newThreeBytesPerPixel = (wOffset == rOffset);
    if (newThreeBytesPerPixel != oldThreeBytesPerPixel) updateLength(numLEDs);
  }
}

// Set the output pin number
void tinyNeoPixel::setPin(uint8_t p) {
  if (begun && (pin >= 0)) pinMode(pin, INPUT);
    pin = p;
    if (begun) {
      pinMode(p, OUTPUT);
      digitalWrite(p, LOW);
    }
    port    = portOutputRegister(digitalPinToPort(p));
    pinMask = digitalPinToBitMask(p);
}

// Set pixel color from separate R,G,B components:
void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if (n < numLEDs) {
    if (brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p;
    if (wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = 0;        // But only R,G,B passed -- set W to 0
    }
    p[rOffset] = r;          // R,G,B always stored
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  if (n < numLEDs) {
    if (brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
      w = (w * brightness) >> 8;
    }
    uint8_t *p;
    if (wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = w;        // Store W
    }
    p[rOffset] = r;          // Store R,G,B
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void tinyNeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if (n < numLEDs) {
    uint8_t *p,
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if (brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    if (wOffset == rOffset) {
      p = &pixels[n * 3];
    } else {
      p = &pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
    }
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

void tinyNeoPixel::fill(uint32_t c, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if (first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if (count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if (end > numLEDs) end = numLEDs;
  }

  for (i = first; i < end; i++) {
    this->setPixelColor(i, c);
  }
}


/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB with the most significant byte set to 0 -- the
           white element of WRGB pixels is NOT utilized. Result is linearly
           but not perceptually correct, so you may want to pass the result
           through the gamma32() function (or your own gamma-correction
           operation) else colors may appear washed out. This is not done
           automatically by this function because coders may desire a more
           refined gamma-correction function than the simplified
           one-size-fits-all operation of gamma32(). Diffusing the LEDs also
           really seems to help when using low-saturation colors.
*/
uint32_t tinyNeoPixel::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  /* Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
   * 0 is not the start of pure red, but the midpoint...a few values above
   * zero and a few below 65536 all yield pure red (similarly, 32768 is the
   * midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
   * each for red, green, blue) really only allows for 1530 distinct hues
   * (not 1536, more on that below), but the full unsigned 16-bit type was
   * chosen for hue so that one's code can easily handle a contiguous color
   * wheel by allowing hue to roll over in either direction.
   */
  hue = (hue * 1530L + 32768) / 65536;
  /* Because red is centered on the rollover point (the +32768 above,
   * essentially a fixed-point +0.5), the above actually yields 0 to 1530,
   * where 0 and 1530 would yield the same thing. Rather than apply a
   * costly modulo operator, 1530 is handled as a special case below.
   *
   * So you'd think that the color "hexcone" (the thing that ramps from
   * pure red, to pure yellow, to pure green and so forth back to red,
   * yielding six slices), and with each color component having 256
   * possible values (0-255), might have 1536 possible items (6*256),
   * but in reality there's 1530. This is because the last element in
   * each 256-element slice is equal to the first element of the next
   * slice, and keeping those in there this would create small
   * discontinuities in the color wheel. So the last element of each
   * slice is dropped...we regard only elements 0-254, with item 255
   * being picked up as element 0 of the next slice. Like this:
   * Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
   * Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
   * Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
   * and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
   * the constants below are not the multiples of 256 you might expect.
   */
  // Convert hue to R,G,B (nested ifs faster than divide + mod + switch):
  if (hue < 510) {         // Red to Green-1
    b = 0;
    if (hue < 255) {       //   Red to Yellow-1
      r = 255;
      g = hue;            //     g = 0 to 254
    } else {              //   Yellow to Green-1
      r = 510 - hue;      //     r = 255 to 1
      g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    r = 0;
    if (hue <  765) {      //   Green to Cyan-1
      g = 255;
      b = hue - 510;      //     b = 0 to 254
    } else {              //   Cyan to Blue-1
      g = 1020 - hue;     //     g = 255 to 1
      b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    g = 0;
    if (hue < 1275) {      //   Blue to Magenta-1
      r = hue - 1020;     //     r = 0 to 254
      b = 255;
    } else {              //   Magenta to Red-1
      r = 255;
      b = 1530 - hue;     //     b = 255 to 1
    }
  } else {                // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 =   1 + val; // 1 to 256; allows >>8 instead of /255
  uint16_t s1 =   1 + sat; // 1 to 256; same reason
  uint8_t  s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
          (((((g * s1) >> 8) + s2) * v1) & 0xff00)       |
          (((((b * s1) >> 8) + s2) * v1)           >> 8);
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t tinyNeoPixel::getPixelColor(uint16_t n) const {
  if (n >= numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if (wOffset == rOffset) { // Is RGB-type device
    p = &pixels[n * 3];
    if (brightness) {
      /* Stored color was decimated by setBrightness().  Returned value
       * attempts to scale back to an approximation of the original 24-bit
       * value used when setting the pixel color, but there will always be
       * some error -- those bits are simply gone.  Issue is most
       * pronounced at low brightness levels.
       */
      return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ((uint32_t)(p[bOffset] << 8) / brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &pixels[n * 4];
    if (brightness) { // Return scaled color
      return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
             (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ((uint32_t)(p[bOffset] << 8) / brightness       );
    } else { // Return raw color
      return ((uint32_t)p[wOffset] << 24) |
             ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  }
}

/* Returns pointer to pixels[] array.  Pixel data is stored in device-
 * native format and is not translated here.  Application will need to be
 * aware of specific pixel data format and handle colors appropriately.
 */
uint8_t *tinyNeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t tinyNeoPixel::numPixels(void) const {
  return numLEDs;
}

/* Adjust output brightness; 0 = darkest (off), 255 = brightest.  This does
 * NOT immediately affect what's currently displayed on the LEDs.  The
 * next call to show() will refresh the LEDs at this level.  However,
 * this process is potentially "lossy," especially when increasing
 * brightness.  The tight timing in the WS2811/WS2812 code means there
 * aren't enough free cycles to perform this scaling on the fly as data
 * is issued.  So we make a pass through the existing color data in RAM
 * and scale it (subsequent graphics commands also work at this
 * brightness level).  If there's a significant step up in brightness,
 * the limited number of steps (quantization) in the old data will be
 * quite visible in the re-scaled version.  For a non-destructive
 * change, you'll need to re-render the full strip data.
 */
void tinyNeoPixel::setBrightness(uint8_t b) {
  /* Stored brightness value is different than what's passed.
   * This simplifies the actual scaling math later, allowing a fast
   * 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
   * adding 1 here may (intentionally) roll over...so 0 = max brightness
   * (color values are interpreted literally; no scaling), 1 = min
   * brightness (off), 255 = just below max brightness.
   */
  uint8_t newBrightness = b + 1;
  if (newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if (oldBrightness == 0) scale = 0; // Avoid /0
    else if (b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for (uint16_t i = 0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

//Return the brightness value
uint8_t tinyNeoPixel::getBrightness(void) const {
  return brightness - 1;
}

void tinyNeoPixel::clear() {
  memset(pixels, 0, numBytes);
}

// A 32-bit variant of gamma8() that applies the same function
// to all components of a packed RGB or WRGB value.
uint32_t tinyNeoPixel::gamma32(uint32_t x) {
  uint8_t *y = (uint8_t *)&x;
  /* All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
   * to avoid a bunch of shifting and masking that would be necessary for
   * properly handling different endianisms (and each byte is a fairly
   * trivial operation, so it might not even be wasting cycles vs a check
   * and branch for the RGB case). In theory this might cause trouble *if*
   * someone's storing information in the unused most significant byte
   * of an RGB value, but this seems exceedingly rare and if it's
   * encountered in reality they can mask values going in or coming out.
   */
  for (uint8_t i = 0; i<4; i++) y[i] = gamma8(y[i]);
  return x; // Packed 32-bit return
}
