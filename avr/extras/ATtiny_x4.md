### ATtiny 24/44/84
![x4 pin mapping](http://drazzy.com/e/img/PinoutT84a.jpg "Arduino Pin Mapping for ATtiny x4 series")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b (3456b/7552b with optiboot)
RAM  | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | Yes, Optiboot w/virtualboot
GPIO Pins | 11
ADC Channels | 8, plus many differential channels
PWM Channels | 4
Interfaces | USI
Clock options | Internal 1/8mhz, external crystal or clock* up to 20mhz

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

Two pinouts are available - this provides compatibility with cores which use either layout. Be sure you are using the pinout you think you are!

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny84/44, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins (see UART section below). The bootloader uses 640b of space, leaving 3456 or7552b available for user code. In order to work on the 84, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

Programming the ATTiny84/44 via ISP without the bootloader is fully supported; the 24 is supported only for ISP programming.

### Tone Support
Tone() uses timer1. For best results, use pin 6 and 5 (4 and 5 with alternate pinout - PA6 and PA5), as this will use the hardware output compare to generate the square wave instead of using interrupts.

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

### ADC Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
