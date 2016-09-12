
### ATtiny 828
![828 Pin Mapping](http://drazzy.com/e/img/PinoutT828.jpg "Arduino Pin Mapping for ATtiny 828")

### Clock options
The ATtiny 828, in the interest of lowering costs, does not provide support for using an external crystal as a clock source, only the internal oscillator (at ~8 or ~1mhz) or an external *clock* source. Note that using an external clock source is not an option in the board drop-down menus, so you cannot set it that way with "burn bootloader" from within the IDE - you must do it manually (this is to prevent new users from accidentally bricking their parts). 

### Internal Oscillator voltage dependence
The internal oscillator is factory calibrated to +/- 10% or +/- 2% for the slightly more expensive 828R. +/- 2% is good enough for serial communication. However, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work. This would interfere with uploads using the bootloader - to work around this, a version of Optiboot is included built assuming the slightly higher operating frequency; this will be used if you select the >4.0v operating voltage prior to doing Burn Bootloader. 

### I2C Support
There is no I2C master functionality implemented in hardware - you must use a software implementation like SoftI2CMaster for I2C master. I2C slave functionality is available in hardware (use WireS library)

### SPI Support
There is full Hardware SPI supply. Use the normal SPI library. 

### UART (Serial) Support
There is one hardware serial port, Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. 

### ADC Reference options
Despite having 28 ADC input channels, the 828 only has the two basic reference options. 

* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
