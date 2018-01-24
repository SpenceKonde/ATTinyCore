ATTiny Core - 1634, x313, x4, x41, x5, x61, x7, x8 and 828 for Arduino 1.6.x
============

[![Join the chat at https://gitter.im/SpenceKonde/ATTinyCore](https://badges.gitter.im/SpenceKonde/ATTinyCore.svg)](https://gitter.im/SpenceKonde/ATTinyCore?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


### [Installation](Installation.md) 
### [Basic Wiring](Wiring.md) 

### ATtinyCore Universal
This core supports the following processors - essentially every ATtiny processor that makes sense to use with Arduino. Click the processor name for part-specific information:

* [ATtiny2313, 4313](avr/extras/ATtiny_x313.md)
* [ATtiny24, 44, 84](avr/extras/ATtiny_x4.md)
* [ATtiny25, 45, 85](avr/extras/ATtiny_x5.md)
* [ATtiny261, 461, 861](avr/extras/ATtiny_x61.md) 
* [ATTiny87, 167](avr/extras/ATtiny_x7.md) (with or without Optiboot bootloader)
* [ATTiny48, 88](avr/extras/ATtiny_x8.md)
* [ATTiny441, 841](avr/extras/ATtiny_x41.md) (With or without Optiboot bootloader)
* [ATTiny1634](avr/extras/ATtiny_1634.md)  (With or without Optiboot bootloader)
* [ATTiny828](avr/extras/ATtiny_828.md) (With or without Optiboot bootloader)

Variants of these are also supported (such as the ATTiny1634R or ATTiny85V)

This core does NOT support ATtiny x14/x17/x18 or other chips with the "PIC-like" peripherals that have come out since the microchip merger. A very large amount of work would be required to support these parts. See #170 for details. 

### Quick Gotcha list:

**When uploading sketches via ISP using the Arduino IDE, you must select a programmer marked ATTiny from the programmers menu (or any other programmer added by an installed third party core) in order to upload properly to most supported chips - this is due to a limitation in the IDE.**

**When using a chip for the first time, or after changing the clock speed or BOD settings, you must do "burn bootloader" to set the fuses, even if you are not using a chip with a bootloader** 

**When using analogRead(), use the A# constant to refer to the pin, not the digital pin number**

**You cannot use the Pxn notation (ie, PB2, PA1, etc) to refer to pins.**

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

External crystal (all except 828 and x8 series):
* 20 MHz
* 16 MHz
* 12 MHz
* 8 MHz
* 6 MHz
* 4 MHz

External crystal (in addition to above, x41, 1634 only in 1.1.4 and earlier, all chips except x8 and 828 on 1.1.5 and later):
* 18.43 MHz
* 14.74 MHz
* 11.056 MHz
* 9.216 MHz
* 7.37 MHz

**Warning** When using weird clock frequencies (ones with a frequency (in MHz) by which 64 cannot be divided evenly), micros() is 4-5 times slower (~110 clocks) (It reports the time at the point when it was called, not the end, however, and the time it gives is pretty close to reality - w/in 1% or so). This combination of performance and accuracy is the result of hand tuning for these clock speeds. For other clock speeds (for example, if you add your own), it will be slower still - hundreds of clock cycles - though the numbers will be reasonably accurate. Millis() is not effected, only micros() and delay(). 

This differs from the behavior of official Arduino boards - the "stock" micros() executes equally fast at all clock speeds, but returns wrong values with anything that 64 doesn't divide evenly by.

Link-time Optimization (LTO) support
------------
In version 1.1.2 and later, this core supports Link Time Optimization (lto). This can substantially reduce the compiled size of your sketch. Version 1.6.13 or later of the official AVR boards package (included with 1.6.11 and later of the IDE) is required for this functionality. Link time optimization can be enabled from the option in the tools menu.

Makefile Support
------------
For those who prefer to compile with a makefile instead of the IDE, sketches can be compiled with https://github.com/sudar/Arduino-Makefile - See the [makefile documentation](makefile.md) for more information on specific steps needed for this process.

I2C support
------------

On the following chips, I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core.  

* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny 1634

On the following chips, slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. This is done automatically with the included Wire library. 
* ATtiny 828
* ATtiny x41 (441/841)

On the following chips, full master/slave I2C functionality is provided in hardware and the Wire library uses it:
* ATtiny x8 (48, 88)

SPI support:
------------

On the following chips, full SPI functionality is provided in hardware, and works identically to SPI on Atmega chips:
* ATtiny 828
* ATtiny x7 (87/167) (it has both a USI and full SPI, but the SPI library will use the SPI hardware)
* ATtiny x41 (441/841)
* ATtiny x8 (48, 88)

On the following chips, SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the **USI does not have MISO/MOSI, it has DI/DO**; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common). Clock dividers 2, 4, 8 and >=14 are implemented as separate routines; **call `SPISettings` or `setClockDivider` with a constant value to use less program space**, otherwise, all routines will be included along with 32-bit math. Clock dividers larger than 14 are only approximate because the routine is optimized for size, not exactness. Also, interrupts are not disabled during data transfer as SPI clock doesn't need to be precise in most cases. If you use long interrupt routines or require consistent clocking, wrap calls to `transfer` in `ATOMIC_BLOCK`.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny 1634

Serial Support
-------

On the following chips, full serial (UART) support is provided in hardware, as Serial (and Serial1 for parts with two serial ports):
* ATtiny x313 (2313/4313)
* ATtiny x7 (87/167 - LIN support, including a very fancy baud rate generator)
* ATtiny x41 (441/841 - two UARTs)
* ATtiny 1634 (two UARTs)
* ATtiny 828

On the following chips, **no hardware serial is available**, however, a built-in software serial named Serial is provided to maximize compatibility. This uses the analog comparator pins (to take advantage of the interrupt, since very few sketches/libraries use it, while lots of sketches/libraries use PCINTs). **TX is AIN0, RX is AIN1** -  This is a software implementation - as such, you cannot receive and send at the same time. If you try, you'll get gibberish, just like using SoftwareSerial.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (261/461/861)
* ATtiny x8 (48/88)

This core is also fully compatible with the usual SoftwareSerial library. 

**Warning: Internal oscillator and Serial**
Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. However, in practice, a larger portion of parts work without tuning than would be expected from the spec. 

The ATtiny x41 series, 1634R, and 828R have an internal oscillator factory calibrated to +/- 2% - but only at operating voltage below 4v. Above 4v, the oscillator gets significantly faster, and is no longer good enough for UART communications. The 1634 and 828 (non-R) are not as tightly calibrated (so they may need tuning even at 3.3v) and are a few cents less expensive, but suffer from the same problem at higher voltages. Due to these complexities, **it is recommended that those planning to use serial (except on a x41, 1634R or 828R at 2.5~3.3v) use an external crystal.**

A tuning sketch is planned for a future version of this core. (#139)

ADC Support
-------
All of the supported parts except for the x313 series have an Analog to Digital converter on chip. **Single-ended ADC inputs can be read using the ADC channel number or the Ax constant (they can NOT be read using the digital pin number)**. In addition to the single-ended input channels marked on the pinout diagrams, many also support differential ADC input channels. To use these, simply call analogRead() with the appropriate ADC channel number. To get the ADC channel number, refer to the datasheet - it is listed in the Register Description section of the chapter on the ADC, under the ADMUX register.

B. O. D. (brown out detect) Configuration option
--------
All chips have a menu to select the level of Brown-out Detection, if any, to use. Brown-out detection continuously monitors Vcc, and holds the chip in reset state (BOR) if the applied voltage is below a certain threshold. This is a good idea with slow-rising power supplies or where it is expected that the supply voltage could droop below the required operating voltage for the frequency it is running at (see the speed grade specification for the part you're using) - without BOD enabled, this can put the chip into a hung state until manually reset. However, BOD increases power consumption slightly, and hence may be inappropriate in low power applications. The selected BOD option is configured by the fuses, and as such these settings are applied upon burning bootloader, not upon sketch upload. 





Pin Mapping
============

### ATtiny 441/841
![x41 pin mapping](http://drazzy.com/e/products/img/PinoutT841_fixed.png "Arduino Pin Mapping for ATTiny 841 and 441")
### ATtiny 1634
![1634 pin mapping](http://drazzy.com/e/img/PinoutT1634a.jpg "Arduino Pin Mapping for ATTiny 1634")

### ATtiny 828
![828 Pin Mapping](http://drazzy.com/e/img/PinoutT828x.jpg "Arduino Pin Mapping for ATtiny 828")

### ATtiny 25/45/85
![x5 pin mapping](http://drazzy.com/e/img/PinoutT85a.jpg "Arduino Pin Mapping for ATtiny 85/45/25")

### ATtiny 24/44/84
![x4 Pin Mapping](http://drazzy.com/e/img/PinoutT84a.jpg "Arduino Pin Mapping for ATtiny 84/44/24")

### ATtiny 261/461/861
![x61 Pin Mapping](http://drazzy.com/e/img/PinoutT861.jpg "Arduino Pin Mapping for ATtiny 861/461/261")

### ATtiny 87/167
![x7 Pin Mapping](http://drazzy.com/e/img/PinoutT167a.jpg "Arduino Pin Mapping for ATtiny 167/87")

### ATtiny 88/48
![x8 Pin Mapping](http://drazzy.com/e/img/PinoutT88x.jpg "Arduino Pin Mapping for ATtiny 88/48 in TQFP")
![x8 Pin Mapping](http://drazzy.com/e/img/PinoutT88-PU.jpg "Arduino Pin Mapping for ATtiny 88/48 in DIP")

### ATtiny 2313/4313
![x313 Pin Mapping](http://drazzy.com/e/img/PinoutT4313a.jpg "Arduino Pin Mapping for ATtiny 4313/2313")


Note that two pin mappings are supported for some devices to retain backwards compatibility with other cores - the pin mapping may be chosen from a menu. 

Note that analog pin numbers (ex A0 ) cannot be used with digitalWrite()/digitalRead()/analogWrite() - all pins have a digital pin number. Analog pin number should only be used for analogRead() - this represents a departure from the behavior used in the official AVR boards. This enables us to expose the advanced ADC functionality available on some of the ATtiny parts with minimal impact, as clearly written code is unlikely to fall afoul of this anyway. 

Hardware
============

To work correctly, these parts should be installed with a 0.1uf capacitor between Vcc and Ground, as close to the chip as possible. Where there are more than one Vcc pin (x61, x7, x8) both must have a capacitor. No other specific hardware is needed, though, when designing a custom board, it is incredibly helpful to provide a convenient ISP header. See the pinout diagrams in the datasheet for the location of the ISP/SPI programming pins. A larger value capacitor for power filtering is recommended - if using a regulator, the ones specified for the regulator are typically sufficient for this. If the power supply rail is shared with higher power devices that will be switched on and off during operation, larger capacitors may be necessary. 


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
* 828: [Bare boards]( https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/) [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/ )
* x61 series (861/461/261): [Bare boards] (https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/) [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
* x7 series (167/87): [Bare boards] (https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/) [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
* x8 series (48/88): [Bare boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/ ) [Assembled Boards] ( https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/ )
* SMD/DIP or DIP ATtiny 85 prototyping board: https://www.tindie.com/products/DrAzzy/attiny85-project-board/
* SMD or DIP ATtiny 84 prototyping board: https://www.tindie.com/products/DrAzzy/attiny84-project-board/





Caveats  
----------


* Some people have problems programming the 841 and 1634 with USBAsp and TinyISP - but this is not readily reproducible. ArduinoAsISP works reliably. In some cases, it has been found that connecting reset to ground while using the ISP programmer fixes things (particularly when using the USBAsp with eXtremeBurner AVR) - if doing this, you must release reset (at least momentarily) after each programming operation. This may be due to bugs in USBAsp firmware - See this thread on the Arduino forums for information on updated USBAsp firmware: http://forum.arduino.cc/index.php?topic=363772 (Links to the new firmware are on pages 5~6 of that thread - the beginning is largely a discussion of the inadequacies of the existing firmware)
* At >4v, the speed of the internal oscillator on 828R, 1634R and 841 parts increases significantly - enough that serial (and hence the bootloader) does not work. It is recommended to run at 3.3v if using internal RC oscillator as a clock source - however, for these chips, a workaround is provided. This takes the form of a bootloader compiled assuming the chip is running a little fast, so that UART communication will work, and a matching board definition that tries to compensate by assuming the chip is running at 8.2mhz instead of 8. If you wish to handle tuning of the oscillator in your sketch (this means that serial won't work and timing will be off until you do something about it), you can use the 5v workaround as the bootloader but compile assuming 8mhz - select that option when you burn bootloader, but not when compiling and uploading. (New feature added in version 1.1.2)

Acknowledgements
-----------------

This core is based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/ 
The ATtiny841 support is based on shimniok's ATTiny x41 core, and the 1634 support on Rambo's ATtiny 1634 core. 
And of course - everything is based on the great work of the Arduino development team for creating the Arduino IDE and community which we all use. 
