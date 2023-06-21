# UniversalWire library
Should provide a mediocre rendition ofthe standard API for most use cases. There is a reason it's not better than this, and it's not exclusively a list of my deficiencies.

## Background: Heterogeneous hardware
The root of most problems with the Universal Wire library is that there are a total of 6 implementations in here - USI master, USI slave, Real TWI master (direct copy from Uno library, I think), Real TWI slave (definitely direct copy from uno version), and for two unfortunate parts, Software I2C master only - though they have slave only hardware implementations too.

So there are three very different hardware systems, each with a separate copy of the library, selected using #ifdefs.

### Standard TWI
Only on the Tiny88. Here, as long as you're able to fit in overall resource limitations, this should really behave just like a '328p.

### Useless Serial Interface (USI)
Some resources refer to this as "Universal". Others refer to it as what you get if you strip out every feature and nicety that we expect from hardware SPI or hardware TWI, and instead of branding it as "crippled SPI/TWI" which doesn't have much of a ring to it, they called it a universal serial interface. It also happens to do very little for you. Essentially, what you get is an 8-bit SPI-like shift register, that is triggered, by USCK/scl, a way to see when 8 bits have been clocked through it, and a data register that that gets copied to at that time. You do get a start condition detector that works to wake from all sleep cycles at least.
You may have noticed that I didn't mention clock generation. That's cause there is none. You have to stobe the pin from software or monopolize a timer to generate a clock, and you cant use timer0 cause that's millis, and if you use timer1, you have only 1 PWM pin left. Hence we do software strobing on the master side,

We are also not allowed to use the internal pullups:
To quote the datasheet:
"The SCL line is held low when a start detector detects a start condition and the output is enabled. Clearing the Start Condition Flag (USISIF) releases the line. The SDA and SCL pin inputs is not affected by enabling this (two wire) mode. **Pull-ups on the SDA and SCL port pin are disabled in Two-wire mode**"

(emphasis mine)

Now, the internal pullups are NEVER strong enough for reliable opperation except under the most favorable conditions, but those conditions are common enough that people will be dissappointed to find that configurations that worked without external pullups require them here. They should not be upset at that, they should be upset with the stock API which hid the critical defect (lack of external pullups) from them. The pullups should be like 4.7-10k at 5V normal speed, and lower values (stronger pullups) at 3.3v and/or higher clock speeds.

### Slave Only TWI
The exiting quartet - 841, 441, 1634 and 828 have a slave only TWI. The 1634 also has a USI, though. So that uses USI TWI master and HW slave TWI as slave.

So 3 of those have only HW slave TWI. Most people consider TWI master to be non-negotiable must-have, since you literally must have a TWI master to use so many devices.

So on these devices, we have the worst I2C master of all: Software I2C. Not only is there no clock generation, since there's no dedicated hardware, bits are clocked out through the Wire.EndTransaction and Wire.request() purely by software manipulation of pins, and we also can't guarantee that it will read correctly in the absence of sufficient setup and hold time. It does not deal very well with clock stretching, and bus errors are not well reported.

It might be possible to achive modest improvements on the master behavior in these cases, but I do not expect to be able to work on that.

On the 828, due to an erratum that will never get corrected, you must have the WDT running (interrupt mode with the interrupt being declared as an empty ISR is recommended). If the WDT oscillator is not running one of the TWI lines will be continually pulled low, rendering the bus unusable.

## Standard Wire API is implemented
All 6 of these implementations do the best they can to provide the same API. The USI has recently been reported to work as both master and slave, and we know the normal-TWI works, so that leaves the situation as master on the 841, 1634, and 828 a bit murkier.

Caveats:
* On 841, 441, and 828, errors are not always properly recognized.
* On 828, the ULP (hence WDT) must be on if slave mode is used, otherwise one of the I2C pins is pulled low with greater strength thatn the pullups can counteract.
* Wire.setClock() has no effect on 841, 441, and 828.
* 828 Wire Master pins are different from normal to get away from the bugged pin.


## Be sure you understand how slaves work in the Arduino API
Yes, this is how the official API demands it be done. Yes, this does preclude the normal register model.

When the master *writes* to the slave:
1. Master generates start condition
2. Master clocks out the 7 bit part address and the Write bit
3. Slave ACKs
4. Master clocks out 8 databytes
5. Slave stores in buffer and ACKs
6. repeat 4-5 until master is done sending or slave's buffer is full causeing it to NACK.
7. Master generates a stop condition.
8. Slave's onReceive handler is called, being passed the number of bytes received.

When the master *reads* from the slave:
1. Master generates start condition
2. Master clocks out the 7 bit part address and the Read bit
3. Slave stretches the clock and calls onRequest. onRequest must put all the datathe master might want to read into the buffer with Wire.write(). You are not told how many bytes the master will attempt to read.
4. Slave stops stretching the clock and ACKs.
5. Master generated 8 clocks on SCL; slave clocks out 1 byte
6. Master either ACKs or NACKs
7. If master ACKs return to step 5. Otherwise, done.

### How normal I2C devices work
Now normal I2C devices have a defacto standard on top of the protocol:
Each slave device has and an array of special function registers (like the ones we have on an MCU, and an address space typically not more than 8 bits in size (though exceptions exist, for example, EEPROMS don't so much have registers as addresses to store data, but they use the same model.)

On these devices, when the master writes:
1. Master generates start condition
2. Master clocks out the 7 bit part address and the Write bit
3. Slave ACKs
4. Master clocks out the address. Slave sets the pointer to this and acks (rarely, the second byte is also part of the pointer address (usually encountered with EEPROMs))
5. Master clocks out one byte of data to write tothat address (most devices autoincrement, a few dont)
6. Slave writes it to that location in the virtual register table and acks
7. Repeat 5 and 6 until all data transferred.
8. Master generates a stop condition

For a read, the master would first perform the first 4 steps above setting the location they want to read from, then either send a stop, then start condition or a repeated start and then reads per steps 5 to 8 of the way the Arduino API provides.

### Yeah, they don't line up so good.
It's like the API designer read the protocol spec and designed to that and had never actually used an I2C device. You cannot have a register-model, because you don't know how many bytes the master will read (the protocol never tells you this), nor can you find out how many are read after the fact, nor can you put the slave to sleep because it might be silently servicing an interrupt for a read that hasn't finished yet. This will generally make you "that device" that when misused, becomes non-responsive with one or both being held low. You don't want to be that device.

The API has been extended for DxCore and mTC, as the problem was far more tractable there (not only is it a single implementation covering a much more cooperative peripheral, the same library works unmodified and likely will continue to do so for the forseeable future.
