### ATtiny 2313/4313
![x4 pin mapping](http://drazzy.com/e/img/PinoutT4313a.jpg "Arduino Pin Mapping for ATtiny x313 series")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b
RAM  | 128/256 bytes
EEPROM | 128/256 bytes
Bootloader | No
GPIO Pins | 17
ADC Channels | None
PWM Channels | 3
Interfaces | UART, USI
Clock options | Internal 0.5/1/4/8mhz, external crystal or clock up to 20mhz

### Tone Support
Tone() uses timer1. For best results, use pin 12, and 13, as this will use the hardware output compare to generate the square wave instead of using interrupts. In order to use Tone(), you must select Initialize Secondard Timers: Yes

### Initialize Secondard Timers option
When an Arduino sketch runs, prior to Setup() being called, a hidden init() function is called to set up the hardware. One of the things it does is configure the timers. Timer0 (used for millis) is always configured, but on the x313 series, due to the extremely limited flash, it is optional whether to initialize Timer1. If Timer1 is not initialized, Tone() and PWM on pins 12 and 13 will not work. 

### I2C Support
There is no hardware I2C peripheral. The USI can be used to provide I2C support with the TinyWireM or TinyWireS library

### SPI Support
There is no hardware SPI peripheral. The USI can be used to provide SPI support with one of the TinySPI libraries

### UART (Serial) Support
There is one full hardware Serial port, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. Be aware that due to the limited memory on these chips the buffers are quite small. 

### There is no ADC
