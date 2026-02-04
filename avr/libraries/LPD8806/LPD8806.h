#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class LPD8806 {

 public:

  LPD8806(uint16_t n, uint8_t dpin, uint8_t cpin); // Configurable pins
  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    updatePins(uint8_t dpin, uint8_t cpin), // Change pins, configurable
    updateLength(uint16_t n); // Change strip length
  uint16_t
    numPixels(void);
  uint32_t
    Color(byte, byte, byte),
    getPixelColor(uint16_t n);

 private:

  uint16_t
    numLEDs; // Number of RGB LEDs in strip
  uint8_t
    *pixels, // Holds LED color values (3 bytes each)
    clkpin    , datapin,     // Clock & data pin numbers
    clkpinmask, datapinmask; // Clock & data PORT bitmasks
  volatile uint8_t
    *clkport  , *dataport;   // Clock & data PORT registers
  void
	startBitbang(void);
  boolean
    hardwareSPI, // If 'true', using hardware SPI
    begun;       // If 'true', begin() method was previously invoked
};
