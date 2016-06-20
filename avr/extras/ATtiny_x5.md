
### ATtiny 25/45/85
![x5 pin mapping](http://drazzy.com/e/img/PinoutT85.png "Arduino Pin Mapping for ATtiny x5 series")

### PLL Clock
The ATtiny x5 series parts have an on-chip PLL. This is clocked off the internal oscillator and nominally runs at 64mhz when enabled. As a result, it is possible to clock the chip off 1/4th of the PLL clock speed, providing a 16mhz clock option without a crystal (this has the same accuracy problems as the internal oscillator driving it). Alternately, or in addition to using it to derive the system clock, Timer1 can be clocked off the PLL - this can be set up automatically using the Timer1 Clock Source option in the tools menu when an x5 or x61 series part is selected. This will effect the frequency of PWM on pin 3.

### Tone Support
Tone() uses timer1. For best results, use pin 3 for tone - this will use Timer1's output compare unit to generate the tone, rather than generating an interrupt to toggle the pin. In this way, tones can be generated up into the MHz range. If timer1 is set to use the PLL clock (provided this is done using the menu option, not manually), Tone will figure this out and output the requested frequency. With timer1 running off the PLL @ 64mhz, tone() should be able to output a 32mhz signal on pin 3!

### I2C Support
There is no hardware I2C peripheral. The USI can be used to provide I2C support with the TinyWireM or TinyWireS library

### SPI Support
There is no hardware SPI peripheral. The USI can be used to provide SPI support with one of the TinySPI libraries

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so it is recommended to keep the baud rate low, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). 
