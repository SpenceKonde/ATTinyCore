# This document lists significant changes and bugfixes, including those not in yet in a release.
Any changes listed at the top, without a version number above thenm, have not been in a release yet, but can be downloaded from github for manual installation.

### 1.5.0
* Dramatic improvement in execution time and flash use with constant pins for digitalWrite/etc (thanks @yumkam!)
* Improvements to millis and micros timing, as well as delayMicroseconds particularly for oddball frequencies. (big thanks to @cburstedde!)
* Update to Micronucleus 2.5 upload tool, now available for all Arduino platforms (#465, #477)
* Add new rationalized pinout for x61
* Correct bug with pin definitions for I2C pins on the ATtiny x61 series (#455)
* Improve timing calculations for Wire on USI devices (#455)
* Correct critical regression introduced in 1.4.0 where PWM controlled by timer 1 would have 1/4th of the expected duty cycle and frequency (#470)
* Fix Serial RX on parts that use tinySoftSerial - LTO broke this because the ring buffer was updated in the ISR but none of the variables were declared volatile... (#472)
* Correct name of bootloader file for 841 Micronucleus
* Add missing bootloader options for 841/441 parts
* Correct documentation for x313 parts to correctly reflect number of available PWM pins
* Clarify USBTinyISP programmer options (#507)
* Add several missing programmers, improve programmers list
* Fix Timer1 PLL clock source menu option on x5, x61 (1.4.0 fix still didn't do it!)
* Fix x313 pinout image to reflect OC0B's existence
* Add in new Adafruit_NeoPixel functions to tinyNeoPixel
* Add Serial.setTxBit() for parts that use tinySoftSerial. Call it before Serial.begin() to move the Tx pin to another pin/bit on the same PORT (#443)
* Correct PWM frequency on ATtiny x5 and x61 parts operating below 3 MHz (#463)
* Move PWM on pin 1 (PB1) on x5 to timer1 instead of timer0
* Select /32 prescaler where available when operating with a system clock between 3 and 8 MHz, moving PWM on those pins back to the targeted 500 Hz to 1 kHz range
* Correct WGM for Timer 1 PWM on x7 to phase correct (0b0001) rather than fast (0b0101) pwm modes

### 1.4.1
* Correct severe regression in timekeeping on 841, 441, 1634, and 828 in version 1.4.0 (#445, 447)
* Correct issue with included bootloaders for MH ET tiny88 micronucleus boards actually not working... re-bootload with ISP programmer to fix with the new bootloaders. (#439)
* Add full suite of entry mode bootloaders for tiny88 micronucleus.
* Fix issue with upgrading bootloader with Micronucleus.
* Correct with timekeeping on x61 parts (1.4.0 regression in timer initialization).
* Use learnings from Micronucleus f/w source code to safely change internal oscillator by dramatic amounts on the t841 (and others, but most relevant here)
* Change "USB tinyISP" to "USB tinyISP FAST", and increase bit clock significantly. Previously we had the normal version at 200kHz and the SLOW at 125kHz, which was redundant, and made the USBtinyISP agonizingly slow, even with the tiny memory of these parts.
* Add in ASCII art pinout to pins_arduino.c for TinyX8_MH pin mapping (#434 - thanks @AingeruJM!)

### 1.4.0
* Add support for MH-Tiny T88 pin mapping.
* Add support for MH-Tiny T88 boards, because I just got a bunch and it didn't look too hard.
* Add support for the DigiSpark Pro pin mapping for the ATtiny87/167 with other bootloader options.
* Trim out references to unsupported parts from Servo library. I am almost positive it did not work on a significant number of parts! I think it should generally work now...
* Servo will clean up after itself if you detach all the servos.
* Fix ATtiny2313/2313A related issues (getting the #defines from the headers for the A, while still being able to upload)
* PLL as Timer1 clock source now actually enables the PLL.
* noTone() now correctly cleans up after Tone().
* Signifiant cleanup in wiring.c
* Included Servo library now works with PLL as Timer1 clock source (trivial fix - I clearly never looked into it when I merged in the 8-bit servo code!). Now it also lets you use Servo if you have external crystal running at unsupported speed if you use the PLL for Timer1...
* Major improvements to part-specific documentation pages.
* Remove the HAVE_BOOTLOADER mechanism, which existed to clean up timer registers that a bootloader may have left configured. Neither Optiboot not Micronucleus do this, so HAVE_BOOTLOADER had no function. Is now commented out in the pins_arduino's where it existed, as is the code it enabled. Saves a small amount of flash, and makes init() in wiring.c far easier to follow once the do-nothing code isn't visible.
* Correct a number of typos in boards.txt
* Fix several missing optiboot bootloaders
* Add support for setting CLKPR so bootloaders off internal at speeds other than 8 and 1 MHz can work at less agonizing baud rates.
* Fix inverted LED blinking on all parts
* Fix LED blinking on x61 family (#264)
* Add support for VUSB uploads to Digispark (t85, t167), Micronucleus/California STEAM (t84a)  Wattuino (841). Requires board manager installation, or another compatible board, in order tp pick up the support files.
* Add support for 16 MHz with *INTERNAL* oscillator on ATtiny841, 441. Support is still experimental; there are a few caveats - see [ATtiny441, 841](avr/extras/ATtiny_x41.md)
* Tested voltage dependence of internal oscillator, allowing significant simplification of the bootloader files for the ATtiny841/441,828,1643.
* Support PIN_Pxn notation.
* Document a few really odd GPIO features on ATtiny841, 441, 828.
* Document minimum baud rate of builtin software serial "Serial" for parts without hardware serial.
* Remove TUNED_OSCCAL_VALUE defines from pins_arduino.h for variants; this was set to OSCCAL in those files (despite the fact that we tested for whether it was defined before trying to use it!), leading to compiled binaries copying OSCCAL to a register and then writing it back. (thanks @ArminJo)
* Fix bug when switching between I2C master and slave modes on USI devices (thanks @martin-schmied1)
* Fix pulseIn(), both pulse length, and timeout (the latter is broken in official avr core too) (#384)
* Add pulseInLong()
* (untested) Don't generate .lst unless told to 'export compiled binary' (#379 - though note that that issue also appears to involve a problem with their compiler, or maybe just an extremely perverse username that's breaking things)

### 1.3.3
* Add support for external CLOCK on 48/88/828. Document how to use external clock on other parts via manual AVRdude step (#355).
* Fix bug with using 32K ULP as clock source on 828.
* A bit of boards.txt cleanup.
* Serial hogged an unnecessary amount of ram on the 841/441/1634. Pulled in the latest version of HardwareSerial from the official AVR core; this sorts out that issue.
* Add printf support to printable class.
* Add CLOCK_SOURCE define to identify clock source (#358)
* Add support for 4MHz internal on all boards (#358)
* Add support for 16.5MHz on x5 and x61 (#349)
* Documentation improvements (#373, #375)
* Burn Bootloader now executes as a single command; this should fix issues with some programmers. (#372)

Prior to 1.3.3, a proper changelog was not kept. See the releases for information about changes introduced during that timeframe.
