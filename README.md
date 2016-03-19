ATTiny Core - 1634, x313, x4, x41, x5, x61, x7 and x8 for Arduino 1.6.x
============

[![Join the chat at https://gitter.im/SpenceKonde/ATTinyCore](https://badges.gitter.im/SpenceKonde/ATTinyCore.svg)](https://gitter.im/SpenceKonde/ATTinyCore?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


Based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/ , shimniok's ATTiny x41 core, and Rambo's ATtiny 1634 core. 

### Grand Merger
As of 3/17/2016, my ATTiny Modern core has been merged into this one! It is not yet available via board manager, assuming everything check out okay, this will be released on board manager, and the old core will be deprecated and no longer updated. Please report any new issues. This will allow more programmers to be included without crapping up the list, as well as simplifying life for all involved. 


All references to the status of various features refer to tests conducted after the fork, and are updated as I feel comfortable declaring them working.  

This core supports the following processors:

* ATtiny2313, 4313 (Working)
* ATtiny24, 44, 84 (Working)
* ATtiny25, 45, 85 (Working)
* ATtiny261, 461, 861 (probably working, lightly tested)
* ATTiny87, 167 (probably working, lightly tested)
* ATTiny48, 88 (probably working, lightly tested)
* ATTiny441, 841 (With Optiboot bootloader)
* ATTiny1634  (With Optiboot bootloader)
* ATTiny828 (With Optiboot bootloader)

**When uploading sketches via ISP, due to limitations of the Arduino IDE, you must select a programmer marked ATTiny from the programmers menu (or any other programmer added by an installed third party core) in order to upload properly to most parts.**

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

External crystal (x41, 1634 only, in addition to above):
* 18.43 MHz
* 14.74 MHz
* 11.056 MHz
* 9.216 MHz
* 7.37 MHz


Status
===========

* Optiboot bootloader included, and works on the 441/841 (7.37, 8, 9.216, 11.056, 12, 14.74, 16, 18.43, and 20mhz), and 1634 (7.37, 8, 9.216, 11.056, 12, 14.74 and 16mhz), 828 (8mhz)
* Board definitions for non-optiboot 441/841 @ 1, 4, 6, 7.37, 8, 9.216, 11.056, 12, 14.74, 16, 18.43, 20mhz, and the 1634 @1, 4, 6, 7.37, 8, 9.216, 11.056, 12, 14.74 and 16mhz, 828 @ 1, 8 mhz (it doesn't support a crystal)
* Tone is untested on all chips. Please report any problems.
* SPI (441/841/828), Serial (all), and Serial1 (441/841/1634) work. 
* I2C/TWI hardware slave on 441/841/828 supported by WireS library: https://github.com/orangkucing/WireS for 441/841/828
* I2C/TWI software master on 441/841/828 works: https://github.com/todbot/SoftI2CMaster - However, libraries must be adapted. See https://github.com/SpenceKonde/LiquidCrystal_I2C_Tiny for an example
* USI for 1634 can be used for I2C - use this library for I2C master: https://github.com/SpenceKonde/TinyWireM  - However, libraries must be adapted. See https://github.com/SpenceKonde/LiquidCrystal_I2C_Tiny for an example
* Optiboot without the LED blink (noLED) for 841 included; this saves 64 bytes of flash (not used by default - modify boards.txt if needed)
* Optiboot on serial 1 for 841, 1634 included, these are postfixed with "ser1". These must be flashed manually or modify boards.txt. 

Pin Mapping
============

### ATtiny 441/841
![x41 pin mapping](http://drazzy.com/e/img/Tiny841.jpg "Arduino Pin Mapping for ATTiny 841 and 441")
### ATtiny 1634
![1634 pin mapping](http://drazzy.com/e/img/Tiny1634.jpg "Arduino Pin Mapping for ATTiny 1634")

### ATtiny 828

```

ATtiny 828 pin mapping. All pin numbers match ADC and PCINT numbers

//             16*   26   24   14
//          17    27   25   15
//             PC0  PD2  PD0  PB6
//          PC1  PD3  PD1   PB7
//             _________________
// 18 RX  PC2 | *               | PB5   13
// 19 TX  PC3 |                 | PB4   12
// 20 *   PC4 |                 | PB3   11
//        VCC |                 | GND
//        GND |                 | PB2   10
// 21 *   PC5 |                 | PB1    9
// 22 *   PC6 |                 | AVCC
// 23     PC7 |_________________| PB0    8
//           PA0  PA2  PA4  PA6 
//              PA1  PA3  PA5  PA7
//            0     2    4    6
//               1     3    5    7

```


Hardware
============

For use with Optiboot, the following components and connections are required:
* Arduino pin 9/PA1/TXD0 to RXI of serial adapter (0/PB0 on 1634)
* Arduino pin 8/PA2/RXD0 to TXO of serial adapter (1/PA7 on 1634)
* Diode between Reset and Vcc (band towards Vcc)
* 0.1uf capacitor between Reset and DTR of serial adapter
* 10k resistor between reset and Vcc
* (optional) LED and series resistor from Arduino pin 2/PB2/physical pin 5 to ground (This is the pin optiboot flashes)

An example amenable to home etching can be found at http://drazzy.com/e/boards/boards.php

Suitable breakout boards can be purchased from my Tindie shop:

841: https://www.tindie.com/products/DrAzzy/attiny84184-breakout/ 

1634: https://www.tindie.com/products/DrAzzy/attiny1634-breakout-wserial-header-bare-board/

828: https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/

Caveats spec
============

* On the 1634 and 841, when using the Optiboot bootloader, the Watchdog Timer interrupt vector will always point to the start of the program, and cannot be used for other functionality. Because the 1634 and 841 do not have built-in bootloader support, this is achieved with "virtual boot" feature of Optiboot. This bootloader rewrites the reset and WDT interrupt vectors, pointing the WDT vector at the start of the program (where the reset vector would have pointed), and the reset vector to the bootloader (as there is no BOOTRST fuse). This does not effect the 828 (it has hardware bootloader support), nor does it effect the 1634 or 841 if they are programmed via ISP.
* Some people have problems programming the 841 and 1634 with USBAsp and TinyISP - but this is not readily reproducible ArduinoAsISP works reliably. In some cases, it has been found that connecting reset to ground while using the ISP programmer fixes things (particularly when using the USBAsp with eXtremeBurner AVR) - if doing this, you must release reset (at least momentarily) after each batch of programming operation. This may be due to bugs in USBAsp firmware, however, people often report worse results after "upgrading". Follow this thread for a project relating to an improved USBAsp firmware: (help wanted - can anyone find the thread?)
* At >4v, the speed of the internal oscillator on 1634R and 841 parts increases significantly - enough that neither serial (and hence the bootloader) does not work. It is recommended to run at 3.3v if using internal RC oscillator as a clock source. A future release may include an 8.1mhz internal RC @5v option, with it's own bootloader. 
* When using weird clock frequencies (ones with a frequency (in mhz) by which 64 cannot be divided evenly), micros() is 4-5 times slower (~110 clocks); it still reports the time at the point when it was called, not the end, however, and the time it gives is pretty close to reality (w/in 1% or so). This combination of performance and accuracy is the result of hand tuning for these clock speeds. For really weird clock speeds (ie, if you add your own), it will be slower still - hundreds of clock cycles - on the plus side, it still gives reasonably accurate numbers back even on exotic clock speeds, ("stock" micros() executes equally fast at all clock speeds, and just returns bogus values with anything that 64 doesn't divide evenly by) 




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

The ATtiny x41 and 1634 have two hardware UARTs, Serial, and Serial1. 

The ATTiny 828 has one hardware UART, Serial. 

The ATtiny x4, x5, x61, and x8 chips do not have hardware serial. For these parts, a software serial is included. This uses the analog comparator pins (to take advantage of the interrupt, since very few sketches/libraries use it, while lots of sketches/libraries use PCINTs); the serial is named Serial, to maximize code-compatibility. TX is AIN0, RX is AIN1. This is a software implementation - as such, you cannot receive and send at the same time. If you try, you'll get gibberish, just like using SoftwareSerial.

Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. 

In addition, these cores are compatible with the usual software serial library. 

Manual Installation
============
Option 1: Download the .zip, extract, and place in the hardware folder inside arduino in your documents folder. (if there is no (documents)/arduino/hardware, create it) 

Option 2: Download the github client, and sync this repo to (documents)/arduino/hardware. 


![core installation](http://drazzy.com/e/img/coreinstall.jpg "You want it to look like this")



### Defines:


You can identify the core using the following:

```

#define ATTINY_CORE       - Attiny Core

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

#define ATTINYX313 1
#define __AVR_ATtinyX313__

//no backwards compatibility options since no previously existing cores used the other convention. 
#define __AVR_ATtinyX41__
#define __AVR_ATtiny1634__ 
#define __AVR_ATtiny828__

```



