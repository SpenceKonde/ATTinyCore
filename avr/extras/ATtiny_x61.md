### ATtiny 261/461/861
![x61 pin mapping](Pinout_x61.jpg "Arduino Pin Mapping for ATtiny x61 series")

Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b (3456b/7552b with optiboot)
RAM  | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | Yes, Optiboot w/virtualboot
GPIO Pins | 15
ADC Channels | 11 (including the one on reset)
PWM Channels | 3
Interfaces | USI, high speed timer
Clock options | Internal 1/8mhz, Internal PLL at 16mhz, external crystal or clock* up to 20mhz
Packages | DIP-20, SOIC-20, MLF-32

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

The ATtiny861 is a specialized microcontroller designed specifically to address the demands of brushless DC (BLDC) motor control. To this end, it has a PLL and high speed timer like the ATtiny85, and it's timer has a mode where it can output three complementary PWM signals (with controllable dead time), as is needed for driving a three phase BLDC motor. It can also be used as a general purpose microcontroller with more pins than the ATtiny84/841. It is available in 20-pin SOIC or DIP package, or TQFP/MLF-32

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny861/461, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins (see UART section below). The bootloader uses 640b of space, leaving 3456 or7552b available for user code. In order to work on the 84, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

Programming the ATTiny861/461 via ISP without the bootloader is fully supported; the 261 is supported only for ISP programming.

### PLL Clock
The ATtiny x61 series parts have an on-chip PLL. This is clocked off the internal oscillator and nominally runs at 64mhz when enabled. As a result, it is possible to clock the chip off 1/4th of the PLL clock speed, providing a 16mhz clock option without a crystal (this has the same accuracy problems as the internal oscillator driving it). Alternately, or in addition to using it to derive the system clock, Timer1 can be clocked off the PLL. See below.

### Timer1 Clock Source option
The ATtiny x61 series parts are equipped with a special high speed 8/10-bit timer, Timer1 (this is very different from the traditional 16-bit timer1 used on the atmega328p and almost every other chip in the 8-bit AVR product line). This timer can be clocked off the system clock, OR from the PLL at 64 mhz or 32mhz. This will impact the frequency of PWM output on Pins 4 and 6.

### Tone Support
Tone() uses timer1. For best results, use pin 6 for tone - this will use Timer1's output compare unit to generate the tone, rather than generating an interrupt to toggle the pin. In this way, tones can be generated up into the MHz range. If timer1 is set to use the PLL clock (provided this is done using the menu option, not manually), Tone will figure this out and output the requested frequency. With timer1 running off the PLL @ 64mhz, tone() should be able to output a 32mhz signal on pin 6! If using SoftwareSerial or the builtin software serial "Serial", tone() will only work on pin 6 while the software serial is actively transmitting or receiving. As only Timer1 is capable of hardware PWM on the x61 series, tone() will break all PWM functionality.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so it is recommended to keep the baud rate low, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART).

To disable the RX channel (to use only TX), the following commands should be used after calling Serial.begin(). No special action is needed to disable the TX line if only RX is needed.
```
ACSR &=~(1<<ACIE);
ACSR |=~(1<<ACD);
```
### Servo Support
As of version 1.2.2, the builtin Servo library supports the Tinyx61 series. As always, while a software serial port is receiving or transmitting, the servo signal will glitch (this includes the builtin software serial "Serial). This  On prior versions, a third party library must be used. The servo library will disable all PWM channels (as timer1 is the only timer capable of hardware PWM).

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
