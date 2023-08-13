# Changing PWM frequency

## Classic AVR
This is concerned with Classic AVRs (those released before 2016), specifically those supported by ATTinyCore. The ATmegas use essentially the same timer1, (usually with the normal 2 outputs, occasionally with a third compare channel) - but they also typically have a very different timer2 than we get. Because this is targeted at ATTinyCore users, we will not be discussing the Async timer2; however for ATmega parts with multiple 16-bit timers that are copies of timer1 (usually timer 1, and 3 through 5 if present are this kind of timer).

Links to resources for modern AVRs are linked below.

You can change the PWM frequency to a limited number of values on **Timer/Counter1** and (if present) **Timer/Counter2** by simply adjusting the prescaler *while still using analogWrite()*. A much wider range of frequencies are accessible if you take full control of the timer, but in these cases, you would need to change registers that analogWrite() assumes a computationally convenient value is stored in, and analogWrite() would no longer work.

Note that since Timer/Counter0 is used for millis, no registers of Timer0 should ever be written unless millis is disabled from the tools menu.

### Background: How timers work
In a timer counter unit, when we're talking about PWM, we're talking about the system consisting of:
* A clock source (almost universally, the system clock is used, however, the x5 and x61 families can also use an on-chip PLL to kick the clock speed up by a factor of 4 or 8 (The 8x mode, followed by a fixed 4x prescaler, can be used as the system clock on these parts, but not on most parts). This is the timebase used for everything else. The increments of time will be herafter referred to as "clock ticks"
* A prescaler - This would, thus, if set to prescale by 8, count 8 clocks, drive the timer clock high, count 8 more clocks, drive timer clock low. The interval of one low and one high period of this output is hereafter a "timer tick".
  * This is essentially a counter, counting unprescaled "clock ticks", but rather than being accessible as a register or doing the things the business end of the counter does, the only output facility consists of lines connected to a subset of bits within the register, going into a multiplexer.
  * When you select a prescaler, you are simply choosing which output of the prescaler to use.
* A counter - This is the counter that you have access to. It counts prescaled timer ticks. It can count up or down, depending on the timer mode - usually up. The timer can also generate an several types of interrupts (exact options depend on part)
  * Input Capture: Fires an interrupt under some condition (usually a pin state) is observed - *and copies the count to the ICR1 register at the moment the interrupt flag is set*. That last part allows you to escape the limits on timing of external events that would apply if you were using Pin Change or Int pins What happens if you have interrupts disabled? If you, for example, need to divide a long, and just can't wait until the next pass through loop to do it, well, that's going to be >600 system clock cycles. At 16 MHz, *that's nearly 40us just for a single division of a 32-bit value* - my point here is that interrupts can be disabled for far longer than an acceptable amount of scatter in the error of the of the measurements, which is what you would get with a PCINT that reads millis or micros. But with input capture, the timer itself saves the value, so you just need to fish it out before the next one happens.
  * A Compare module - The timer has several "OCR" (output compare register) registers, depending on how many channels it has, and each one is the width of a the counter. These hold the values - one per channel - against which count is compared - for ech of the the 0-3 (on classic AVRs) channels. The result of this comparison can be set to trigger an interrupt upon a match. Most cores will initialize the timers to count at between 490 and 1960 Hz.
    * Output Compare is the part this document is concerned with: Output compare is the mechanism that flips the designated output pin on a compare match. In fast PWM mode, it sets the pin at BOTTOM, clears it at the compare match, and then sets it again when it overflows from TOP to BOTTOM. In Phase Correct mode, it counts up to TOP like normal, and that's when the double-buffered registers update - but then it starts counting down, setting the pin when it passes the compare value, then repeating once it reaches BOTTOM. In "Phase correct" mode, the double-buffered registers change at top, which can throw off the second half of a ramp. On "Phase and frequency correct" PWM, the update occurs at the bottom, reducing glitches caused by changing the period during operation.
    * Depending on the part, there may be several different modes available for the timer to generate PWM via. Some of these only count in one direction, but most parts support at least phase correct PWM.
    * On Timer 1 (which is not used for millis), you can optionally get an additional /2 prescaling by using phase correct mode. Many cores copy the official core and always use this mode, but this can slow the PWM down to speeds we might prefer it avoid - or slow them down **from** speeds we'd like to avoid. by being willing to change this mode, it becomes much easier to stay within the target PWM frequency of 490-980 Hz, or at absolute most 1960 Hz (chosen for a number of reasons - it's fast enough that you can blink an LED and it won't appear to flicker, but slow enough that you can PWM most power MOSFETs with just a small gate resistor while staying in spec. At higher frequencies, microcontrollers begin to have trouble driving the pins fast and hard enough to switch the pin without the use of a gate driver), since on most parts the prescaling options are  0 (timer off) 1, 8, 64, 256, and 1024. On the x5 and x61, Timer1 has prescale options of 0, 1, and 2^n where n can be any integer up to 14 (ie, divide by 16384). On parts where timer1 doesn't have that crazy prescaler, which easily gets us into our target range for any frequency, we will choose fastPWM or phase correct PWM if it gets us closer to the target range.
* When full control of the timer is taken the additional functionality might include (briefly - these features are beyond the scope of this document):
  * Periodic interrupts with CTC (Clear Timer on Compare match)
  * Arbitrary TOP values, sometimes without the loss of an output
  * Higher resolution on Timer1 and Timer2 (where present) - those are both mostly standard 16-bit timers.
  * Asynchronous low speed opperation from an external clock or watch crystal (x7, possibly a small number of others)
  * Asynchronous high speed operration from a x8 PLL on the 8 MHz internal osc. (x5, x61 26 only)

### What can I change without breaking analogWrite() or other core API functions?
* TC0 cannot be reconfigured at all without breaking millis, and should only be used if millis is disabled from the tools submenu.
* TC1 has a small number of options that impact the frequency of PWM being output which do not impact analogWrite(), depending on the part
* TC2 on the few classic tinies that have one, works just like TC1

#### Well behaved 16-bit timers
These can be found on all parts except:
* t43, though most of what is described works on a t43, though the timer we get kinda sucks. (Really lame 8-bit timer like TC0 instead of a 16-bit one - it has 3 WGM bits instead of 4, and only 6 of the values do anything (0:Normal, 1:phase correct 8-bit PWM, 2:CTC, 3:fast 8-bit PWM), 5: Phase correct PWM, OCB2 only, OCR1A holds TOP. 5: fast PWM, OCB2 only, OCR1A holds TOP - that odd pattern is a legacy of the time when AVRs commonly had a PWM bit in TCCR1A in place of WGM10, and thus, when that became part of WGM, some parts had their datasheets "clarified" to use the new names, used the remaining 2 or 3 bits to describe various parameters), but otherwise very similar to the 16-bit timers - the prescaler options are the same, and the CS1n bits and the three WGM bits present line up with their usual locations).
* t26, x5, and x61 - these have an 8 or 10 bit high speed timer instead.
* Many parts do not have the full complement, or the same suite of WGMs available,

