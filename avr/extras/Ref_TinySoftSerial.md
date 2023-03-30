# TinySoftSerial ("Serial" on parts with no hardware serial port)
Many of the classic tinyAVR parts lack a hardware serial port. This is obviously inconvenient when the majority of us do our debugging entirely over serial (a small number - not including myself - are more sophisticated and know how to do hardware debugging).

Working with the tinyAVR parts without any sort of serial port is quite challenging, and beyond that, a UART is often the only mechanism available for communication with some types of peripherals. There is of course the SoftwareSerial library, but that takes over every single PCINT (to use PCINTs it must take over one port worth, but it permits software serial on ANY pin, so must take over PCINTs on EVERY pin).

## TinySoftSerial and baud rates

The builtin software serial implementation has a rather limited range between maximum and minimum possible baud rates - the "delay" loop used for timing also has a lower bound on the baud rate that it can generate, because it uses a byte to count down it's iterations. There is no means of error reporting of the normal sort - however you can test if begin() has been called succeessfully with if (Serial)  - this will return false if Serial has not yet been given a valid baud rate. If the port has not yet been "begun", all transmission attempts will fail immediately, with write returning 0.

Clock Speed  | Minimum Baud | Maximum plausible Baud
-------------|--------------|---------------------------
1 MHz        | 600 (322)    | 4800, maybe 9600.
8 MHz        | 4800 (2580)  | 57600
12 MHz       | 4800 (3870)  | 76800
12.8 MHz     | 4800 (    )  | 76800
16 MHz       | 9600 (5161)  | 115200
16.5 MHz     | 9600 (5322   | 115200
20 MHz       | 9600 (6451)  | 115200


Note that there is no particular advantage to the "UART Clocks" when using any software serial implementation, and it is not recommended to use them in these cases, hence they are not listed on the table.

The main takeaway from the above table should be that:
1. There is a rather narrow range of baud rate possible with any given clock. Practical baud rates vary only over less than a factor of 20.
2. All clocks above 1 MHz should be able to do 9600 baud.
3. 16 MHz and up is required for 115200 to work, and that's about the highest you can expect.

Some other things that were observed during this investigation - while some baud rates are more favorable than others, and have a closer match, the "special baud rate" effect is much smaller than with hardware serial.

All software serial implementations make different tradeoffs between code size, supported baud rates, and pin and interrupt usage. This one is designed for minimal overhead,
