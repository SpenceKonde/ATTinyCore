## tinyNeoPixel - a  library for WS2812 "NeoPixel" and similar at full range of supported clock speeds

Outputting data to WS2812 and compatible LEDs requires carefully tuned assembly to get correct output. This means a separate implementation must be provided for several ranges of clock speeds to support the full range of clock speeds that ATTinyCore supports; the readily available WS2812 libraries do not support this. These included libraries support all speeds that ATTinyCore does from 7.3728MHz and up. It is not possible to drive neopixels at slower clock speeds.

### tinyNeoPixel Port menu option
At less than 15.4MHz, the hand-tuned assembly code used to send the data to the LEDs requires the port to be set at compile time. The original Adafruit_NeoPixel library used if/then statements to allow it to work with different ports - however this requires multiple copies of the assembly code in the compiled binary which increases flash usage. Since the pin that the NeoPixel is on is known when the sketch is written, a submenu is provided to select the port to be used with the NeoPixel library - this is used only at speeds below 15.4MHz (and only on parts with more than 8 I/O pins - the x5-series only has one port, and hence there is no menu for the port); at 16MHz and higher, this option is ignored and any pin will work regardless of the menu setting.

### tinyNeoPixel and tinyNeoPixel_Static
There are two versions of this library provided. `tinyNeoPixel` implements the entire API that Adafruit_NeoPixel does, including setting the pin and length of the string at runtime (as noted above, at speeds below 16MHz, if the pin is changed, it must be on the same port). This provides maximum portability between code written for use with Adafruit_NeoPixel and tinyNeoPixel (code written for the Adafruit_NeoPixel library will work with only the name of the library, and the name of the constructor call being changed).

`tinyNeoPixel_Static` is slightly cut-down, removing the option to change the length at runtime, and instead of using malloc() to dynamically allocate the pixel buffer, the user must declare the pixel buffer and pass it to the constructor. Additionally, it does not set the pinMode of the pin (the sketch must set this as OUTPUT). Finally, no call to begin() should be made - begin() is removed entirely. These changes reduce flash use (by eliminating malloc() and free()), and because the pixel buffer is statically declared, the memory used for it is included in count of used SRAM output when the sketch is compiled. Removal of the pinMode() call within the library, while it requires an additional line of code in the sketch, opens the option of eliminating all calls to pinMode() from a sketch (replacing with direct port manipulation) when maximum flash savings is required.

### API Summary


`tinyNeoPixel(uint16_t n, uint8_t p=6, neoPixelType t=NEO_GRB)` constructor for tinyNeoPixel - the first argument is is the number of LEDs in the string, the second is the pin, and the final argument is the color order of the LEDs in use; the library provides #defines for every possible color order for RGB and RGBW LEDs (full list below)

`tinyNeoPixel()` - empty constructor, set pin and length later with setPin(), updateLength(), and updateType().

`tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t,uint8_t *pxl);` constructor for tinyNeoPixel_Static - the final argument is a uint_8 (byte) array sized to accommodate the data to be sent to the LED. For example:


  >#include <"tinyNeoPixel_Static.h">                 <br/>
  >#define NUMLEDS 10                                 <br/>
  >byte pixels[NUMLEDS*3];                            <br/>
  >tinyNeoPixel(NUMPIXELS, 5, NEO_GRB, pixels);       <br/>
  >void setup() {                                     <br/>
  >  pinMode(5,OUTPUT);                               <br/>
  >}

`begin()` Enable the LEDs, on tinyNeoPixel, must be called before show() - not applicable for tinyNeoPixel_Static

`show()` Output the contents of the pixel buffer to the LEDs

`setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)` set the color of pixel `n` to the color `r,g,b` (for RGB LEDs)

`setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)` set the color of pixel `n` to the color `r,g,b,w` (for RGBW LEDs)

`setPixelColor(uint16_t n, uint32_t c)` set the color of pixel `n` to color c (expressed as a uint_32 - as returned from getColor()).

`setBrightness(uint8_t)` set the brightness for the whole string (0~255).

`clear()` clear the pixel buffer (set all colors on all LEDs to 0).

`setPin(uint8_t p)` Set the pin for output. At 8 or 10MHz, this must be on the port selected from the tools -> tinyNeoPixel Port submenu. Note that in tinyNeoPixel, the old pin is set input, and the new pin set OUTPUT and written LOW. This is not done on tinyNeoPixel_Static for the reasons described above.

`updateLength(uint16_t n)` Set the length of the string of LEDs. Not available on tinyNeoPixel_Static.

`updateType(neoPixelType t)` Set the color order. Not available on tinyNeoPixel_Static.

`getPixels()` Returns a pointer to the pixel buffer (a uint_8 array).

`getBrightness()` Returns the current brightness setting (per setBrightness())

`getPin()` Returns the current pin number.

`numPixels()` Returns the number of LEDs in the string

`Color(uint8_t r, uint8_t g, uint8_t b)` Return the color `r,g,b` as a uint_32 (For RGB leds)

`Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)` Return the color `r,g,b,w` as a uint_32 (For RGBW leds)

`getPixelColor(uint16_t n)` Returns the color of pin `n` as a uint_32

### Pixel order #defines
These are the same names for the #defines used by Adafruit_NeoPixel; these are used for the third argument to tinyNeoPixel().

#### For RGB LEDs
>NEO_RGB            <br/>
>NEO_RBG            <br/>
>NEO_GRB            <br/>
>NEO_GBR            <br/>
>NEO_BRG            <br/>
>NEO_BGR

#### For RGBW LEDs
>NEO_WRGB            <br/>
>NEO_WRBG            <br/>
>NEO_WGRB            <br/>
>NEO_WGBR            <br/>
>NEO_WBRG            <br/>
>NEO_WBGR            <br/>
>NEO_RWGB            <br/>
>NEO_RWBG            <br/>
>NEO_RGWB            <br/>
>NEO_RGBW            <br/>
>NEO_RBWG            <br/>
>NEO_RBGW            <br/>
>NEO_GWRB            <br/>
>NEO_GWBR            <br/>
>NEO_GRWB            <br/>
>NEO_GRBW            <br/>
>NEO_GBWR            <br/>
>NEO_GBRW            <br/>
>NEO_BWRG            <br/>
>NEO_BWGR            <br/>
>NEO_BRWG            <br/>
>NEO_BRGW            <br/>
>NEO_BGWR            <br/>
>NEO_BGRW
