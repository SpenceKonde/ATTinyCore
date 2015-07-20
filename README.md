ATTiny Core - x313, x4, x5, x61, x7 and x8
============

This is a fork of TCWORLD's ATTinyCore, ported to 1.6.x and taking full advantage of the new board menu options. This core supports the following processors:

* ATtiny2313, 4313 (untested, probably working)
* ATtiny24, 44, 84 (Working)
* ATtiny25, 45, 85 (Working)
* ATtiny261, 461, 861 (probably working)
* ATTiny87, 167 (Untested)
* ATTiny48, 88 (Basics work, rest untested)


Notes from original readme:
=============

Based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/


You can identify the core using the following:
#define ATTINY_CORE       - Attiny Core
#define ATTINY_CORE_NM    - No-Millis Attiny Core

These are used to identify features:
#define USE_SOFTWARE_SERIAL    (0 = hardware serial, 1 = software serial
#define USE_SOFTWARE_SPI       (not defined if hardware spi present)
#define HAVE_ADC               (1 = has ADC functions)

The following identify board variants:
#define ATTINYX4
#define ATTINYX5
#define ATTINYX61
#define ATTINYX7
#define ATTINYX313


Also included for the Tiny84 and Tiny85 is a functioning Optiboot bootloader. There are two versions of this:
> The first is one which is the normal optiboot which can be compiled using the "make attiny84.bat" and "make attiny85.bat" files. This can be hit and miss whether it works due to the fact that the internal oscillator of the ATtiny's is not well calibrated.
> The second is optiboot but it comes with the TinyTuner sketch built in. The bootloader takes up no additional space as the tuning sketch is overwritten by any uploaded program, however it allows the oscillator to be fully calibrated before sketches are downloaded. This is the version supplied (can be recompiled with "make attiny84_tuned.bat" and "make attiny85_tuned.bat" files). See the readme in the tiny\bootloades\optiboot\ folder for more info.
