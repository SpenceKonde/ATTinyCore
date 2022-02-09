# tinyNeoPixel documentation is elsewhere
tinyNeoPixel is a tweaked version of adafruitNeoPixel, supporting more clock speeds, but should be code compatible excepting the change to the class name. This core also includes tinyNeoPixel_Static, which has been modified further to reduce the flash footprint. It is recommended to read the first one first, as it gives brief summaries of the API, before and discusses supported parts and particularly common issues relating to these parts.
* [Summary and changes specific to tinyNeoPixel_Static](https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/tinyNeoPixel.md)
* [Full class reference from Adafruit](https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html)

In the event that a function listed in that class reference is not defined in tinyNeoPixel, or in the event of differences in behavior between adafruitNeoPixel and tinyNeoPixel (except as noted in the first documentation link above), that is a bug, and is likely unknown to the maintainer of megaTinyCore - please report it via github issue (preferred) or by emailing SpenceKonde@gmail.com

## Licensing
**tinyNeoPixel is LGPL 3 not LGPL 2.1**
It is derived from Adafruit's adafruitNeoPixel library, which is licenced under LGPL 3.
See [LICENSE.md for the library](LICENSE.md)

The rest of this core (except where noted) is LGPL 2.1.

This distinction is rarely relevant, but consult with a legal professional if in doubt.
