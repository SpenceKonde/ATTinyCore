ATTiny Core - x313, x4, x5, x61, x7 and x8 for Arduino 1.6.x
============
Based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/

All references to the status of various features refer to tests conducted after the fork, and are updated as I feel comfortable declaring them working.  

This core supports the following processors:

* ATtiny2313, 4313 (Working)
* ATtiny24, 44, 84 (Working)
* ATtiny25, 45, 85 (Working)
* ATtiny261, 461, 861 (probably working, lightly tested)
* ATTiny87, 167 (probably working, lightly tested)
* ATTiny48, 88 (probably working, lightly tested)

**When uploading sketches via ISP, due to limitations of the Arduino IDE, you must select a programmer marked ATTiny Classic or ATTiny Modern from the programmers menu (or any other programmer added by an installed third party core) in order to upload properly to most parts.**

### Supported clock speeds:

Internal:
* 8 MHz
* 1 MHz
* 16 MHz (PLL clock,  x5, x61 only)
* 4 MHz (x313 only)
* 0.5 MHz (x313 only)
* 128 kHz 

External crystal (all except x8 series):
* 20 MHz
* 16 MHz
* 12 MHz
* 8 MHz
* 6 MHz
* 4 MHz


Board Manager Installation
============

This core can be installed using the board manager. The board manager URL is:

`http://drazzy.com/package_drazzy.com_index.json`

1. File -> Preferences, enter the above URL in "Additional Board Manager URLs"
2. Tools -> Boards -> Board Manager...
  *If using 1.6.6, close board manager and re-open it (see below)
3. Select ATTinyCore (Modern) and click "Install". 

Due to [a bug](https://github.com/arduino/Arduino/issues/3795) in 1.6.6 of the Arduino IDE, new board manager entries are not visible the first time Board Manager is opened after adding a new board manager URL. 

### Hardware:

To work correctly, these parts should be installed with a 0.1uf capacitor between Vcc and Ground, as close to the chip as possible. Where there are more than one Vcc pin (x61, x7, x8) both must have a capacitor. No other specific hardware is needed, though, when designing a custom board, it is incredibly helpful to provide a convenient ISP header. See the pinout diagrams in the datasheet for the location of the ISP/SPI programming pins. 

Except for the x5, x4, and x313 series, these are only available in surface mount packages. Breakout boards are available from my Tindie store (these are the breakout boards used for testing this core), which have the pins numbered to correspond with the pin numbers used in this core

* x61/x7 series (861/167): https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/
* x8 series (48/88): https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/
* SMD/DIP x5 project board: https://www.tindie.com/products/DrAzzy/attiny85-project-board/
* SMD x4 project board: https://www.tindie.com/products/DrAzzy/attiny84-project-board/



### Serial Support

The ATtiny x4, x5, x61, and x8 chips do not have hardware serial. For these parts, a software serial is included. This uses the analog comparator pins (to take advantage of the interrupt, since very few sketches/libraries use it, while lots of sketches/libraries use PCINTs); the serial is named Serial, to maximize code-compatibility. TX is AIN0, RX is AIN1. This is a software implementation - as such, you cannot receive and send at the same time. If you try, you'll get gibberish, just like using SoftwareSerial.

Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. 



### Defines:


You can identify the core using the following:

```

#define ATTINY_CORE       - Attiny Core
#define ATTINY_CORE_NM    - No-Millis Attiny Core (probably doesn't work - I have plans to deal with this better)

```


These are used to identify features:

```

#define USE_SOFTWARE_SERIAL    (0 = hardware serial, 1 = software serial
#define USE_SOFTWARE_SPI       (not defined if hardware spi present)
#define HAVE_ADC               (1 = has ADC functions)

```

The following identify board variants (various cores have used both styles of defines, so both are provided here to maximize compatibility):

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

Manual Installation
============
Option 1: Download the .zip, extract, and place in the hardware folder inside arduino in your documents folder. (if there is no (documents)/arduino/hardware, create it) 

Option 2: Download the github client, and sync this repo to (documents)/arduino/hardware. 


![core installation](http://drazzy.com/e/img/coreinstall.jpg "You want it to look like this")




### Bootloaders - (Untested, boards.txt entries known bad, hence commented out. See #18 )

Also included for the Tiny84 and Tiny85 is a functioning Optiboot bootloader. There are two versions of this:
> The first is one which is the normal optiboot which can be compiled using the "make attiny84.bat" and "make attiny85.bat" files. This can be hit and miss whether it works due to the fact that the internal oscillator of the ATtiny's is not well calibrated.
> The second is optiboot but it comes with the TinyTuner sketch built in. The bootloader takes up no additional space as the tuning sketch is overwritten by any uploaded program, however it allows the oscillator to be fully calibrated before sketches are downloaded. This is the version supplied (can be recompiled with "make attiny84_tuned.bat" and "make attiny85_tuned.bat" files). See the readme in the tiny\bootloades\optiboot\ folder for more info.
