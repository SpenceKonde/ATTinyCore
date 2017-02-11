#UNDER CONSTRUCTION - USE THE OTHER VERSION (see link below) NOT THIS ONE!
http://github.com/JChristensen/tinySPI  
ReadMe file  
Jack Christensen Oct 2013  
Leonardo Miliani Nov 2014

####CC BY-SA-NC
This work is licensed under the Creative Commons Attribution-ShareAlike NotCommercial 4.0
Unported License. To view a copy of this license, visit
http://creativecommons.org/licenses/by-sa-nc/4.0/ or send a letter to Creative
Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.

##Description
*tinySPI* is an Arduino SPI master library for ATtiny24/44/84/25/45/85/2313/4313 that utilizes the USI
hardware in the ATtiny MCUs.

Because the USI hardware is not as sophisticated as the dedicated SPI hardware
in the ATmega MCUs, *tinySPI* is more limited than the [standard Arduino
SPI library](http://arduino.cc/en/Reference/SPI) but still has several advantages over software bit-bang approaches.

As compared to the [Arduino shiftOut() function](http://arduino.cc/en/Reference/ShiftOut), *tinySPI*
- is about 15 times faster,
- has a smaller program memory footprint, and
- has a consistent bit clock period.
 
*tinySPI* does not support
- sending least-significant bit first,
- SPI data modes 2 and 3, or
- various SPI clock frequencies.
 
*tinySPI* will generate an SPI bit clock that is about one-tenth the MCU
system clock frequency. For example, given a 1MHz system clock, the SPI clock
will be about 100kHz, and it will take about 80µs to transfer one byte. 

Interrupts are inhibited during each byte transfer to ensure a consistent
bit clock period.

Included with the library are example sketches that demonstrate driving a 74HC595 shift register from an ATtiny84 and from an ATtiny85.

*tinySPI* was developed with [Arduino 1.0.5](http://arduino.cc/en/Main/Software) and the [Arduino-Tiny](http://code.google.com/p/arduino-tiny/) ATtiny core 0100-0017.
Actually, it compiles well with Arduino 1.0.6 and ATtiny core 0100-0018 and with Arduino 1.5.8 and ATtiny core 0150-0020.

##Methods
* begin()
* end()
* setDataMode()
* transfer()

###begin()
#####Description
Initializes the SPI bus, sets the USCK and DO pins to outputs, sets the DI pin to input.
#####Syntax
SPI.begin();
#####Parameters
None.
#####Returns
None.

###end()
#####Description
Disables the SPI bus (leaving the pin modes unchanged).
#####Syntax
SPI.end();
#####Parameters
None.
#####Returns
None.

###setDataMode()
#####Description
Sets the SPI data mode, i.e. clock polarity and phase. See the Wikipedia article on SPI for details.
#####Syntax
SPI.setDataMode(mode)
#####Parameters
mode: SPI_MODE0 or SPI_MODE1 (modes 2 and 3 are not supported)
#####Returns
None.

###transfer()
#####Description
Transfers one byte over the SPI bus, both sending and receiving. Interrupts are inhibited during each byte transfer to ensure a consistent bit clock period.
#####Syntax
SPI.transfer(val);
#####Parameters
val: the byte to send over the bus _(byte)_
#####Returns
the byte read from the bus _(byte)_
