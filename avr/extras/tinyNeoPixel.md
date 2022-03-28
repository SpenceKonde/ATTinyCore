## tinyNeoPixel - a  library for WS2812 "NeoPixel" and similar at full range of supported clock speeds

Outputting data to WS2812 and compatible LEDs requires carefully tuned assembly to get correct output. This means a separate implementation must be provided for several ranges of clock speeds to support the full range of clock speeds that ATTinyCore supports; the readily available WS2812 libraries do not support this. These included libraries support all speeds that ATTinyCore does from 7.3728MHz and up. It is not possible to drive neopixels at speeds much below that: 8 MHz has only 10 clock cycles per bit if we are aspiring to meet the standards that this library always has. If the no-dead-time requirement is relaxed, and you were willing to drag a menu option or port-specific code back in, it could be done potentially at as low as 4 MHz but then you will be worrying about the pixels latching between bytes. 2812-alikes latch within just 6-7 us, not the 50 us that the datasheet claims - though some that require 250 us (!!) exist. It is difficult to know which ones you have without careful testing.

## tinyNeoPixel Port menu option REMOVED from ATTinyCore 2.0.0
There assembly has been refactored and now uses ST and works on all pins with neither multiplication of flash use nor the user having to supply an appropriate menu option.
~At less than 15.4MHz, the hand-tuned assembly code used to send the data to the LEDs requires the port to be set at compile time. The original Adafruit_NeoPixel library used if/then statements to allow it to work with different ports - however this requires multiple copies of the assembly code in the compiled binary which increases flash usage. Since the pin that the NeoPixel is on is known when the sketch is written, a submenu is provided to select the port to be used with the NeoPixel library - this is used only at speeds below 15.4MHz (and only on parts with more than 8 I/O pins - the x5-series only has one port, and hence there is no menu for the port); at 16MHz and higher, this option is ignored and any pin will work regardless of the menu setting.~

