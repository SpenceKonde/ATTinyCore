ATTiny Core - x313, x4, x5, x61, x7 and x8 for Arduino 1.6.x
============
Based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/

All references to the status of various features refer to tests conducted after the fork.  

This core supports the following processors:

* ATtiny2313, 4313 (untested, probably working)
* ATtiny24, 44, 84 (Working)
* ATtiny25, 45, 85 (Working)
* ATtiny261, 461, 861 (probably working)
* ATTiny87, 167 (mostly/probably working)
* ATTiny48, 88 (Basics work, rest untested)

*WHEN UPLOADING OR BOOTLOADING, YOU MUST SELECT A PROGRAMMER MARKED "ATTINY CLASSIC"*
Otherwise, the default avrdude.conf will be used, and it does not provide proper support for many of these chips

### Installation:

Option 1: Download the .zip, extract, and place in the hardware folder inside arduino in your documents folder. (if there is no (documents)/arduino/hardware, create it)

Option 2: Download the github client, and sync this repo to (documents)/arduino/hardware.

This repo is frequently updated, so the second option is recommend if you're comfortable with github, since it will expedite future updates. 


You can identify the core using the following:

```

#define ATTINY_CORE       - Attiny Core
#define ATTINY_CORE_NM    - No-Millis Attiny Core (untested)

```


These are used to identify features:

```

#define USE_SOFTWARE_SERIAL    (0 = hardware serial, 1 = software serial
#define USE_SOFTWARE_SPI       (not defined if hardware spi present)
#define HAVE_ADC               (1 = has ADC functions)

```

The following identify board variants:

```
#define ATTINYX4 1
#define __AVR_ATtinyX4__

#define ATTINYX5 1
#define __AVR_ATtinyX5__

#define ATTINYX61 1
#define __AVR_ATtinyX61__

#define ATTINYX7 1
#define __AVR_ATtinyX7__

\#define ATTINYX313 1
#define __AVR_ATtinyX313__

```

### Bootloaders - untested

Also included for the Tiny84 and Tiny85 is a functioning Optiboot bootloader. There are two versions of this:
> The first is one which is the normal optiboot which can be compiled using the "make attiny84.bat" and "make attiny85.bat" files. This can be hit and miss whether it works due to the fact that the internal oscillator of the ATtiny's is not well calibrated.
> The second is optiboot but it comes with the TinyTuner sketch built in. The bootloader takes up no additional space as the tuning sketch is overwritten by any uploaded program, however it allows the oscillator to be fully calibrated before sketches are downloaded. This is the version supplied (can be recompiled with "make attiny84_tuned.bat" and "make attiny85_tuned.bat" files). See the readme in the tiny\bootloades\optiboot\ folder for more info.
