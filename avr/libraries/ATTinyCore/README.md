# ATTinyCore library - ATtiny classic examples and helper functions
This library serves two purposes:
* First, it's original purpose: As a way to get example code specific to the core or the parts supported by the core to the user (there isn't a facility provided for a core to associate examples with the core explicitly), such as using the temperature sensor, or the ADC noise reduction mode, etc.
* Second, it provides a way to add helper functions to take advantage of chip features that don't belong in the core itself, but which are specific to only a few parts, and don't make sense as a separate library either.


## Helper function reference

### Functions specific to the ATtiny85 and ATtiny861 (and their smaller flash versions)
These functions involve configuration of the high speed timer, Timer1, on these parts to generate PWM of different frequencies.

They can be used in two ways: Either you do the math, call setTimer1Clock(), setTimer1Prescale() and setTimer1Top() (this uses less flash), or you can call setTimer1Frequency(). It will do the calculation for you, but it will use more flash.

Once this is done, you have either the number you set the TOP value to, or the value returned by setTimer1Frequency.

You can then use analogWrite() to set the duty cycle, however you must account for the fact that the above noted TOP value is 100% duty cycle. If you want, say, 50% duty cycle, and your TOP is 180, you would analogWrite(pin,90).

#### void setTimer1Clock(TIMERONE_CLOCK_tclk)
This sets the clock source of Timer1 to either the system clock, the full speed PLL at 64 MHz (or 66 when running at 16.5 MHz for Micronucleus/VUSB), or the PLL in LSM mode (not available if the system clock is derived from the PLL)

The argument is an enumerated type, TIMERONE_CLOCK and must be one of these constants:
```text
TIMER1_FCPU
TIMER1_64M_PLL
TIMER1_32M_PLL
```

#### void setTimer1Prescaler(TIMERONE_PRESCALE_t presc)
This sets the clock prescaler of Timer1 to the specified division factor (A "prescaler" divides a clock by some factor before it travels on to whatever thing it is clocking - all else being equal, setting it to TIMER1_DIV8 would result in PWM at 1/4th the frequency of TIMER1_DIV2). Like setTimer1Clock, TIMERONE_PRESCALE_t is an enumerated type, use one of the below constants
```text
TIMER1_STOPPED
TIMER1_DIV1
TIMER1_DIV2
TIMER1_DIV4
TIMER1_DIV8
TIMER1_DIV16
TIMER1_DIV32
TIMER1_DIV64
TIMER1_DIV128
TIMER1_DIV256
TIMER1_DIV512
TIMER1_DIV1024
TIMER1_DIV2048
TIMER1_DIV4096
TIMER1_DIV8192
TIMER1_DIV16384
```

#### void setTimer1Top(uint8_t newtop)
This sets the TOP value for the timer. Each timer cycle counts this many increments of the prescaled clock plus one (it includes zero). Combined with the two above functions it allows you to generate PWM of almost any frequency. You can control the duty cycle using analogWrite() with one caveat: Instead of 254 being "almost 100%" duty cycle, TOP-1 would have that role, and you must scale the values you pass to analogWrite() yourself. You can use map() if you want, but map is a rather inefficient function, and if you can adapt your code to generate numbers between 0 and TOP-1, you will save flash and get better performance.

#### uint8_t setTimer1Frequency(uint32_t target)
Alternately, if you don't want to figure out what prescaler, clock source, and TOP value is needed for the frequency you want, you can call this, passing the target frequency in Hz. It will always try to find the maximum possible TOP value (hence greatest resolution) that will give the requested frequency. That value is what is returned by this function; you must retain that number and use it to calculate duty cycles passed to analogWrite!


#### void setTimer1Top_10bit(uint16_t newtop) *(x61 only)*
This works like setTimer1Top() above - except that TOP can be up to 1023, running the timer in 10-bit mode. To set the duty cycle, however, you must use setTimer1DutyCycle_10bit().

#### uint16_t setTimer1Frequency_10bit(uint32_t target) *(x61 only)*
This works like setTimer1Frequency - except again, TOP is not constrained to 1023, since the timer can act as a 10-bit timer. setTimer1DutyCycle_10bit() must be used to control the duty cycle.

#### void setTimer1DutyCycle_10bit(uint8_t channel, uint16_t duty) *(x61 only)*
Unlike analogWrite, this expects a channel number (to simplify the implementation of this function and minimize overhead). Channel must be 0, 1, or 2.
* Channel 0 is OC1A, on PB1.
* Channel 1 is OC1B, on PB3.
* Channel 2 is OC1D, on PB5.

Like analogWrite, passing 0 or 1023 will call digitalWrite and turn off the PWM, and passing any other number will turn on the PWM.

Unlike analogWrite, this does not set the pin OUTPUT. It is your responsibility to do that prior to calling this if you want it to actually give you PWM.

## Chip feature grab-bag
These return 0 on success, 1 on failure, and throw an error if called on a part that doesn't have the feature at all.

### uint8_t enableHighSinkPort(uint8_t port, bool mode)
You can enable port C (`PC`) as a high sink port with this on the tiny828 only - it's no great shakes and the same absolute maximums apply, but for a given current, PORTC can drive lower than other pins, and even lower if you enable this.

### uint8_t enableHighSinkPin(uint8_t pin, bool mode) {
Same deal as above, but this works on PIN_PA5 and PIN_PA7 on the ATtiny841 and 441 only.

### void enableISRC(bool mode)
The tiny 87 and 167 have an internal current source (like a very accurate pullup) intended for LIN addressing but usable for many purposes. This will enable and disable it. The datasheet discourages one from leaving it on for an extended period of time. No return value, only the error if called from a part that doesn't have it, because there is no condition under which the current source is not available. The current source on the x7's is located on PA3.

### void disableAllPullups(bool mode)
Many parts have the option to disable all pullups on the chip, if they for some reason cause problems for your application (this would be very uncommon). This simple wrapper sets or clears the PullUpDisable bit.

### void disablePortPullups(uint8_t port, bool mode)
A small number of parts allow port-wise disabling of pullups. This sets and clears it those bits for you.  If either the global or the port PUD bit is set, the pullups are disabled.

### uint8_t enableBBM(uint8_t port, bool mode)
Some parts support "Break-Before-Make" switching, which inserts 1 system clock of tristate on a pin when the DDR register is changed. I think the point is so that you can turn one pin into an output and the other to an input with a single write to DDRx, even if any actual overlap would be problematic? I dunno, I've never heard of anyone using this. Returns 0 on success, 1 on failure (ie, the port didn't exist or doesn't have a BBM bit), and errors if there's no port control register at all.
