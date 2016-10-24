
### ATtiny 48/88
![x8 Pin Mapping](http://drazzy.com/e/img/PinoutT88x.jpg "Arduino Pin Mapping for ATtiny 88/48 in TQFP")
![x8 Pin Mapping](http://drazzy.com/e/img/PinoutT88-PU.jpg "Arduino Pin Mapping for ATtiny 88/48 in DIP")


 Specifications |  .
------------ | -------------
Flash (program memory)   | 4096b/8192b
RAM  | 256/512 bytes
EEPROM | 64 bytes
Bootloader | No
GPIO Pins | 27
ADC Channels | 8
PWM Channels | 4
Interfaces | SPI, I2C
Clock options | Internal 1/8mhz, external clock (no crystal) up to 20mhz

The ATtiny x8 series is intended as a low cost option compatible with the popular ATmega x8 series. As such, they have a nearly identical pinout (with a couple of extra GPIO pins in the TQFP version). Although these have the full hardware I2C and SPI peripherals, they lack both a hardware serial port and the option to use a crystal as a clock source. 


### Clock options
The ATtiny x8 series of microcontrollers, in the interest of lowering costs, does not provide support for using an external crystal as a clock source, only the internal oscillator (at ~8 or ~1mhz) or an external *clock* source. The internal oscillator is only factory calibrated to +/- 10%, so for timing critical tasks, other arrangements (or a different chip) must be used. Note that using an external clock source is not an option in the board drop-down menus, so you cannot set it that way with "burn bootloader" from within the IDE - you must do it manually (this is to prevent new users from accidentally bricking their parts).

### I2C Support
There is full Hardware I2C support. Use the normal Wire library. 

### SPI Support
There is full Hardware SPI support. Use the normal SPI library. 

### UART (Serial) Support
There is no hardware UART support (for cost saving). If running off the internal oscillator (since this chip does not support a crystal), you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so it is recommended to keep the baud rate low, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). 

### ADC Reference options
* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Purchasing ATTiny828 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny88 boards through my Tindie store - your purchases support the continued development of this core. 

![Picture of ATTiny88 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-05-18T04:57:39.963Z-AZB-8_V2_Asy.png.855x570_q85_pad_rcrop.png)
###[Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
###[Bare Boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/)