For this majority of classic AVRs parts, ("normal" ones), the prescaler can be set to one of 7 speeds - or off.  is set by changing the 3 low bits of TCCR1B in accordance with below (or TCCR2B for parts with the second 16-bit timer), eg:
```c
uint8_t tccr1b = TCCR1B; //copy this out so changes are smaller and more efficient, otherwise we would have to write and reload it before proceeding.
tccr1b &= ~0b00000111; // clear  the low bits.
tccr1b |=  0b00000001; // set the one low bit(s) we want - this sets the timer to run at the system clock speed (ie, far too fast for most PWM)
TCCR1B = tccr1b;
```

You can also change the Waveform Generation Mode (WGM)) - on a normal 16-bit timer this is 2 bits each in TCCR1A (0bxxxxxx10) and TCCR1B (0xbxxx43xxx) to another 8-bit mode (including the frequency and phase correct PWM option where TOP is specified by ICR1, if and only if ICR1 is set to exactly 255. You should probably at a minimum disable interrupts, and stop the timer, write TCCR1A, then TCCR1B, then reenable interrupts. . Depending on the application, you may also want to zero out TCNT1.

Consult the datasheet for the list of WGMs that your timer supports and the values corresponding to them.

```c
// switch to Phase and Freq Correct mode with ICR-defined TOP, and set TOP to 255;  This will halve the PWM speed if it's not already set phase correct.
// You may combine these two examples to change both the mode and prescaler.

uint8_t tccr1b = TCCR1B; //copy this out so changes are smaller and more efficient, otherwise we would have to write and reload it before proceeding.
tccr1b &= ~0b00011000; // clear  bits.
tccr1b |=  0b00001000; // set new WGM bit - skip if all bits are 0, ofc
uint8_t tccr1a = TCCR1A; //
tccr1a &= ~0b00000011; // clear bits
tccr1a |=  0b00000010; // set new WGM bit - skip if all bits are 0, ofc.
uint8_t oldsreg = SREG; // save the sreg (cli() then sei() only works in the special case where you are certain interrupts will always be enabled when the code runs, otherwise we would accidentally turn them back on)
cli();  // turn off interrupts so we can't have an interrupt firing while we're between writing values;
TCCR1B = 0; // first stop timer
ICR1 = 255;
// TCNT1 = 0; // clear count
TCCR1A = tccr1a; //assign the value
TCCR1B = tccr1b;
SREG = oldsreg;
```


The mode and prescaler are really the only options that can be changed without losing analogWrite() (full takeover of timers is beyond the scope of this document. )

#### Disagreeable high speed timers
The high speed timers have a few bits of odd behavior to them, the most relevant being that because they are asynchronous timers, if you clear an interrupt flag, and then *immediately* enter a while loop that spins until that flag is set again, it may immediately exit, because you need to allow several clock cycles for synchronization to occur before the flag reads as not set. Whereas in modern AVRs, they were very explicit about synchronization and how it worked, and had protections in place to make sure you didn't own yourself with it. These timers are also markedly more constrained in the selection of modes they can use, while having an incredible prescaler selection - overall, however, they are notably less cooperative than the 16-bit sync timers most parts have.

When either of these timers used to generate PWM, which is what they're made for, OCR1C is TOP (though this defaults to 255), while OCR1A, B, and D if present are the output channels.

For the Tiny x5, you again have two swiches available that impact the frequency. The Tx61 does as well, but also lets you choose one of the whopping 4 WGMs available; all of these can be done without losing analogWrite(). Either way, the prescaler is 4 bits now (0 = stopped, otherwise, the prescaler is 2^(n-1))

The bits are located in TCCR1 on the tiny85:
```c
uint8_t tccr1 = TCCR1; //copy this out so changes are smaller and more efficient, otherwise we would have to write and reload it before proceeding.
tccr1 &= ~0b00001111; // clear  the low bits.
tccr1 |=  0b00001101; // set the one low bit(s) we want - divide by 4096 ()
TCCR1 = tccr1;
```

And TCCR1B on the x61:
```c
uint8_t tccr1b = TCCR1B; //copy this out so changes are smaller and more efficient, otherwise we would have to write and reload it before proceeding.
tccr1b &= ~0b00001111; // clear  the low bits.
tccr1b |=  0b00001101; // set the one low bit(s) we want - divide by 4096 ()
TCCR1B = tccr1b;
```

On both timers, you can also engage the PLL and run the device from either 64 or 32 MHz base clock. This will increase the frequency by a factor of 8, or a factor of 4 in LSM (Low Speed Mode, required to be in spec below 2.7V or so). This is controlled by PLLCSR:
```c
uint8_t pllcsr = PLLCSR;
if !(pllcsr & (1 << PLLE)) {
  // PLL not enabld, gotta do that first.
  // skipped if the PLL is already on, for example because the PLL/4 system clock is in use.
  pllcsr |= 2;
  PLLCSR = pllcsr;
}
if (!(pllcsr & (1 << PLOCK))) {
  // then we need to wait for the PLL to lock.
  while(!(PLLCSR & 1));
  // wait until PLOCK readonly bit is set.
}
pllcsr |= 1 << PCKE; // No low speed mode
//pllcsr |= (1 << PCKE) | (1 << LSM); Low speed mode.
PLLCSR = pllcsr; //finally, enable the PLL as timer clock source.
```

On the x61, you can change the WGM - there are only 2 bits, and they're the two LSBs of TCCR1D, though unless you're driving a three-phase BLDC motor, only 2 of them are useful - 00 is fast PWM, and 1 is 01 is phase and frequency correct, which is what the core uses. The rest of the register is concerned with the fault protection feature (which is far beyond the scope of this document), and if you're not using that, you can use simple assignment to write a 0 or 1 to that register.


```c
// switch to Fast PWM mode (we normally configure this timer for phase correct mode. );  This will halve the PWM speed if it's not already set phase correct.
// Use the very high prescaler of 4096, but use the PLL to kick the clock speed up by a factor of 8;

uint8_t tccr1b = TCCR1B; //copy this out so changes are smaller and more efficient, otherwise we would have to write and reload it before proceeding.
tccr1b &= ~0b00001111; // clear  bits.
tccr1b |=  0b00001101; // set new prescale bits
// Turn on the PLL
uint8_t pllcsr = PLLCSR;
if !(pllcsr & (1 << PLLE)) {
  // PLL not enabld, gotta do that first.
  // skipped if the PLL is already on, for example because the PLL/4 system clock is in use.
  pllcsr |= (1 << PLLE);
  PLLCSR = pllcsr;
}
if (!(pllcsr & (1 << PLOCK))) {
  // then we need to wait for the PLL to lock.
  while(!(PLLCSR & 1));
  // wait until PLOCK readonly bit is set.
}
pllcsr |= 1 << PCKE // ready to write this one now
uint8_t oldsreg = SREG; // save the sreg (cli() then sei() only works in the special case where you are certain interrupts will always be enabled when the code runs, otherwise we would accidentally turn them back on)
cli();           // turn off interrupts so we can't have an interrupt firing while we're between writing values;
TCCR1B = 0;      // first stop timer
PLLCSR = pllcsr; // enable the PLL as timer clock source.
TCCR1D = 0;      // assign the value
TCCR1B = tccr1b; // and write TCCR1B last
SREG = oldsreg;
```
#### Table of available prescalers on most classic tinyAVRs

| Prescale | Almost all |    tx5/x61 |
|----------|------------|------------|
|        0 | 0b00000000 | 0b00000000 |
|        1 | 0b00000001 | 0b00000001 |
|        2 |          - | 0b00000010 |
|        4 |          - | 0b00000011 |
|        8 | 0b00000010 | 0b00000100 |
|       16 |          - | 0b00000101 |
|       32 |          - | 0b00000110 |
|       64 | 0b00000011 | 0b00000111 |
|      128 |          - | 0b00001000 |
|      256 | 0b00000100 | 0b00001001 |
|      512 |          - | 0b00001010 |
|     1024 | 0b00000101 | 0b00001011 |
|     2048 |          - | 0b00001100 |
|     4096 |          - | 0b00001101 |
|     8192 |          - | 0b00001110 |
|    16384 |          - | 0b00001111 |
| Ext, fall| 0b00000110 |          - |
| Ext, rise| 0b00000111 |          - |
|  bitmask | 0b00000111 | 0b00001111 |



## Modern AVRs
This document is not concerned with the modern AVRs.

### tinyAVR 0/1/2 and megaAVR 0
 * [tinyAVR 0/2/1-series TCA](https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/Ref_Timers.md)
 * [tinyAVR 1-series TCD](https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/Ref_Timers.md)
 * MegaCoreX and the official Arduino Nano Every core have a PWM implementation for TCBs. See notes below.

### AVR Dx, Ex-series
 * [Ex/Dx TCA/TCD](https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/Ref_Timers.md)
 * [Dx TCD](https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/Ref_TCD.md)
 * Ex TCE - details still unknown. 16-bit 4-channels, no split mode, but possible enhancements from WEX and HIRES to possibly get higher resolution or and additional features. This is going to take some serious study of the datasheet, and this chapter is gonna be a doozy.
 * Ex TCF - details still unknown. 24-bit pulse or squarewave generator clockable from PLL, but if you want real PWM, you have to put it into dual channel 8 bit pwm mode.

### TCB PWM notes
* On the tinies, the extra TCB pins are mostly on existing TCA pins, and so these are not particularly useful, and the TCBs are bad at pwm, period - whereas whenever any value passed to analogWrite() *or digitalWrite()* as a pin number, if that isn't compile time known (as in, if the value is determined by anything that isn't constant, OR is constant but in a way that the compiler can't assume anything about, it has to be able to react an arbitrary value; (that is, if you have a switch case with 4 options, LTO will optimize those at link time, and see that "Oh this function only gets called with the first two options, and there's no way the third or fourth can be invoked" (before LTO it couldn't do this sort of thing across files, but now it can)). This is more of a chore on the modern AVRs than it was on classics - I didn't think it was worth it for a single additional PWM pin on high pincount tinyAVRs, considering the overhead).
