
### ATtiny 24/44/84
![x4 pin mapping](http://drazzy.com/e/img/PinoutT84a.jpg "Arduino Pin Mapping for ATtiny x4 series")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b
RAM  | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | No
GPIO Pins | 11
ADC Channels | 12 (including the one on reset), many differential channels
PWM Channels | 4
Interfaces | USI
Clock options | Internal 1/8mhz, external crystal or clock up to 20mhz

Two pinouts are available - this provides compatibility with cores which use either layout. 

### Tone Support
Tone() uses timer1. For best results, use pin 6 and 8 (2 and 4 with alternate pinout - PA6 and PB1), as this will use the hardware output compare to generate the square wave instead of using interrupts. 

### I2C Support
There is no hardware I2C peripheral. The USI can be used to provide I2C support with the TinyWireM or TinyWireS library

### SPI Support
There is no hardware SPI peripheral. The USI can be used to provide SPI support with one of the TinySPI libraries

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so it is recommended to keep the baud rate low, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). 

### ADC Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
