### ATtiny 828
![828 Pin Mapping](http://drazzy.com/e/img/PinoutT828.jpg "Arduino Pin Mapping for ATtiny 828")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 8192b ( 7680b with bootloader)
RAM  | 512 bytes
EEPROM | 512 bytes
Bootloader | Yes, Optiboot
GPIO Pins | 27
ADC Channels | 28 (including the one on reset), many differential channels
PWM Channels | 4
Interfaces | UART, SPI, slave I2C
Clock options | Internal 1/8mhz, external clock (no crystal) up to 20mhz

### Bootloader Support
The ATtiny828 has hardware bootloader support, and an Optiboot bootloader is included for the ATtiny828. This functionas at 57600 on the hardware serial port; the bootloader is 512b in size leaving 7680b available for user code. Virtual Boot is not used, so there are no restrictions on use of the watchdog timer. Note the issues below involving the clock speed, which may make use of the bootloader difficult in some conditions. As always, the 828 may be used without the bootloader by programming it over ISP.

### Clock options
The ATtiny 828, in the interest of lowering costs, does not provide support for using an external crystal as a clock source, only the internal oscillator (at ~8 or ~1mhz) or an external *clock* source.

### Using external CLOCK on 828
These parts do not support using an external crystal. External Clock, however, is supported - this requires an external clock source (not just a crystal) connected to the CLKI pin. **DANGER** if this clock source is not present, you must supply a clock source to CLKI pin before it can be reprogrammed, including to use a different clock source. Unlike the parts which support a crystal, where this "potentially dangerous" clock option is not available in the menus because there are menu options for the supported speeds with a crystal as a clock source, so the procedure below can be used. On the 48/88/828, these menu options will enable the external CLOCK option. **This is not the same as external crystal - do not use this option if you are unsure about the difference between external clock and external crystal!**

### Internal Oscillator voltage dependence
The internal oscillator is factory calibrated to +/- 10% or +/- 2% for the slightly more expensive 828R. +/- 2% is good enough for serial communication. However, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work. This would interfere with uploads using the bootloader - to work around this, a version of Optiboot is included built assuming the slightly higher operating frequency; this will be used if you select the >4.0v operating voltage prior to doing Burn Bootloader.

### I2C Support
Slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. This is done automatically with the included version of the Wire library.

### SPI Support
There is full Hardware SPI support

### UART (Serial) Support
There is one hardware serial port, Serial. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels
```
UCSRB &=~(1<<TXEN); // disable TX
UCSRB &=~(1<<RXEN); // disable RX
```

### ADC Reference options
Despite having 28 ADC input channels, the 828 only has the two basic reference options.

* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1


### Purchasing ATTiny828 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny828 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATTiny828 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-05-18T04:57:39.963Z-AZB-8_V2_Asy.png.855x570_q85_pad_rcrop.png)
###[Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
###[Bare Boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/)
