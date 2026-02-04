#include "LPD8806.h"

// Arduino library to control LPD8806-based RGB LED Strips
// (c) Adafruit industries
// MIT license

/*****************************************************************************/

// Constructor for use with arbitrary clock/data pins:
LPD8806::LPD8806(uint16_t n, uint8_t dpin, uint8_t cpin) {
  pixels = NULL;
  begun  = false;
  updateLength(n);
  updatePins(dpin, cpin);
}

// Activate hard/soft SPI as appropriate:
void LPD8806::begin(void) {
  startBitbang();
  begun = true;
}

// Change pin assignments post-constructor, using arbitrary pins:
void LPD8806::updatePins(uint8_t dpin, uint8_t cpin) {
  datapin     = dpin;
  clkpin      = cpin;
  clkport     = portOutputRegister(digitalPinToPort(cpin));
  clkpinmask  = digitalPinToBitMask(cpin);
  dataport    = portOutputRegister(digitalPinToPort(dpin));
  datapinmask = digitalPinToBitMask(dpin);

  if(begun == true) { // If begin() was previously invoked...
    startBitbang(); // Regardless, now enable 'soft' SPI outputs
  } // Otherwise, pins are not set to outputs until begin() is called.

  // Note: any prior clock/data pin directions are left as-is and are
  // NOT restored as inputs!

  hardwareSPI = false;
}

// Enable software SPI pins and issue initial latch:
void LPD8806::startBitbang() {
  pinMode(datapin, OUTPUT);
  pinMode(clkpin , OUTPUT);
  *dataport &= ~datapinmask; // Data is held low throughout (latch = 0)
  for(uint8_t i = 8; i>0; i--) {
    *clkport |=  clkpinmask;
    *clkport &= ~clkpinmask;
  }
}

// Change strip length (see notes with empty constructor, above):
void LPD8806::updateLength(uint16_t n) {
  if(pixels != NULL) free(pixels); // Free existing data (if any)
  numLEDs = n;
  n      *= 3; // 3 bytes per pixel
  if(NULL != (pixels = (uint8_t *)malloc(n + 1))) { // Alloc new data
    memset(pixels, 0x80, n); // Init to RGB 'off' state
    pixels[n]    = 0;        // Last byte is always zero for latch
  } else numLEDs = 0;        // else malloc failed
  // 'begun' state does not change -- pins retain prior modes
}

uint16_t LPD8806::numPixels(void) {
  return numLEDs;
}

// This is how data is pushed to the strip.  Unfortunately, the company
// that makes the chip didnt release the protocol document or you need
// to sign an NDA or something stupid like that, but we reverse engineered
// this from a strip controller and it seems to work very nicely!
void LPD8806::show(void) {
  uint16_t i, n3 = numLEDs * 3 + 1; // 3 bytes per LED + 1 for latch
  
  // write 24 bits per pixel

  for (i=0; i<n3; i++ ) {
    for (uint8_t bit=0x80; bit; bit >>= 1) {
      if(pixels[i] & bit) *dataport |=  datapinmask;
      else                *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
    }
  }

}

// Convert separate R,G,B into combined 32-bit GRB color:
uint32_t LPD8806::Color(byte r, byte g, byte b) {
  return 0x808080 | ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

// Set pixel color from separate 7-bit R, G, B components:
void LPD8806::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    *p++ = g | 0x80; // LPD8806 color order is GRB,
    *p++ = r | 0x80; // not the more common RGB,
    *p++ = b | 0x80; // so the order here is intentional; don't "fix"
  }
}

// Set pixel color from 'packed' 32-bit RGB value:
void LPD8806::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    *p++ = (c >> 16) | 0x80;
    *p++ = (c >>  8) | 0x80;
    *p++ =  c        | 0x80;
  }
}

// Query color from previously-set pixel (returns packed 32-bit GRB value)
uint32_t LPD8806::getPixelColor(uint16_t n) {
  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    return ((uint32_t)((uint32_t)pixels[ofs    ] << 16) |
            (uint32_t)((uint32_t)pixels[ofs + 1] <<  8) |
             (uint32_t)pixels[ofs + 2]) & 0x7f7f7f;
  }

  return 0; // Pixel # is out of bounds
}
