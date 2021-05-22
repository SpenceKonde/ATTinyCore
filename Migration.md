# ATTinyCore Migration Guide

This document describes the considerations for users of the Arduino IDE when migrating to ATTinyCore from other hardware packages that support the ATtiny line or other 8-bit AVR devices, and from the ATtiny84 to the ATtiny841.

## To ATTinyCore from other cores

Migration to ATTinyCore from other ATtiny hardware packages is typically straightforward - no code modification is typically required, however there are a number of features that require code changes to take full advantage of.

* Two different pin mappings have been used for the ATtiny84 on different cores. Both are supported by ATtinyCore; be sure to select the correct mapping from the Tools -> Pin Mapping menu.
* In order to support the full range of ADC channels, you must use the A# constants, or the ADC channel number when calling analogRead(), not the digital pin number. You may read from differential ADC channels by specifying channel number specified in the datasheets. You may read from the on-board temperature sensor via analogRead(ADC_TEMPERATURE).
* When using digitalRead/digitalWrite/pinMode, you must use the digital pin number, the A# constant, or the PIN_xn constants (for example PIN_B3 for PB3).
* ATTinyCore features a builtin universal SPI and Wire library - with other cores, you need to use libraries like TinySPI, TinyWire, USIWire, etc, with any libraries for SPI or I2C devices modified to use these libraries. With ATTinyCore, you may simply include SPI.h or Wire.h, and use unmodified libraries (the ones that would be used on an ATMega device). For the '841/441/828, when using I2C, select from the tools menu whether you want to support master, slave, or both - selecting both will significantly increase flash usage when Wire.h is included.
* ATTinyCore features a builtin Software Serial implementation named Serial on all parts except the '841/441/828/1634/2313/4313 (these parts have proper hardware serial). This uses the AIN1 and AIN0 pins (see the part-specific documentation pages for details); while this is named Serial to make it easy to port code from ATmega-based devices, it is still a software implementation: It is half-duplex (attempting to send and receive at the same time will result in sending and/or receiving gibberish), and sending and receiving is blocking (ie, the processor can do nothing else). Unlike SoftwareSerial, this uses the ACO vector instead of PCINTs, allowing PCINTs to be used by the application or other libraries. SoftwareSerial library may be used instead if desired.
* When using a serial bootloader on any part except the ATtiny828, the EERDY vector may not be used by the application (this is not used by the EEPROM library, and is usually not an issue). Additionally, if the part is reset during the very start of a serial upload, the bootloader will be corrupted - if this happens, the sketch will not work and the bootloader cannot be entered. To resolve this, connect an ISP programmer and "Burn Bootloader" again. Neither of these caveats apply when the serial bootloader is not used.
* When using direct port manipulation on the ATtiny828/841/441/1634, pullups are controlled by the PUEx register (see datasheet for details)

## From ATtiny84 to ATtiny841 (and from ATtiny44 to ATtiny441)

The ATtiny841 is largely compatible with the ATtiny84, only with additional features. There are a few differences to be aware of:

* The ATtiny841 has an additional timer, Timer2, which behaves identically to Timer1 - it is a 16-bit timer with all the same functionality.
* The ATtiny841 PWM channels can be remapped to different pins. If this is done, analogWrite() will not work (you must directly manipulate the relevant registers).
* The ATtiny841 allows the SPI and UART pins to be remapped. If this is done, the SPI library will automatically adapt; no other measures are needed.
* The ATtiny841 has hardware SPI and I2C slave, but no USI or hardware I2C master. Instead of using special Tiny-specific SPI/Wire libraries, and libraries modified to use them, use SPI.h or Wire.h - the Universal Wire and SPI libraries included with this core will automatically use the correct implementation of SPI or Wire for the hardware, and work with unmodified libraries (ie, ones meant for ATmega devices with normal SPI/I2C hardware).
* When using direct port manipulation, pullups are controlled by the PUEx register, rather than by DDRx set to input and PORTx set high.

## To ATTinyCore from an ATmega-based device

* ATTinyCore features a builtin universal SPI and Wire library; no changes should be necessary to the code or libraries. For the '841/441/828, when using I2C, select from the tools menu whether you want to support master, slave, or both - selecting both will significantly increase flash usage when Wire.h is included.
* ATTinyCore features a builtin Software Serial implementation named Serial on all parts except the '841/441/828/1634/2313/4313 (these parts have proper hardware serial). This uses the AIN1 and AIN0 pins (see the part-specific documentation pages for details); while this is named Serial to make it easy to port code from ATmega-based devices, it is still a software implementation: It is half-duplex (attempting to send and receive at the same time will result in sending and/or receiving gibberish), and sending and receiving is blocking (ie, the processor can do nothing else). Unlike SoftwareSerial, this uses the ACO vector instead of PCINTs, allowing PCINTs to be used by the application or other libraries. SoftwareSerial library may be used instead if desired.
* When using a serial bootloader on any part except the ATtiny828, the EERDY vector may not be used by the application (this is not used by the EEPROM library, and is usually not an issue). Additionally, if the part is reset during the very start of a serial upload, the bootloader will be corrupted - if this happens, the sketch will not work and the bootloader cannot be entered. To resolve this, connect an ISP programmer and "Burn Bootloader" again. Neither of these caveats apply when the serial bootloader is not used.
* The ATtiny devices have far less RAM than ATmega devices - while it is never a good idea to use Strings instead of c-strings (null terminated fixed length character arrays) on AVRs, this is especially true of ATtiny parts.
* The ATtiny841 has a Timer2 - this is a 16-bit timer, identical to Timer1 on most AVRs. Libraries meant to work with Timer2 (an 8-bit async timer) on the ATmega devices will not work here. Other ATtiny parts supported do not have a Timer2 at all. Timer2 is used for Tone on the 841/441 to leave Timer1 open for other uses.
* The Timer1 on the ATtiny85 and ATtiny861 are special high-speed timers. Libraries meant to work with Timer1 on other AVRs will not work on these parts.
* When using direct port manipulation on the ATtiny828/841/441/1634, pullups are controlled by the PUEx register (see datasheet for details)
