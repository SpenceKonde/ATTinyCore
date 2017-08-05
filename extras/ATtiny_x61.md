
### ATtiny 261/461/861
![x61 pin mapping](http://drazzy.com/e/img/PinoutT861a.jpg "Arduino Pin Mapping for ATtiny x61 series")

Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b
RAM  | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | No
GPIO Pins | 15
ADC Channels | 11 (including the one on reset)
PWM Channels | 3
Interfaces | USI, high speed timer
Clock options | Internal 1/8mhz, Internal PLL at 16mhz, external crystal or clock up to 20mhz
Packages | DIP-20, SOIC-20, MLF-32

The ATtiny861 is a specialized microcontroller designed specifically to address the demands of brushless DC (BLDC) motor control. To this end, it has a PLL and high speed timer like the ATtiny85, and it's timer has a mode where it can output three complementary PWM signals (with controllable dead time), as is needed for driving a three phase BLDC motor. It can also be used as a general purpose microcontroller with more pins than the ATtiny84/841. It is available in 20-pin SOIC or DIP package, or TQFP/MLF-32 

### PLL Clock
The ATtiny x61 series parts have an on-chip PLL. This is clocked off the internal oscillator and nominally runs at 64mhz when enabled. As a result, it is possible to clock the chip off 1/4th of the PLL clock speed, providing a 16mhz clock option without a crystal (this has the same accuracy problems as the internal oscillator driving it). Alternately, or in addition to using it to derive the system clock, Timer1 can be clocked off the PLL. See below.

### Timer1 Clock Source option
The ATtiny x61 series parts are equipped with a special high speed 8/10-bit timer, Timer1 (this is very different from the traditional 16-bit timer1 used on the atmega328p and almost every other chip in the 8-bit AVR product line). This timer can be clocked off the system clock, OR from the PLL at 64 mhz or 32mhz. This will impact the frequency of PWM output on Pins 4 and 6. 

### Tone Support
Tone() uses timer1. For best results, use pin 6 for tone - this will use Timer1's output compare unit to generate the tone, rather than generating an interrupt to toggle the pin. In this way, tones can be generated up into the MHz range. If timer1 is set to use the PLL clock (provided this is done using the menu option, not manually), Tone will figure this out and output the requested frequency. With timer1 running off the PLL @ 64mhz, tone() should be able to output a 32mhz signal on pin 6!

### I2C Support
There is no hardware I2C peripheral, but there is a USI. As of version 1.1.3, the included Wire.h library will use the USI to provide I2C functionality, and can be used as a drop-in replacement for Wire.h with the caveat that clock speed cannot be set. 

### SPI Support
There is no hardware SPI peripheral. As of version 1.1.3, the included SPI.h library will use the USI to provide SPI functionality, and can be used as a drop-in replacement for SPI.h with the caveat that clock speed cannot be set. 

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so it is recommended to keep the baud rate low, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). 

### ADC Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
* INTERNAL2V56: Internal 2.56v reference, with external bypass
* INTERNAL2V56_NO_CAP: Internal 2.56v reference, without external bypass
* INTERNAL2V56NOBP: Synonym for INTERNAL2V56_NO_CAP

### Purchasing ATTiny861 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny861 boards through my Tindie store - your purchases support the continued development of this core. 

![Picture of ATTiny861 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-04-15T05:26:46.803Z-AZB61_Asy.png.855x570_q85_pad_rcrop.jpg)
### [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)

### [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/)
