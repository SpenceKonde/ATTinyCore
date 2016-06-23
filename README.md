ATTiny Core - 1634, x313, x4, x41, x5, x61, x7, x8 and 828 for Arduino 1.6.x
============

[![Join the chat at https://gitter.im/SpenceKonde/ATTinyCore](https://badges.gitter.im/SpenceKonde/ATTinyCore.svg)](https://gitter.im/SpenceKonde/ATTinyCore?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

### [Installation](Installation.md) 

### ATtinyCore Universal
This core supports the following processors - essentially every ATtiny processor that makes sense to use with Arduino:

* ATtiny2313, 4313 (Working)
* ATtiny24, 44, 84 (Working)
* ATtiny25, 45, 85 (Working)
* ATtiny261, 461, 861 (probably working, lightly tested)
* ATTiny87, 167 (with or without Optiboot bootloader. Lightly tested)
* ATTiny48, 88 (probably working, lightly tested)
* ATTiny441, 841 (With or without Optiboot bootloader)
* ATTiny1634  (With or without Optiboot bootloader)
* ATTiny828 (With or without Optiboot bootloader)

**When uploading sketches via ISP, due to limitations of the Arduino IDE, you must select a programmer marked ATTiny from the programmers menu (or any other programmer added by an installed third party core) in order to upload properly to most parts.**

### Bootloader Support (ATtiny 841, 828, 1634, 87, 167 only)

The Optiboot bootloader is included for the ATtiny 841, 1634, 828 and x7 series (87 and 167). This runs at 57600 baud at 8mhz and slower, and  115200 baud above that. By default it uses UART0 or the LIN/UART as appropriate (bootloaders that use UART1 for devices that have a second UART are included, prefixed with "ser1" - you must flash them manually or modify boards.txt if you wish to use them). Once the bootloader is programmed, the target can be programmed over serial; the bootloader will run after reset, just like on a normal Arduino. The standard DTR reset circuit is highly recommended. 

The ATtiny841, ATtiny1634, and the ATtiny x7 series do not have hardware bootloader support. To make the bootloader work, the "Virtual Boot" functionality of Optiboot is used. Because of this, the Watchdog Timer interrupt vector will always point to the start of the program, and cannot be used for other functionality. Under the hood, the bootloader rewrites the reset and WDT interrupt vectors, pointing the WDT vector at the start of the program (where the reset vector would have pointed), and the reset vector to the bootloader (as there is no BOOTRST fuse).  As a result of this, the Watchdog Timer cannot be used as a software interrupt on these parts, and attempting to do so will cause strange behavior - however, it can still be used to reset the chip. This does not effect the 828 (it has hardware bootloader support), nor does it effect the 1634 or 841 if they are programmed via ISP. 

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

**Warning** When using weird clock frequencies (ones with a frequency (in MHz) by which 64 cannot be divided evenly), micros() is 4-5 times slower (~110 clocks); it still reports the time at the point when it was called, not the end, however, and the time it gives is pretty close to reality (w/in 1% or so). This combination of performance and accuracy is the result of hand tuning for these clock speeds. For other clock speeds (for example, if you add your own), it will be slower still - hundreds of clock cycles - though the numbers will be reasonably accurate. The "stock" micros() executes equally fast at all clock speeds, and just returns wrong values with anything that 64 doesn't divide evenly by.  

I2C support
------------

On the following chips, I2C functionality can be achieved with the hardware USI, using a library like TinyWireM. This library has the necessary #defines for use with these parts: https://github.com/SpenceKonde/TinyWireM.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny 1634

On the following chips, slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. Use https://github.com/orangkucing/WireS for hardware slave functionality, or https://github.com/todbot/SoftI2CMaster for software implementation of I2C master. 
* ATtiny 828
* ATtiny x41 (441/841)

On the following chips, full master/slave I2C functionality is provided in hardware allowing use of the normal Wire library:
* ATtiny x8 (48, 88)

SPI support:
------------

On the following chips, SPI functionality can be achieved with the hardware USI, using an SPI USI library. 
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny 1634

On the following chips, full SPI functionality is provided in hardware allowing use of the normal SPI library:
* ATtiny 828
* ATtiny x7 (87/167) (it has both a USI and full SPI)
* ATtiny x41 (441/841)
* ATtiny x8 (48, 88)

Serial Support
-------

On the following chips, full serial (UART) support is provided in hardware, as Serial (and Serial1 for parts with two serial ports):
* ATtiny x313 (2313/4313)
* ATtiny x7 (87/167 - LIN support)
* ATtiny x41 (441/841 - two UARTs)
* ATtiny 1634 (two UARTs)
* ATtiny 828

On the following chips, no hardware serial is available, however, a built-in software serial is provided. This uses the analog comparator pins (to take advantage of the interrupt, since very few sketches/libraries use it, while lots of sketches/libraries use PCINTs); the serial is named Serial, to maximize code-compatibility. TX is AIN0, RX is AIN1. This is a software implementation - as such, you cannot receive and send at the same time. If you try, you'll get gibberish, just like using SoftwareSerial.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (261/461/861)
* ATtiny x8 (48/88)

These cores are compatible with the usual SoftwareSerial library. 

Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. However, in practice, a larger portion of parts work without tuning than would be expected from the spec. There are two exceptions to this: the ATtiny x41 series, 1634R, and 828R have an internal oscillator factory calibrated to +/- 2% - but only at operating voltage below 4v. Above 4v, the oscillator gets significantly faster, and is no longer good enough for UART communications. The 1634 and 828 (non-R) are not as tightly calibrated (so they may need tuning even at 3.3v) and are a few cents less expensive, but suffer from the same problem at higher voltages. Due to these complexities, it is recommended that those planning to use serial (except on a x41, 1634R or 828R at 2.5~3.3v) use an external crystal or other clock source.

ADC Support
-------
All of the supported parts except for the x313 series have an Analog to Digital converter on chip. Single-ended ADC inputs can be read using the pin number or the Ax constant. In addition to the single-ended input channels marked on the pinout diagrams, many also support differential ADC input channels. To use these, simply call analogRead() with the appropriate ADC channel number, as if it were a pin. To get the ADC channel number, refer to the datasheet - it is listed in the Register Description section of the chapter on the ADC, under the ADMUX register.



Pin Mapping
============

### ATtiny 441/841
![x41 pin mapping](http://drazzy.com/e/products/img/PinoutT841_fixed.png "Arduino Pin Mapping for ATTiny 841 and 441")
### ATtiny 1634
![1634 pin mapping](http://drazzy.com/e/img/PinoutT1634.jpg "Arduino Pin Mapping for ATTiny 1634")

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

### ATtiny 25/45/85
![x5 pin mapping](http://drazzy.com/e/img/PinoutT85.png "Arduino Pin Mapping for ATtiny x5 series")

### ATtiny 24/44/84
![x4 Pin Mapping](http://drazzy.com/e/img/PinoutT84.png "Arduino Pin Mapping for ATtiny x4 series")

### ATtiny 261/461/861
![x61 Pin Mapping](http://drazzy.com/e/img/PinoutT861.png "Arduino Pin Mapping for ATtiny x61 series")

### ATtiny 87/167
![x7 Pin Mapping](http://drazzy.com/e/img/PinoutT167.jpg "Arduino Pin Mapping for ATtiny x7 series")

### ATtiny 2313/4313
![x313 Pin Mapping](http://drazzy.com/e/img/PinoutT4313.jpg "Arduino Pin Mapping for ATtiny x7 series")


Note that two pin mappings are supported for some devices to retain backwards compatibility with other cores - the pin mapping may be chosen from a menu. 

Full pin mapping diagrams are planned for a future version see #50

Note that analog pin numbers (ex A0 ) cannot be used with digitalWrite()/digitalRead()/analogWrite() - all pins have a digital pin number. Analog pin number should only be used for analogRead()

Hardware
============

To work correctly, these parts should be installed with a 0.1uf capacitor between Vcc and Ground, as close to the chip as possible. Where there are more than one Vcc pin (x61, x7, x8) both must have a capacitor. No other specific hardware is needed, though, when designing a custom board, it is incredibly helpful to provide a convenient ISP header. See the pinout diagrams in the datasheet for the location of the ISP/SPI programming pins. A larger value capacitor for power filtering is recommended - if using a 


For use with Optiboot, the following additional components and connections are required:
* Arduino pin 9/PA1/TXD0 to RXI of serial adapter (0/PB0 on 1634)
* Arduino pin 8/PA2/RXD0 to TXO of serial adapter (1/PA7 on 1634)
* Diode between Reset and Vcc (band towards Vcc)
* 0.1uf capacitor between Reset and DTR of serial adapter
* 10k resistor between reset and Vcc (required)
* (optional) LED and series resistor from Arduino pin 2/PB2 (on 841) or pin 13 (on 1634/828) to ground (This is the pin optiboot flashes to let you know it's running)



### Buy Breakout boards
Except for the x5, x4, x61, and x313 series, these are only available in surface mount packages. Breakout boards are available from my Tindie store (these are the breakout boards used for testing this core), which have the pins numbered to correspond with the pin numbers used in this core. Where applicable, all of these assembled boards have the bootloader installed, and all are set to run at the advertised speed (most are available with several speed/voltage combinations). 
* 841: [Bare boards](https://www.tindie.com/products/DrAzzy/attiny84184-breakout/) - [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny841-dev-board-woptiboot/ )
* 1634: [Bare boards](https://www.tindie.com/products/DrAzzy/attiny1634-breakout-wserial-header-bare-board/) - [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny1634-dev-board-woptiboot-assembled/ )
* 828: [Bare boards]( https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/) Assembled Boards coming 4/16
* x61/x7 series (861/167): [Bare boards] (https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/) Assembled Boards coming 4/16
* x8 series (48/88): [Bare boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/ ) Assembled Boards coming 4/16
* SMD/DIP or DIP x5 prototyping board: https://www.tindie.com/products/DrAzzy/attiny85-project-board/
* SMD or DIP x4 prototyping board: https://www.tindie.com/products/DrAzzy/attiny84-project-board/





Caveats  
----------


* Some people have problems programming the 841 and 1634 with USBAsp and TinyISP - but this is not readily reproducible ArduinoAsISP works reliably. In some cases, it has been found that connecting reset to ground while using the ISP programmer fixes things (particularly when using the USBAsp with eXtremeBurner AVR) - if doing this, you must release reset (at least momentarily) after each batch of programming operation. This may be due to bugs in USBAsp firmware - See this thread on the Arduino forums for information on updated USBAsp firmware: http://forum.arduino.cc/index.php?topic=363772 (Links to the new firmware are on pages 5~6 of that thread - the beginning is largely a discussion of the inadequacies of the existing firmware)
* At >4v, the speed of the internal oscillator on 828R, 1634R and 841 parts increases significantly - enough that neither serial (and hence the bootloader) does not work. It is recommended to run at 3.3v if using internal RC oscillator as a clock source - however, for the 828 (as it cannot use an external crystal) a workaround is provided in the form of a special bootloader for use at voltages above 4v, which fudges the baud rate calculations for the bootloader, so that you can upload while running at 5v - but not at 3.3v. This does not effect your sketch - your sketch still needs to deal with it (either by modifying OSCCAL, or adjusting baud rates to compensate). 


Acknowledgements
-----------------

This core is based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/ 
The ATtiny841 support is based on shimniok's ATTiny x41 core, and the 1634 support on Rambo's ATtiny 1634 core. 
And of course - everything is based on the great work of the Arduino development team for creating the Arduino IDE and community which we all use. 
