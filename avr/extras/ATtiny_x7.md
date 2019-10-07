### ATtiny 87/167
![x7 pin mapping](Pinout_x7.jpg "Arduino Pin Mapping for ATtiny x7 series")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 8096b/16768b ( 7552b/15744b with bootloader)
RAM  | 512 bytes
EEPROM | 512 bytes
Bootloader | Yes, Optiboot w/Virtual Boot
GPIO Pins | 15
ADC Channels | 11
PWM Channels | 3
Interfaces | LIN/UART, USI, SPI
Clock options | Internal 1/8mhz, external crystal or clock* up to 20mhz

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny87 and 167, operating on the hardware UART/LIN port at 115200 baud for 12 or 16mhz clock speed, and 57600 when running at 8mhz. In order to work on the x7 series, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the WDT vector gets pointed to the start of the application. As a consequence, the watchdog timer interrupt cannot be used if you're using the Optiboot bootloader (Watchdog reset still works), and the (sketch name)_bootloader.hex file generated via sketch -> Export Compiled Binary won't work. These issues are only relevant when programming through the bootloader.

### Tone Support
Tone() uses timer1. For best results, use pin 11, and 14, as this will use the hardware output compare to generate the square wave instead of using interrupts.

### Alternate Pinout option
There was an old ATtiny x7 core with a different and more awkward pinout. This is supported via the pin mapping option - we recommend the "new" option, which is used in the pinout diagram above.

### I2C Support
There is no hardware I2C peripheral. The USI can be used to provide I2C support with the TinyWireM or TinyWireS library

### SPI Support
There is a hardware SPI port and the normal SPI library can be used.

### UART (Serial) with LIN support
There is one full hardware Serial port with LIN support, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. The ATtiny x7 series has LIN support, unique among the ATtiny linup; LIN (Local Interconnect Network) is frequently used in automotive and industrial applications.

### ADC Reference Options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
* INTERNAL2V56: Internal 2.56v

### Purchasing ATTiny167 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny167 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATTiny167 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-04-19T01:35:24.770Z-AZB7_Asy.png.855x570_q85_pad_rcrop.png)
###[Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
###[Bare Boards](https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/)