## Library name
While this library was initially created in order to both ensure compatibility with, and through the Static version, fit within the flash and memory constraints of the tinyAVR line of parts, now with the removal of dependence on a tools menu option in ATTinyCore 2.0.0 (tinyNeoPixel 1.5.0) **this library is entirely suitable for non-tiny devices** - this version (1.5.x) is suitable for all classic AVR devices, and the version distributed with megaTinyCore and DxCore (2.0.x) is suitable for all modern (AVRxt) parts; It offers all the functionality of the Adafruit version on the library, with the addition of the Static mode. This library is code compatible with the version distributed with megaTinyCore and DxCore. Only `show()` has non-trivial changes (those parts are AVRxt, with new and improved instruction timing, that had to be accounted for, and AVR Dx-series parts can be overclocked at high as 48 MHz, requiring the addition of new "speed buckets" with appropriate implementations of Show. .

Like the normal Adafruit_NeoPixel library, this supports WS2811/WS2812/SK6812 and all the nominally compatible single-wire individually addressable LEDs. Unlike that library, support for the really old first generation ones that use the 400 kHz data rate was dropped. I have not seen those for sale in ages, but they do make the library larger. It is also not compatible with the ones that use both data and clock linea, like the APA102 or the even more numerous (global dimming and competition on the basis of pwm frequency for POV systems added more ways to differentiate copycat devices) knockoffs thereof; libraries for those do not pose special compatibility issues with either classic ATtiny or modern AVRxt parts

### Version numbers
The version numbering may be confusing if encountered without warning. There are two active "branches" of this library.
* Version 1.5.x - (released 2022) These versions are included with ATTinyCore 2.0.0 and are the first versions to not require the tools submenu to select the port on classic AVRs. They also include correcting a number of silent problems also fixed in 2.0.4.
* Version 2.0.x - (released 2018-2019) These versions support the modern AVRs (those released in 2016 and later) with the improved instruction timing. The assembly that actually outouts data is not the same. Almost everything else is, and the two branches will track eachother's changes.
* Version 1.0.x - This version predates either of those. It is obsolete and contains significant defects. It should not be used.

## tinyNeoPixel and tinyNeoPixel_Static
There are two versions of this library provided. `tinyNeoPixel` implements the entire API that Adafruit_NeoPixel does, including setting the length and type of LEDs (color order and whether it's RGB or RGBW) of the string at runtime. This provides maximum portability between code written for use with Adafruit_NeoPixel and tinyNeoPixel (only the constructor and library name need to be changed) - however, the memory used to store the "frame buffer" (the color of every pixel) is not included in the SRAM usage displayed when compiling a sketch because it is "dynamically allocated". This can be a large portion of the memory available on smaller parts (I have seen support inquiries from people trying to control a string of LEDs which would require several times the total memory of the part for this alone); trying to allocate a larger array than will fit in RAM does not generate any sort of error - it just fails to allocate anything at runtime, nothing gets sent to the light string, and no LEDs turn on. This dynamic memory allocation also requires compiling in malloc(), free(), and associated functions; on parts with small flash (ie, tinyAVR), this can be significant. Finally, dynamic memory allocation is arguably bad practice on small embedded systems like the AVR devices.

`tinyNeoPixel_Static` is slightly cutdown, removing the option to change the length or type of pixels at runtime (which relies on dynamic memory allocation). Instead it requires the user to manually declare the pixel array and pass it to the tinyNeoPixel constructor. **Additionally, it does not set the pinMode of the pin (the sketch must set this as OUTPUT);** - the reason for this was that it is a no-holds-barred attempt to cut the flash consumption as much as possible, Removing that last pinMode cuts out over 100 bytes, which is nothing to sneeze at on the tiniest of tinies! Note that the savings requires eliminating *all* calls to pinMode and replacing them with writes to`VPORTx.DIR` registers (if you have to set only a small number of bits in one of those, and everything is known at compile time, you're often better with several consecutive `|=` operators each setting one bit, because those end up as a single-word SBI or CBI - while `|=` for more than 1 bit requires a read/modify/write (IN, ORI, OUT, which must be done with interrupts disabled if any interrupts modify the same register. Finally, no call to begin() should be made, nor can it be made. These changes reduce sketch size and provide greater visibility on the memory usage. **Unless you need to change string length or type at runtime, it is recommended that `tinyNeoPixel_Static` be used** - it uses less flash, particularly if you don't use malloc elsewhere (I try to avoid it - the core certainly doesn't, nor, to my knowledge, do any of the included libraries!


## Constructors
The constructor is the declaration that you call outside of any function, to create the global tinyNeoPixel object. The example sizes were recorded on megaTinyCore 2.4.3 on an ATtiny1624 @ 16 MHz

`tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t=NEO_GRB)` - for `tinyNeoPixel` only.
```c++
#include <tinyNeoPixel.h>
#define NUMLEDS 100
tinyNeoPixel leds = tinyNeoPixel(NUMLEDS, PIN_PA6, NEO_GRB);
void setup() {
  leds.begin();
  leds.setPixelColor(0,255,0,0); // first LED full RED
  leds.show();                   // LED turns on.
}
void loop() {/* empty loop */}
```
This example uses 1946 bytes of flash, and reports using only 34 bytes of RAM (it actually uses 334!). If you didn't have enough free memory the call to leds.begin() would fail and the LEDs would not be enabled. No facility is provided detect that begin() didn't.

`tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t=NEO_GRB, uint8_t *pxl)` - for `tinyNeoPixel_static` only - note that the class name is the same, only the library name changes. Also notice how the `pixels` array is handled: It is *declared* as an array. But it is passed without subscripting, thus passing a pointer to the start of the array. Take care to ensure that the number of leds you pass times the number of channels (3 or 4) match what you pass to the constructor. This is C++, it doesn't check array bounds. I strongly recommend
```c++
#include <tinyNeoPixel_Static.h>
#define NUMLEDS 100
byte pixels[NUMLEDS * 3];
tinyNeoPixel leds = tinyNeoPixel(NUMLEDS, PIN_PA6, NEO_GRB, pixels);
void setup() {
  pinMode(PIN_PA6, OUTPUT);
  leds.setPixelColor(0, 255, 0, 0); // first LED full RED
  leds.show();                   // LED turns on.
}
void loop() {/* empty loop */}
```
The equivalent example with the Static version uses only 894 bytes and reports (accurately) that it uses 323 bytes of RAM. While you can't run many leds with them, you can still cram in the basics on 2k parts.


`tinyNeoPixel()` -  Empty constructor for `tinyNeoPixel` only - for when you won't even know the type of LEDs, or how many of them, until your sketch starts running. You set pin and length later with `setPin()`, `updateLength()`, and `updateType()`, which must be set before you can control any LEDs.
```c++
#include <tinyNeoPixel.h>
volatile uint16_t numleds=100;        // Declaring these volatile prevents the constants from being optimized.
volatile neoPixelType led_type;       // This is the easiest way to simulate not knowing what you're driving until runtime.
volatile uint8_t neopin = PIN_PA6;    // Maybe you read from EEPROM, maybe you have jumpers or switches, etc.
tinyNeoPixel leds = tinyNeoPixel();   // empty constructor...
void setup() {
  leds.updateLength(numleds);         // Set the length
  leds.updateType(led_type);          // Set the type
  leds.setPin(neopin);                // set the pin

  leds.setPixelColor(0, 255, 0, 0);      // first LED full RED
  leds.show();                        // LED turns on.
}

void loop() {/* empty loop */}
```
This compiles to 2204 bytes and reports 39 bytes of RAM used as well (actually uses 339).

## API Summary

`begin()` Enable the LEDs, on tinyNeoPixel, must be called before show() - not applicable for tinyNeoPixel_Static.

`show()` Output the contents of the pixel buffer to the LEDs

`setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)` set the color of pixel `n` to the color `r,g,b` (for RGB LEDs)

`setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)` set the color of pixel `n` to the color `r,g,b,w` (for RGBW LEDs)

`setPixelColor(uint16_t n, uint32_t c)` set the color of pixel `n` to color c (expressed as a uint_32 - as returned from getColor())

`getPixelColor(uint16_t n)` Returns the color of pin `n` as a uint_32

`fill(uint32_t c, uint16_t first, uint16_t count)` set `count` pixels, starting from `first` to color `c` which is a 32-bit "packed color". If `first` is unspecified, the first LED on the string is assumed. If `count` is unspecified, or if 0 is passed to it, all the LEDs from `first` to the end of the strip will be set. And if `c` is not specified, it is assumed to be 0 (off) - so `fill()` with no arguments is equivalent to `clear()`.

`setBrightness(uint8_t)` set the brightness for the whole string (0-255). Adjusting the brightness is implemented as multiplying each channel by the given brightness to get a uint16_t, and then taking only the high byte; once brightness has been set, this is done every time pixel(s) are set. Because this process is lossy, frequently adjusting the brightness will lead to quantization errors. At least the modern AVR devices have hardware multiply (note - this adjustment is performed on the whole pixel array when setBrightness() is called, and on specific pixels any time something changes their brightness)

`clear()` clear the pixel buffer (set all colors on all LEDs to 0).

`setPin(uint8_t p)` Set the pin for output; in `tinyNeoPixel_Static`, it is your responsibility to ensure that this pin is set OUTPUT. `tinyNeoPixel` copies the Adafruit behavior, and called pinMode() on it. Be aware

`updateLength(uint16_t n)` Set the length of the string of LEDs. Not available on tinyNeoPixel_Static.

`updateType(neoPixelType_t)` Set the color order and number of colors per pixel. Not available on tinyNeoPixel_Static.

`getPixels()` Returns a pointer to the pixel buffer (a uint_8 array); note that this is the same pointer that you passed the constructor if using tinyNeoPixel_Static.

`getBrightness()` Returns the current brightness setting (per setBrightness())

`getPin()` Returns the current pin number.

`numPixels()` Returns the number of LEDs in the string

`sine8(uint8_t angle)` Returns the sine of the angle (angle in 256's of a circle, that is, 128 = 180 degrees), from 0 to 255. Used for some animation effects. Because this makes use of a lookup table

`gamma8(uint8_t input_brightness)` Performs basic gamma correction for smoother color transitions, returns a gamma corrected brightness that can be passed to setPixelColor().

`gamma32(uint_32 input_color)` As gamma8, only acts on and returns a 32-bit "packed" color (uint32_t).

`Color(uint8_t r, uint8_t g, uint8_t b)` Return the color `r,g,b` as a "packed" color, which is a uint32_t (For RGB leds)

`Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)` Return the color `r,g,b,w` as a uint_32 as a "packed" color, which is a uint32_t (For RGBW leds)

`ColorHSV(uint16_t hue, uint8_t sat, uint8_t val)` Return the color described by the given Hue, Saturation and Value numbers as a uint32_t

## Pixel order constants
In order to specify the order of the colors on each LED, the third argument passed to the constructor should be one of these constants; a define is provided for every possible permutation, however only a small subset of those are widespread in the wild. GRB is by FAR the most common. No, I don't know why either, but I wager there was a reason for it; the human visual system does some surprising things with light and color, and mankind has been figuring out how to make the most of those unexpected factors since we first started painting on cave walls.
### For RGB LEDs
```c++
    NEO_RGB /* Less common than you'd think */
    NEO_RBG
    NEO_GRB /* Very common */
    NEO_GBR
    NEO_BRG
    NEO_BGR
```
### For RGBW LEDs
```c++
    NEO_WRGB
    NEO_WRBG
    NEO_WGRB
    NEO_WGBR
    NEO_WBRG
    NEO_WBGR
    NEO_RWGB
    NEO_RWBG
    NEO_RGWB
    NEO_RGBW
    NEO_RBWG
    NEO_RBGW
    NEO_GWRB
    NEO_GWBR
    NEO_GRWB
    NEO_GRBW
    NEO_GBWR
    NEO_GBRW
    NEO_BWRG
    NEO_BWGR
    NEO_BRWG
    NEO_BRGW
    NEO_BGWR
    NEO_BGRW
```
### Other color orders
You may be thinking - how is that possible? The above list covers every possible permutation! But that is only true when we restrict ourselves to the two most common sets of colors. There are two others (both believed to be based on SK6812) in circulation, both directed towards similar ends. "WWA" is White, Warm White, and Amber (all generated through a phosphor - sp it's basically neutral white, warm white, and VERY warm, white). The order, in the ones I have at least, is Cool White, Amber, Warm White. Use a different color order; then you need only make note in comments something like "GRB order gives  red = Amber, green = CW, and blue = WW for WWA"

The other oddball I've seen is WW ("warm white"? "white/warm white?"). I have not actually mounted those onto a PCB to test out yet, however examination under UV implies that there is only one color of white - a uniform warm-ish white. If anyone has played with them, I'd love to hear how they work - that is, do they have multiple warm white LEDs? Do they take fewer data bytes (potentially only one)? Or is it something else?); until such a time as I get more information, making this library control those shall remain an exercise for the user. If there are a full 3 emitters, it's straightforward to just set colors where all three channels are equal. If they take fewer than 3 bytes, that gets a bit trickier.

## Refresh Rate
Data is sent at 800 kbit/s, or 100k byte/s - 33,333 LEDs/s. Latching takes 50us of idle time, which is pretty small (only long enough for 5 LEDs of data). Note: Although show() guarantees that it can't be called for 50 us after it last returned. This check relies on micros() and is disabled when millis timekeeping is not used or is based on the RTC (hence no micros() is available) as well as on chips with less than 8k of flash (since micros() is painfully large to pull in). A warning is issued in these cases, and it's up to you to ensure that there's a brief pause between calls to show() if you aren't doing something else, like updating the colors of the pixels you're going to send (and if you aren't doing that, why are you sending data again?!). There do exist a few "WS2812" leds with some other chip in them that waits 200us to latch. These are rare, but if you end up using them for some reason, you may need to add a brief delay between calls to show(). On the other hand, apparently most WS2812's will latch the colors after only a few microseconds, so this issue may be largely academic.

## Supported LEDs and LED asssemblies
All individually addressable LEDs marketed as WS2811/2812/SK6812 or SK6805, as well as those claimed to be compatible with such strips are expected to work (supposedly P943S P9411 SM16703 UCS1903 TM1804 are some examples). I am aware of no comprehensive list. Once the WS281x's got popular, everyone started cloning them. There are well in excess of a dozen by now - most of have no english datasheet, and it is not unusual to see a timing diagram cribbed (in some cases literally an exact copy) taken from the WS2812 datasheet. They're usually like the WS2811, ie, they get connected to external LEDs, not integrated with the LEDs themselves in a single package (as noted below, integrating the IC die with the LED poses technical challenges). They sometimes vary in thresholds of their timing (I have, for example, seen cases where a controller would only drive one of the two types, but I've never seen one that couldn't use the output of other "2812-alikes") what the maximum voltage the pins can hold off is (some are built to support multiple LEDs connected in series), and at what price they are sold. However, with the exception of the SK6812/6805 and WS281x (as well as the APA102 and it's clones), they are not widely available in the western world under their real names/part numbers. The part numbers of the two well known addressable LEDs are widely used to refer to this sort of LED = regardless of what part number the LED controller has. In finished devices, the WS2812 and SK6812 names are used interchangibly when finished devices are sold - regardless of which one is used (be it SK68xx, WS2812B, or some randome no-name clone). One thing that is worh noting is that people, over time, have debated whether SK6812 or WS2812 is better; one does have to admit that SK6812's are available in in RGBW (indeed, with Cool, Neutral and Warm white) and WWA versions, while the WS2812's are only RGB.

I won't try to state which controllers come in which packages: I simply don't know, because the sellers aren't honest and/or don't know which precise controller was used. The only way I could find out would be to depot them and examine the die under a microscope, and even that isn't foolproof. In any event, made by someone or another, they are available in nearly every plausible component package: 5050, 3535, at least two drastically different packages both called 2020 by the merchant selling them, 1515 (that's 1.5mm on a side), 2427 (these I am told are what are under the surface of COB led strip), 3216 (in english units, 1206 - the same size as the resistors), and more. In the other direction, there are 3mm and 5mm through-hole diffused ones (though they are quite uncommon, and are *not* cheap, and there are even a few PCBs that pair a WS2811 and a trio of beefy MOSFET drivers for *MONSTER* leds, in the 10-50 watt range; the use case they're intended for remains unclear to me. Even at Direct-from-China prices, even a modest string of them would cost thousands of dollars, not counting the added heatsink you'd need for each LED.

### Key differences
There are a very small number of *significant* differences between the 2812-alikes, other than package and who made them:
* The 5050 package with 6 pins was used by the original WS2812,, the WS281 and WS2815,  and a small number of compatible clones. It is also used by a very large number of incompetible leds. If you see leds with a 6-pin package and were considering buying them, you need to do a bit of further investigation. There are three possibilitis, and only one of them is appropriate if you wanted 2812-alikes. The pinout chart will hold the answers; if it's strip, you can probably get the key information from the markings on the strip:
  * If they have two power pins per LED, or (for strip) if there is only one data line, they are actual WS2812-not-B's. Pass, these burned out easily and had no particular advantage.
  * If they have a DI/DO and BI/BO - these are WS2813/WS2815 LEDs! These are the top of the line from World Semi. DI/DO is the normal date line, while BI/BO is a backup data line. If one LED in the strip fails for any reason, the rest of the strip after it will not fail too. The difference between the two is that WS2813's are 5v, and behave much the same as typical 2812-alikes except for the redundant data lines. WS2815's run at 12v, drawing only 1/3rd the current for the same brightness (my understanding is that they have three tiny LED dies in series). Since the voltage drop along a conductor is proportional to the current, you have to inject power less frequently. Knowing that power is defined as P = V * I, and that V = 12, and I is one third that of the WS2812, we can see that the overall power ends up as: (12/5 * 1/3) or 4/5ths that if a 2812. As noted, they're of similar brightness; the operating forward voltage of a blue LED is around 3.2V. With perfect drivers (assuming no minimum dropout - this isn't the case, but they're quite good), there is 1.8v or so of margin on the blue channel (this is the one that has the highest voltage requirement) Once the blue hits it's limit, every 0.1V below that lowers the voltage applied to the LED by 0.1s, causing color rendition failure. The 12V ones have around 2.4V of margin for blue, and once you do hit that limit, the the voltage seen by each LED will only drop by one third that much. The disdvantages of the the WS2815's are the higher cost, the lack of RGBW options, and the fact that they need a different voltage power supply than the controller. They also supposedly will wait longer before latching (250uS according to some source).
  * If they have DI/DO and CI/CO, they aren't 2812-alikes. They are APA102-alikes. They use a two-wire non-timing-critical control scheme, cost more, PWM at a higher frequency for less weird and undesirable persistence of vision effects, and use 4 bytes per LED, including a 5-bit global dimming field. The two wire control allows data to be pushed through them far faster, increasing the effective maximum frame rate by more than an order of magnitude (depending on the clone, the maximum ranges from 10 MHz to 40 MHz). These are excellent LEDs. The added finesse of global diming makes low brightness colors look great. They are also significantly more expensive, have no RGBW or WWA versions, and as noted, have a different control scheme. This library does not support APA102 or APA102-alikes: Since the control can be achieved with the SPI port very efficiently and there are no modern-AVR-specific caveats, you can use the usual libraries recommended for them on other Arduino-compatible boards. They should work on modern AVRs too.

### Flexible PCB strip
Typically made with sticky tape on one side, and 5050 LEDs on the other, or covered in a silicon waterproofing layer for outdoor use. This stuff is incredibly common, and can often be seen in the wild decorating storefronts, and is the least expensive of these options. It looks pretty lousy if you don't use some sort of diffuser, or have the light reaching viewers indirectly (such as locating it in a recessed fixture on the ceiling, or at the top of a glass-fronted display case that is below eye level). The points are just too bright for the tiny area of the visual field that they come from. This complaint is apparently widespread, since there are a number of solutions to this. There are little diffuser caps you're supposed to put over 5050 LEDs, individually. They don't really help. And there is a much more effective method that essentially places the strip within an opaque rubber sleve which directs the light in one direction or another as well as diffusing it; these actually work pretty well, though the added expense is not negligible, the result is bulkier, and these sleeves are available in many crosssections, and it may not be clear which is ideal for a given use case.

There area few notable variations on the LED strip:
### SK6805 COB LED strip
"COB" LEDs are those LEDs wherein the emitter is embedded in some sort of soft rubber, generally featuring large numbers of smaller LEDs (which is more efficient, and looks better). In this case, there is a row of incredibly high density (332/m) of SK6805 LEDs in a very small packages, hidden under a white rubbery diffuser that leaves the total strip barely thicker than a standard 5050-strip. While it doesn't throw off as much light because of the lower power LEDs, the tight packing of the pattern makes it appear almost continuous. Vendors claim (though I find dubious) that they only need power injected every 3m (at both ends). 1M with power into 1 end does work, though, so maybe the claim holds up. It certainly shouldn't be hard to avoid voltage droop browning if it doesn't actually hold up by limiting brightness in software. COB LED strip is incredibly beautiful - and eye-wateringly expensive - $30/m or so.
* Individually addressable 12v LED strip with external IC's and standalone RGB LEDs, usually with three LEDs per IC has each group of LEDs controlled as a group. This means less data needs to be sent for the same number of LEDs, but also limits flexibility. This is not recommended (though see Diffused String Lights below).
* Individually addressable 12v LED strip with just the LEDs is usually based on the WS2815 (this is likely what you want if going the 12v route - they put three tiny lower current LEDs in series, and drop the current they put through them). These also have an extra "bypass" data in and out, so that if one of the LEDs fails completely, the whole string doesn't go out.
* Warning: There exist 12v LED strips that are not addressable. These have no relation to this library, they are controlled using beefy mosfets, and all the LEDs on the strip must be the same color. Further discussion of these is beyond the scope of this document. They're no where near as much fun as addressable ones.

### Rigid PCBs amd panels
Pre-populated PCBs full of addressable LEDs are readily available. They are commonly shaped like rings, bars, or squares. These also almost universally use 5050 package LEDs (with the exception of some panels, which use smaller packages to more effectivelty make a display). The refresh rate quickly becomes problematic on larger displays. While the LEDs are still not terribly comfortable to look at, the smaller number of them and their confinement to a small part of ones visual field on small panels, or distance and the larger number involved in a truly massve display on the wall of a building or something isn't unpleasant to look at in the same way. Many panels sold use an entirely different control scheme, however, on account of the limited frame  rate achievable as the number of LEDs increases.

### "String Lights"
Name in quotes because I don't think this is an official name; I don't know that they have one, but I have to call them something. These are the only type that is essentially always based on the WS2811: Each module has a single large RGB LED with the fogged plastic for light diffusion, connected to the '2811 driver IC (this is in spite of the existence of LEDs of the same size with 2812-alike controllers inside - I guess it's cheaper to do it this way, and the folks going . Each of those LED assemblies, with 6 wires from one the end opposite the LED, then has plastic surrounding it (it looks to me like they got pushed into a housing that fit tightly around the bulb and epoxy was then poured in). You should inject power at the start (or end) of each string of 50. You should use the additional power wires they provide for this; JST-SM connectors like they use can **barely** handle 3A (the maximum current from 50 LEDs), and you never want to push something right to it's limit. So should have power coming into one or both of the pairs of power wires on every section.
#### "Cursed" String Lights - mystery has been solved
I was recently made aware of (as in was wondering what the heck was wrong with my lights) a type of light string which to all outward appearances, looks almost identical to normal string lights. Side-by-side, tiny differences like the exact tint of the potting compound may be visible. However, even though the data rate coming out is correct, many LEDs will not work when down-stream from them. It is nearly impossible to confirm this behavior without a 'scope: These apparently use a different controller which also comes in a SOIC8 package... but this one outputs 3V logic levels. I didn't think it makes much sense either... until several weeks later, I was checking light strips and came upon them again - only this time the LED that was in my hand happened to have a smoother surface to the epoxy-filled case near the bottom, so I cold read the silkscreen inside. Gnd. Dat. 12v. This was twelve volt LED string! Likely the logic voltage was going through an internal regulator. If it was expecting 7 volts of headroom, they wouldn't think twice about using cheaper regulator technology, resulting in 7805-like minimum dropouts, which from 5v would yield right around 3.3v. So these are not actually cursed, but are likely better. The only thing that gives me pause is that the brightness wasn't obviously "way too dim". There are no inductors inside for buck or boost conversion, so this raises some questions...

### Diffused "String Lights"
These diffusers are almost always hemispheres, affixed to a round PCB, with two sets of wires coming out opposite sides (imagine taking a round PCB with LED(s) and maybe an IC on it, cutting a pingpong ball in half, and putting that over it. The flat side is usually equipped with adhesive tape for mounting against the wall. Sometimes, each module has a WS2811 and 3 LEDs. These are likely the least common of the types listed here. (I actually *did* the pingpong ball thing, with cheapo knockoff pingpong ball. That was back before such strings were easy to buy; besides I wanted APA102's for that project, and I've only seen those on panels and plain strip. It took two halves stacked to get enough diffusing, for the one led in the middle. While they came out beautifully, the process was was a ton of work, and I can't recommend that to others - besides, I used the apparently extinct "APA102TW", which has a distinctly better red).

Real pingpong balls are insanely flammable - for reasons I've never understood they are still made from 50% nitrocellulose, which is one of the active components of smokeless gunpowder. It was widely used as a plastic (it was the original material that film for movies was made from, too) before less flammable ones were invented. But it is still almost universally used in real pingpong balls (maybe competition rules dictate that any other material is not legal?). Never, ever tape them to your wall or put heat generating electronics in them (they can be recognized by scratching with your fingernail - real ones also contain camphor, which has a distinct smell. You can also always cut off a small piece, take it outdoors away dry grass or other highly flammable material, and set it aight.Don't hold it in your hand while you light it, you cannot drop it fast enough. Normal plastic will burn with a small flame, melting, maybe dripping molten plastic on the ground, take tens of seconds or more to burn, and will leave behind some unburned crud. Nitrocellulose will burn completely. leaving just a thin wisp of ash in a second or two.

### Fairy Lights
These are, as far as I know, a fairly recent innovation. These use thinner wires and MUCH smaller leds (looks like a single LED, with wires soldered directly onto it, covered in a blob of diffusing plastic. They come in strings of (typically) 50, 100, and 200. Some of the 50's can be daisy-chained (and include wires to inject power). The longer strings generally can't, and most have an integral USB power connector and controller (which generates impressively ugly patterns). It can be desoldered where it meets the LEDs, though I'd recommend cutting the wire, or desoldering it at the USB end, as their wire looks more attractive that the wire most of us have kicking around, and the LED wire itself is is a bit tricky to work with. These strings use lower current SK6805 LEDs, which allows the existence of 100 and 200 LED strings with such thin wire and with USB power (though in the case of 200-LED strings, if you start playing with them, you'll realize that the the controller is limiting the brightness in software to stay within a power budget and keep the ones at the end from going ofg-color. Note also that there are (or at least were) two kinds of these; in both cases, the wire looks fragile, but through totally different mechanisms. The currently dominant version has solid (not stranded) wire. On the one hand, this holds its shape very well and can easily be bent into a desired shape. On the downside, this bending action makes it more likely to break via fatigue, and the wires are also more visible. In the past, I have received a few sets made with a very different wire - incredibly flexible; I think there were other fibers woven in with the copper for improved flexibility; it looked great, but it was eager to knot itself if you did anything suboptimal to it (like putting it onto a reel without spinning the reel on it's axis - if the reel does not rotate on it's axis during rolling and unrolling each wrap introduces a 360 degree twist!).

#### Fairy Light controllers
The fairy lights appear to use an LED that has slightly different timing from the standard ones - or at least, one that *tolerates* slightly different timing than the standard. It's not clear to me exactly *what* is different, but something is. They work with this library, but the controllers don't work with many other LEDs - they run much slower. So if you were thinking you could chop off the controller, and wire it up to an Arduino, you can. But if you were expecting to use that controller to debug any other kind of addressable LEDs? Nope - they seem to work with only a minority of LEDs, making them of practically zero use as a debugging aid.

### Notable LEDs that are NOT compatible
* Anything that is not addressable, such as standard (single color or RGB) LED strip. As far as I have seen, every listing for a WS2812 compatible device has either WS2811, WS2812, or SK6812, and usually all three, in the title of the listing on marketplace sites like EBay, AliExpress, and Amazon.
* Anything with a data AND a clock line - APA102, APA107 SK9822, HD107 and similar. These are all great LEDs - they implement 5-bit of global diming dimming, in some cases using *actual constant current sources* instead of a superimposing a second PWM frequency!  Because of that, and because they generate PWM at a higher frequency, they are suited for persistance-of-vision dependent art without looking like dashed lines, and do a far better job of rendering dim colors. Since there is both a clock and data line the transfer rate is governed by the controller and the maximum specified by the LEDs. These maximums are often high enough that in practice, the limit of the wiring will be encountered first unless great care is taken in the wiring. There exist multiple other libraries for these, since they opperate via SPI (be it software or hardware SPI), no special measures are required to make those libraries compatible with AVRxt. More care and research is needed when buying these, too, since the specific LED, in some applications, actually makes a big difference, even though there is a control scheme which works for all of them (though not the HD108, which wants 8 bytes per LED instead of 4, for 16-bit color - and of course, is completely incompatible with everything else, which is likely why it doesn't seem to have set the world on fire like the WS2812 and APA102 did).

### Bad pixels
They do exist, and travel in groups. Apparently when these were relatively new there were a LOT of bad batches of them making the rounds; the root cause, apparently, was that the transparent material atop the die and LEDs was not making an airtight seal, and corrosion would rapidly destroy them at that point. On the other hand, this problem seems to be considerably less common than it was, and they are now fairly rare. At the peak of the pixel plague (as in a plague that afflicts pixels, not a plague of pixels; the latter is ongoing with no sign of slowing), the "wisdom of the internet" was that the SK6812 LEDs (which are knockoffs) were in fact more reliable than WS2812's of that era. As I said, I think WorldSemi (assuming they even still make them, and didn't throw their hands up in despair over the clones) has gotten their process sorted out.

### SK6805 vs SK6812
There exists a very similar led to the SK6812, called the SK6805. There are, as far as I can tell, two key differences. Physically, SK6805 is typically smaller (though there is some overlap in available packages) - the smallest I have seen is 1.5 x 1.5mm (yes, you read that right, you can fit a 3x3 array of them in the footprint of a standard 5050 LED), while 3.5x3.5 is very common, and 2427 is supposedly used in "COB" led strips. These are the ones used in most fairy lights. The second difference is that, on account of the smaller size and likely closer spacing, they only draw about 5 mA per channel.

## Safety Notice
Large LED lighting projects can pose safety hazards when connected improperly (generally fire, though the exposed mains terminals on the backs of some power supplies commonly used for this should also be covered, along with other exposed wires). While these concerns are by no means unique to LED lighting projects, such projects often combine high current, shoddy components, inexperienced designers/programmers and operation for prolonged periods of time with little or no supervision - so particular care is warranted.

Large LED light strips can draw a surprising amount of current (60mA per LED and up to 144 LEDs/m with some strip = 8.6A/m), and a great deal of discussion has been devoted how and how often power should be injected. What I rarely see mentioned is the limit of the ubiquitous JST-SM connectors that these strips usually use. Those are rated at a MAXIMUM of 3A, when used with 22 gauge wire, and when the terminals involved are actual JST terminals (most from China are almost certainly copies, so maybe best not push it if you're not buying name brand connectors). I have seen wires supplying power to light strings get so hot (without any sign of failure of the string) that the red positive wire had darkened to the same color as the black ground wire. How could this have happened? I used pre-wired connectors with 22 AWG wire to bring in power! Examination revealed that the wire, with "22 AWG" printed right on it, was actually between 29 and 30 AWG, with the difference in outside diameter made up with thicker insulation. This is much worse than usual, but it is not unusual for "22 AWG" wire from China to turn out to be only 26 AWG.

In addition to following all the usual safety guidelines, pay particular attention to these:
* Do not overload connectors; check the maximum current rating for any type of connector you're using, and make sure that the worst case scenario remains comfortably below it. Surprisingly few people show much concern about overloading their connectors.
* Always **measure** the gauge of the wire in a batch of wire or pre-wired connectors; **intentionally mismarked wire is very common** on the Chinese market, with thicker insulation to make it look like what it is sold as. This is particularly troublesome when you were planning to crimp on connectors, as the insulation thickness no longer matches what the crimper and terminals expect.
* Always pull-test a few samples from a batch of prewired connectors (most commercial dupont line fails the pull test, and even some JST-SM prewired connector. Hopefully if you crimp your own, you're already pull testing them.).
* When using pre-wired connectors, use a pin extractor to pull out a male and female pin from a given batch and compare them to a pin on the "same" connectors you already have. Proceed only with great caution and testing if you discover that the existing connectors and the ones you just got do not look identical - some terminals are made in several versions with different mating force or for different wire gauges. These both matter, but are generally designed to be compatible. On the other hand there are horror stories of incompatible male and female pins from different suppliers that would fail over time when used together, or male pins that were ever so slightly too large and would damage female terminals from other manufacturers - both could result in a poor (high resistance) connection which might get hot without causing visible symptoms until you start a fire, or notice that the wires have been discolored from heat..
* Make sure that any power supply you use will turn itself off in the event of overcurrent, instead of changing to constant current mode. Folding back to constant current mode is great for a benchtop supply in your workshop, but in the field, all it does is waste power generating a bunch of heat wherever the fault is, until either someone notices, something burns out, or it starts a fire.
* Be certain that the system will deal gracefully with all pixels being written white. I have witnessed malfunctioning code result in this on a number of occasions. If you compile code for one speed, and it somehow ends up on a board running markedly slower, it will result in all the bits, 0 or 1, looking like 1's and hence all LEDs coming on white.
* Consider the use of a resettable circuit breaker (these aren't that expensive) or even a fuse if you are not satisfied with the safety of the supply you are using when you attempt to draw excessive current.
* Consider implementing an automatic shutoff in software the lights are left on for a certain length of time without the settings being changed (or on other criteria depending on what sensors it may have) - cases where the LEDs are at nearly maximum brightness are of greatest concern.
* Are you using a buck or boost converter anywhere? If so, make sure it uses synchronous rectification to minimize the amount of waste heat it generates (and reduce overall power consumption). I often use laptop power bricks (approx 3A at 19-20V, in a nice consumer-proof form, which can usually be obtained for free if you don't need them to match any laptop that's still useful), carried by a second pair of wires ziptied along the length of the string, combined with a high efficiency buck converter at each point where power is injected. Buck converters can only pull the voltage upwards, not downwards, so they can't "fight" like some power supplies can. Under the no-load condition, whichever converter has the highest set point due to variation between units will carry most of the current - but as soon as the lights are on and there's a load, the same wire resistance that forces us to use power injectors in the first place will distribute the load more or less evenly. Use buck converters that are capable of handling a far larger load than you are asking of them.

## Comparison with other WS2812 libraries
Both FastLED and Adafruit libraries have the buffered design which - while expensive in terms of RAM, you need it for arbitrary control over the colors of each pixel. Other schemes can be used for cases where the patterns are simple, at substantal memory savings (an example that supports these parts is [FAB_LED](https://github.com/sonyhome/FAB_LED)), and while those are very valuable in applicable situations, I wanted something that could take the place of a buffered, full-service LED copntrol library. That meant that I should be modifying either FastLED or AdafruitNeoPixel (ideally keeping the API nearly identical); Opening the source code for the libraries, I found the Adafruit one to be readily understood, simple, with one purpose that it handles capably, and extensively commented, especially the assembly. Meanwhile FastLED was largely unreadable. I felt that the code suffered from "kitchen-sinkism" - when the author throws feature after feature, up to and including the kitchen sink, into a gigantic omnibus library. I had yet to locate the code that actually generated the output in the FastLED source within the length of time it took to modify the Adafruit library to achieve what I needed in the first tinyNeoPixel release.

## New Adafruit additions
If Adafruit has added new methods to their library, please report via an issue in one of my cores that ships with this library so that I can pull in the changes.

## Changelog - V1.5.x (AVRe/AVRe+) version
* 1.5.0 - Remove memu options for port and adapt assembly to output with valid timing with ST. Correct insufficiently constraining constraints in operands passed to inline asm. Should now be up to date with Adafruit version in terms of accessory functions.



## License
Unlike the core itself (as noted within the library files), tinyNeoPixel is licensed under LGPL 3, not LGPL 2.1, since the Adafruit library was always LGPL 3.

`*` - C++ predates the invention of names like "properties" and "methods" (as well as countless programming concepts that we take for granted on more modern lanuages, if you haven't noticed). Strictly speaking, they are called "member variables" and "member functions" - however when you call them those names, nobody without exposure to C++ outside Arduino who didn't learn programming 25+ years ago will have any idea what you're talking about.
