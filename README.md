# ATTinyCore **Universal**
Arduino support for almost every classic tinyAVR device! Supports ATtiny 1634, 2313/4313, 24/44/84, 441/841, 25/45/85, 261/461/861, 87/167, 48/88, 43, 26 and 828.

Supports programming vis ISP, Serial (Optiboot) or VUSB (Micronucleus)
## [Check it out, we have "discussions" now!](https://github.com/SpenceKonde/ATTinyCore/discussions)
Let's use that, not gitter.

## ATTinyCore 2.0.0 is available for public test. Things that don't work should be reported.

## ATTinyCore 2.0.0 - lots of changes, some of them big, a few of them may cause breakage.
I cobbled ATTinyCore together with far less experience than I have now (indeed, I'd barely covered the basics when I started trying to get a working ATtiny841 core). I like to think I have a much better idea of how a core should be designed now. But this meant some terrible decisions were made in the past. Decisions that we have been paying the price for ever since. I decided that the core should be advanced to a state where the bad decisions have been fixed, and everything that needs to be exposed on the parts is exposed in a consistent manner (too much was done incrementally, and not enough planning was done, ever). This core should not expect any significant new feature enhancements from here on out. The new feature development will be for megaTinyCore and DxCore, as those represent the future of the AVR architecture. Bug fixes will still be made.
The most significant changes are:
1. **analogRead() and channel/pin numbers** ATTinyCore followed what the core it was based on did, which was to use analog channel numbers, not digital pin numbers for analogRead(). Originally, the An-constants were #defined as the number itself. Later, to make them work with the digital IO functions, I changed to  `#define An (n | 0x80)` - digital functions could check for the high bit, and if present, strip it off and use the analogInputToDigitalPin macro to find the digital pin number. I never did the inverse with analogRead() because it would have broken code which used the raw analog channel numbers. This is just absurd in this day and age, where every other core allows you to analogRead() digital pin numbers and it just works. As of 2.0.0, analogRead() takes either a digital pin number, a constant of the form An (one per analog pin, shown on pinout chart! corresponds directly to analog channels), one of the ADC_CHANNEL constants listed in the part-specific documentations pages (these are things like `ADC_TEMPERATURE` and `ADC_INTERNAL1V1`), or - assuming it has a differential ADC - one of the differential channels. If you were generating an analog channel at runtime, you can pass a number through the ADC_CH() macro to get a number that will be recognized as an analog __channel__ number (this is preferred to directly setting the high bit, since it makes clear that you're doing it to get that analog channel). Several people to whom I have spoken about this to
2. **Differential channels!** Yeah - about half of the parts we support have them, and they can be useful for accurately measuring small differences in voltage. These vary per part - some are rather basic, while the t861 and t841 are very fancy - and are listed in full in the part-specific documentation. These are now fully supported. There is also support for analogRead with Noise Reduction mode (chip goes into sleep mode while taking the reading. For parts that support both, there is also a way to select bipolar (-512 to 511) vs unipolar (0 to 1023) mode.
3. All the analog reference sources are named consistently, old (deprecated) names for references are still supported, but not recommended. **potential breakage** If you used to refer to a reference with a raw number, instead of the name (ie, if you did analogReference(0) instead of analogReference(DEFAULT), this will be totally broken for values other than zero (and 0 doesn't have a consistent meaning). The ADC_REF() macro can be used to convert from the REFS bits to a reference constant *if you must - but you shouldn't be using a raw number to select the reference if you want to be able to move it to different parts* - The 1.1v internal reference `INTERNAL1V1` is 2 on some parts, and 0 on others... almost everything has the 1.1v reference and everything can use Vcc, but that's really where the similarities end. If you are writing code that users might want to make work on any other part, you need to use the names, not numbers. With parts having at most 8 options for the analog reference, and most having fewer than that, I do not expect that this is a particularly burdensom requirement. .
4. **Legacy PIN_xn constants gone** `PIN_An` constants have the standard meaning, ie, PIN_An is the digital pin that analog channel `An` is on. Previously some (but to my surprise and horror, not even all) parts had a set of `PIN_xn` constants defined that worked like the new `PIN_Pxn` ones do. I was reading pins_arduino.h from an official core a few weeks after implementing my `PIN_xn` constants for a bunch of parts and discovered that `PIN_An` was already in use meaning something different. Use `PIN_Pxn` where x is the letter of a port, and n is the number of a pin within that port.
5. `PIN_Pxn` constants are in for all supported parts to refer to pins by port and bit. This is the recommended way to refer to pins, as it frees you from the need to consider which pin mapping is in use. If you soldered the LED to pin PB2, PIN_PB2 is going to control the LED no matter which pin mapping you have selected.
6. For various historical reasons, some parts have up to 3 pin mappings. These are now named consistently, and listed and described in the part specific documentation - All parts have a recommended pin mapping, some of them have a second one for a specific VUSB board (digispark pro, MH-ET) with the pins numbered differently, and some of them have a "legacy" pin mapping with the pins in an order that makes less sense, and which makes converting between analog and digital pins harder (as in, if there is stuff determined at runtime, it uses more flash and is slower), but which has been widely used in the past and is what existing code may have been written for. A number of inconsistencies between these pin mappings (where information was missing from one or the other) have been fixed, and they are now formatted and commented consistently.
7. The ATtiny1634 and ATtiny861 are now supported for Micronucleus. Test and demonstration boards will be available from my Tindie store. New versions of bootloader for all existing Micronucleus boards. Users should use the bootloader upgrade functionality to ensure that they have the latest version of the bootloader, and that it has their desired entry conditions (on reset pin, power on reset, power-on with pin held down, reset/power on w/reset held high (in case of disabled reset, holding the reset pin high during power on will make it enter bootloader - takes advantage of the fact that reset PIN bit always reads 0 when reset is not disabled... Actually, one wonders if it would work if you did PORTx |= 1 << RESETBIT; then test if that bit is set - does disabling reset actually make registers not store the value? If so, that would be even better - no dependence on hardware, clear the bit if could set it and run app, otherwise run bootloader. )




### [Installation](Installation.md)
### [Wiring and required external components](Wiring.md)
### [Using with Micronucleus boards](avr/extras/Ref_Micronucleus.md)
### [Programming Guide](Programming.md)
### [Migration Guide - moving to ATTinyCore from a different ATtiny board package](Migration.md)
### Current **strongly** recommended IDE version: 1.8.13


## Supported devices
This core supports the following processors - essentially every "classic" tinyAVR processor that makes sense to use with Arduino. The modern (post-2016 release) tinyAVR parts have [their own core](https://github.com/SpenceKonde/megaTinyCore), as there is practically zero code at the core level that can be shared with classic parts (porting application code (sketches) typically poses fewer challenges - code that did not stray from the Arduino API functions can be ported easily, while code that interacts with registers directly does need to be modified). Porting application code between different parts supported by this core is generally a matter of pin adapting to the different arrangement of peripherals among the pins on the two parts

* [ATtiny441, 841](avr/extras/ATtiny_x41.md) (With or without Optiboot or Micronucleus bootloader) - One of the last classic tinyAVR parts released, and probably the single best of them. This has a second 16-bit timer (like Timer1) giving it a total of 6 PWM channels, two hardware serial ports, analog input on every pin with differential capability with programmable gain. On top of that, if you tune the internal oscillator to nearly the maximum, 90+% of these will run at 16 MHz.
* [ATtiny1634](avr/extras/ATtiny_1634.md)  (With or without Optiboot bootloader - Micronucleus probably coming soon) - One of the last classic tinyAVR parts released. Sporting 16k of flash, 1k of RAM, and dual hardware serial ports spread across it's 18 I/O pins, this is one of the stars of the classic AVR line, though it would have really benefited if it got the same featureset that the 841 did.
* [ATtiny87, 167](avr/extras/ATtiny_x7.md) (with or without Optiboot or Micronucleus bootloader) - originally targeted at automotive applications, the x7-series offers 16 I/O pins, a hardware serial port with an advanced baud rate generator (it's intended for LIN, an automotive communication protocol, which requires flexible baud rate adjustment) and a differential ADC. TimerB is highly flexible about which pins it uses. A Digispark Pro was made with the Micronucleus bootloader (and the second-stupidest pin mapping I have ever seen on an AVR), and those are now available cheaply online. Before you ask - the stupidest pinpapping? That would be the one that the old ATTinyCore-with-167-support used before I took it over.
* [ATtiny25, 45, 85](avr/extras/ATtiny_x5.md) (With or without Optiboot or Micronucleus bootloader) - The most popular tinyAVR, with just 5 I/O pins, and available in a DIP package, this was popular even before the advent of the Digispark which paired it with the Micronucleus vUSB bootloader - those boards have now been widely cloned and are available for dirt cheap. This popularity is in spite of the tightly constrained pincount and weird Timer1 that is incompatible with many libraries. Can run from internal oscillator at 16 MHz using the on-chip PLL.
* [ATtiny24, 44, 84](avr/extras/ATtiny_x4.md) (With or without Optiboot or Micronucleus bootloader) - Probably the second most popular of the classic tinyAVR parts, being available in a 14-pin DIP package. There's nothing particularly exotic or standout about this, but it checks most of the boxes, having 2 PWM channels from each timer (both of which are normal and well-behaved), a modest differential ADC, and so on. No surprises really, either good or bad, and overall exactly what you'd expect from a 14-pin classic tinyAVR.
* [ATtiny261, 461, 861](avr/extras/ATtiny_x61.md) (With or without Optiboot bootloader - Micronucleus probably coming soon) - One of the stranger tinyAVR parts, this was designed with two tasks in mind: making differential ADC measurements, and controlling 3-phase BLDC motors. Timer1 is weird - a 10-bit variation on the high-speed timer that the x5 used (complete with the deadtime generator and PLL), with a third output compare channel. The differential ADC is arguably the best one on a classic tinyAVR (the t841 is the one one that could put up a fight, but lacks a separate AVCC pin, so I can't say I have much faith that the 100x gain isn't just amplifying noise), and until the release of the tinyAVR 2-series in 2021, was in the top tier of ADC capability (that top tier has now been replaced with a top tier composed of the 2-series alone, as they wait for the AVR EA's to join them)
* [ATtiny26](avr/extras/ATtiny_x61.md) (No bootloader. The predecessor of the already long-in-the-tooth x61-series). Added in 2.0.0 by commission from someone with a huge stash of them. Don't use these unless they're all you can get. Two PWM pins, and all the quirks of the x61, only more so. At least it's got a differential ADC that is on the same level as most classic tinyAVRs with differential ADC.
* [ATtiny48, 88](avr/extras/ATtiny_x8.md) (With or without Optiboot or Micronucleus bootloader) - a 32-pin (or 28-pin in DIP) tiny-ified version of the ATmega x8-series. Hardware TWI and SPI, lots of pins, and cheap - but no crystal, no hardware serial, and only timer1 can do PWM (at least it's a normal timer1). It should come as no surprise that the ADC the same boring 8-channel single-ended one the m328p has. Available with Micronucleus support as the "MH-tiny" or "MHET tiny" on eBay and Aliexpress.
* [ATtiny828](avr/extras/ATtiny_828.md) (With or without Optiboot bootloader) - A 32-pin (28 I/O pin) part with hardware serial and an analog input on every pin - and a mysterious and tragic history that left it far less powerful than it could have been.
* [ATtiny2313, 4313](avr/extras/ATtiny_x313.md) (no bootloader) - An expensive, ancient chip with tiny amount of flash. For a long time it was the only tinyAVR with a USART. There are far better choices now.
* [ATtiny43U](avr/extras/ATtiny_43.md) (no bootloader) - An otherwise rather dismal part: 16 I/O pins, but only 2 PWM channels, timer1 is 8-bit not 16, and it only has a few ADC channels - with one unique feature: *an on-chip boost converter* allowing operation from a single, partly discharged, alkaline battery. Unlike using an external boost converter, this can use it's knowledge of the chip state to inform the boost converter's tradeoff between power consumption and accurate regulation, saving significant power while lowering part count. It still requires an external schottky diode, inductor, and capacitor.

Variants of these are also supported (such as the ATtiny1634R, ATtiny2313A or ATtiny85V)

### Non-supported parts
ATTinyCore supports classic ATtiny parts. It does not support any other AVR devices - There is a different core for the ones that are practical for use with Arduino, depending on the product line:
* [tinyAVR 0/1/2-series](https://github.com/SpenceKonde/megaTinyCore/) Modern tinyAVR (with 0, 1, or 2 as next-to-last digit) are supported by my megaTinyCore instead. They are totally different in every way except the "t-word" in the name, and the fact that they're great low-pin-count parts and work well with Arduino.
* [ATtiny13/13A](https://github.com/MCUdude/MicroCore/) are supported by MicroCore by @MCUdude - 2k of flash is the lower cutoff for ATTinyCore.
* The ATtiny28L is not supported by any Arduino core. Ancient, weird, and most importantly, they don't have RAM, just registers. Yeah, seriously.
* ATtiny 4/5/10/11 and any other "AVRrc" (reduced core) parts, these combine minuscule memory sizes with a gimped CPU. [Try this core](https://github.com/technoblogy/attiny10core)
* Anything with "ATmega" in the name - you want [one of MCUDude's cores](https://github.com/MCUdude/)
* AVR Dx-series (AVR128DA64, etc) - [the crown jewel of the AVR product line](https://github.com/SpenceKonde/), supported by my DxCore.

## Quick Gotcha list - having trouble, read these first

**Windows users must install Micronucleus drivers manually**
If you want to use Micronucleus (VUSB) boards on Windows, you must manually install the drivers - Arduino does not run "post-install" tasks for third party libraries, due to "security" considerations. This is of course nonsensical - if the core was malicious, it could do just as much by running the malicious command as part of a compile or upload recipe. I have gotten word that I am not the first person to raise this objection and that restriction will be removed from a future version of the IDE.

During the install process it will print the path of a post_install.bat that it skipped running. Running that will install the drivers - it's easiest if you copy/paste it, as after installation the drivers will be located in `C:\Users\YourUserName\AppData\Local\Arduino15\packages\ATTinyCore\tools\micronucleus\2.5-azd1\`  Or they can be downloaded from the following URL https://azduino.com/bin/micronucleus/Drivers-Digistump(win).zip . Unzip, run the installation batch file.

**This core includes part specific documentation - click the links above for your family of chips and READ IT**
The classic tinyAVR parts are a motley bunch - some of them are very mundane, with little to distinguish them other than the fact that they just work like you'd expect, while others are just bizarre. The part specific documentation covers most of the relevant topics that apply specifically to a given family of parts.

**problems dynamically linking libusb-0.1 on linux** can occur if Arduino was installed through the Snap package manager. The Arduino IDE should always be installed from the tarball available from http://arduino.cc, never from a package manager.

**There are several problems encountered when using versions of Arduino older than 1.8.13**
That version has been out for over a year and a half. While we do not intentionally break things on older IDE versions, we also do not test on older versions.

**Windows store version sometimes experiences strange issues**. The windows store issues are difficult to reproduce on other systems, and no reliable solutions to them are currently known. We recommend using the .zip package or standard installer version of the IDE, not the Windows Store version.

**When using an individual chip for the first time, or after changing the clock speed, EESAVE or BOD settings, you must do "burn bootloader" to set the fuses, even if you are not using the chip with a bootloader**
The option should really be named "Set fuses and install bootloader (if any)" rather than "burn bootloader", which makes it sound irreversible (it's not), and by not mentioning the fuses, many users think that it isn't needed if a bootloader is being used. But it is, because this is the only time fuses are set; this is the same behavior as all other classic AVR cores, because it is possible to soft-brick the parts if the fuses are misconfigured.

**Problems programming some parts for first time, especially ATtiny841/441** These parts are less forgiving of the SCK clock rate being on the high edge of the spec.  Arduino as ISP or USBTinyISP SLOW will program without issue.

**Chips sold as "ATtiny85" with wrong signature** - enable verbose upload output, and it will tell you what sig it actually saw. **0x1e9005** means you got scammed (all 0's or all F's is wiring problem, or bricked chip from choosing clock source that isn't present). Apparently one or more foreign sellers have been remarking the much cheaper ATtiny12 as an 85 and ripping people off on ebay/etc.

**free(): invalid next size (normal) error** This error is due to a bug in AVRdude ( https://savannah.nongnu.org/bugs/?48776 ) - and it's a spurious error, as when it is displayed, the programming operation has actually completed successfully (you can see for yourself by enabling verbose upload, and noting the successful write before this error is shown. It is unknown under what conditions this error appears, though it has been recorded on a USBTinyISP on Linux when bootloading an attiny88 with optiboot.

**When using analogRead(), use the A# constant to refer to the pin, not the digital pin number.** Analog channel number (see table in datasheet entry for ADMUX register) can also be used - unlike the official core, you can use analogRead() with the differential ADC channels (for example).

**When using I2C on anything other than the ATtiny48/88** you **must** use an I2C pullup resistor on SCL and SDA (if there isn't already one on the I2C device you're working with - many breakout boards include them). 4.7k or 10k is a good default value. On parts with real hardware I2C, the internal pullups are used, and this is sometimes good enough to work without external pullups; this is not the case for devices without hardware I2C (all devices supported by this core except 48/88) - the internal pullups can't be used here, so you must use external ones. **That said, you should always use external pullups, even on the 48/88**, as the internal pullups are not as strong as the specification requires.

**You cannot use the Pxn notation (ie, PB2, PA1, etc) to refer to pins** - these are defined by the compiler-supplied headers, and not to what an arduino user would expect. To refer to pins by port and bit, use PIN_Pxn (ex, PIN_PB2); these are #defined to the Arduino pin number for the pin in question, and can be used wherever digital pin numbers can be used. We recommend this method of referring to pins, especially on parts with multiple pinmapping options

**All ATtiny chips (as well as the vast majority of digital integrated circuits) require a 0.1uF ceramic capacitor between Vcc and Gnd for decoupling; this should be located as close to the chip as possible (minimize length of wires to cap). Devices with multiple Vcc pins, or an AVcc pin, should use a cap on those pins too. Do not be fooled by poorly written tutorials or guides that omit these. Yes, I know that in some cases (ex, the x5-family) the datasheet doesn't mention these - but other users as well as myself have had problems when it was omitted on a t85.**

**For low power applications, before entering sleep, remember to turn off the ADC (`ADCSRA &= (~(1 << ADEN))`) - otherwise it will waste ~270uA**

**When in power down sleep mode, the clock is stopped - using a slower clock speed does not save power while sleeping** I am asked very frequently about using very low clock speeds in low-power applications. Almost invariably, upon further interrogation, the user reveals that they plan to have the part in power-down sleep mode almost all the time. In this case, there is often little to be gained from running at a lower clock speed, since it only effects power use in the brief moments that the chip is awake - even more so because these moments may well be shorter when running at a higher clock speed. Clock speeds below 1MHz can present problems while programming due to the programmer using an SCK clock speed too fast for the target chip running at such a low system clock speed.

**When using the WDT as a reset source and NOT using a bootloader** remember that after reset the WDT will be enabled with minimum timeout. The very first thing your application must do upon restart is reset the WDT (`wdt_reset()`), clear WDRF flag in MCUSR (`MCUSR &= ~(1 << WDRF)`) and then turn off or configure the WDT for your desired settings. If using the Optiboot bootloader, this is already done for you by the bootloader.

## ATTinyCore Features

### Optiboot Support - serial bootloader for ATtiny441/841, 44/84, 45/85, 461/861, 48/88, 828, 1634, 87, 167 only)

The Optiboot bootloader is included for the ATtiny441, 841, 44, 84, 45, 85, 461, 861, 48, 88, 1634, 828, 87 and 167. On parts with a hardware serial port, this is used (those lucky parts with two can choose the serial port from a tools submenu), other boards will use a software serial implementation (on these boards TX/RX pins are the same as the builtin software serial, the ones marked AIN0 (TX) and AIN1(RX)). The baud rate used depends on the system clock speed as shown in the table below.  Once the bootloader is programmed, the target can be programmed over serial; the bootloader will run after reset, just like on a normal Arduino. Bootloaders are not included for the 2k-flash versions of these parts - more than a quarter of the available flash would go to the bootloader, and the Arduino abstraction is already severely constrained with just 2k of flash. For these parts, we recommend ISP programming. Clock speeds lower than 1MHz are not supported because the resulting baud rates would be so low that it some serial adapters start having problems talking to them. At 1MHz, uploads are painfully slow. We recommend using the bootloader only at 8MHz or higher. If you are trying to save power, it is usually better to run at 8MHz and put the chip to sleep for most of the time.

The ATtiny441/841, ATtiny1634, ATtiny44/84, ATtiny45/85, ATtiny461/861, ATtiny48/88 and the ATtiny x7-family do not have hardware bootloader support. To make the bootloader work, the "Virtual Boot" functionality of Optiboot is used. Because of this, another vector is used to point to point to the start of the applications - this interrupt cannot be used by the application - under the hood, the bootloader rewrites the reset and "save" interrupt vectors, pointing the save vector at the start of the program (where the reset vector would have pointed), and the reset vector to the bootloader (as there is no BOOTRST fuse). Up until version 1.2.0 of this core, the WDT vector was used for this purpose. In 1.2.0 and later, the EE_RDY vector (which is not used by anything in Arduino-land - the EEPROM library uses a busy-wait) is used instead. **If the bootloader was burned with 1.1.5 or earlier of this core, the WDT cannot be used to generate an interrupt** (WDT as reset source is fine) - re-burning bootloader with 1.2.0 or later will resolve this.

#### Big changes coming in 2.0.0
The baud rates used for uploading in 1.x were chosen poorly. 57600 baud at 8 MHz - and 115200 baud at 16 MHz - already has 2% baud rate error before accounting for any oscillator error - and it's in the direction that makes the most common oscillator error with the 841, 1634, and 828 worse rather than counteracting it. **Those baud rates were not appropriate** - they increased the chance that an internal oscillator would be too far off of the nominal frequency for serial communication, and this posed repeated headaches for users. The changes to the baud rates do mean that you need to reinstall the bootloader. These speeds were not changed for the 87/167: Those parts have a much better baud rate generator and do not suffer from baud rate accuracy problems like normal classic
AVRs.

 2.0.0 will use the following baud rates:

| System Clock | Baud (UART) | Baud (softser) | Baud (LIN - 87/167 only) |
|--------------|-------------|----------------|--------------------------|
|       20 MHz |      115200 |          57600 |                   115200 |
|     16.5 MHz |         n/a |            n/a |                      n/a |
|       16 MHz |       76800 |          57600 |                   115200 |
|     12.8 MHz |         n/a |            n/a |                      n/a |
|       12 MHz |      115200 |          57600 |                   115200 |
|        8 MHz |       76800 |          38400 |                   115200 |
|        6 MHz |       57600 |          28800 |                    76800 |
|   4 MHz xtal |       38400 |          14400 |                    56700 |
|   4 MHz int. |       9600* |          4800* |                    9600* |
|   2 MHz int. |       9600* |          4800* |                    9600* |
|        1 MHz |        9600 |           4800 |                     9600 |
|    3.686 MHz |       56700 |        14400** |                    56700 |
|    7.372 MHz |      115200 |        38400** |                   115200 |
|    9.216 MHz |      115200 |        38400** |                   115200 |
|  11.0592 MHz |      115200 |        38400** |                   115200 |
|  14.7456 MHz |      115200 |        57600** |                   115200 |
|   18.432 MHz |      115200 |        57600** |                   115200 |

`*` On these parts, the chip is started in 1 MHz mode via the CKDIV8 fuse, and that is the speed that Optiboot runs at, and only switched to the specified speed once the sketch starts running. One of the side benefits of this is that in many cases a single binary can be used for all of these speeds, as well as the 8 MHz (internal or xtal) or 4 MHz xtal cases, reducing the absurd number of bootloader binaries that we had to distribute.

`**` These are USART crystals (they divide perfectly to common baud rates), and on parts with a hardware serial port, they eliminate calculation error in the baud rates. That's why can run the bootloader on parts with hardware serial at such high baud rates vs their clock speed - all the common baud rates come out perfectly (the limit on baud rate is from the calculation accuracy, not the speed of the target chip). The cost is that all other timekeeping is worse (takes longer to return and/or is less accurate). **there is no benefit to using a USART crystal on parts without hardware serial** as the math there doesn't have the factor-of-eight reduction in precision that comes with the hardware serial port. Software serial is a real hackjob anyway, and is not recommended for any purposes.

16.5 and 12.8 MHz are not supported for Optiboot. Those speeds are achieved by tuning performed by the bootloader (if micronucleus, which of course isn't optiboot) or by the initialization code that runs before setup (but after the bootloader). Even when the target speed is 16.5 or 12.8 via tuning, Optiboot will run at 16.0 or 8.0 MHz.

#### A warning about Virtual Boot
Virtual boot relies on rewriting the vector table, such that the RESET vector points to the bootloader. This presents a potential issue: If the bootloader starts to write the first page, but then - for some reason - fails (such as a poorly timed reset right after the programming process begins), the page containing the reset vectors will be erased but not rewritten, with the result being that both the sketch and bootloader are hosed. The board will run neither the application nor bootloader until ISP programming is used to reinstall the bootloader. A solution is possible - and it is well known and tested on Micronucleus. But bringing that to optiboot is non-trivial.

**Because of this issue, Optiboot should not be used for production systems**

See the [Programming Guide](Programming.md) for more information on programming parts using Optiboot.

### Micronucleus - VUSB bootloader for 841, 167, 85, 88 and 84/84a
It's finally here! As of 1.4.0, we now offer Micronucleus (aka Digispark) support for some of the more popular boards for these bootloaders. This allows sketches to be uploaded directly via USB, which many users find highly convenient. This comes at a cost in terms of flash - they typically use around 1.5k of flash, and they sometimes have problems connecting to specific USB ports. These boards are available from various vendors; see the part-specific documentation pages for more information on the implementation used on specific parts. For more information on using Micronucleus, see the [usage documentation](avr/extras/Ref_Micronucleus.md).

### Changing the ATtiny clock speed and other settings

Changing the ATtiny clock speed, B.O.D. settings etc, is easy. When an ATTinyCore board is selected from the Tools -> Board menu, there will appear extra submenus under Tools menu where we can set several ATtiny properties:

* Tools > Chip: (Select the part being used )
* Tools > Clock:  (Select the desired clock speed)
* Tools > Save EEPROM: (Boards without bootloader only - controls whether EEPROM is erased during a chip erase cycle)
* Tools > B.O.D Level: (trigger voltage for Brown Out Detection - below this voltage, chip will be held in reset)
* Tools > B.O.D. Mode (active): (441, 841, 1634, 828 only - see B. O. D. section below)
* Tools > B.O.D. Mode (sleep): (441, 841, 1634, 828 only - see B. O. D. section below)

After changing the clock source, BOD settings, or whether to save EEPROM on chip erase), you must do "Burn Bootloader" with an ISP programmer. See [Programming Guide](Programming.md)

#### Supported clock speeds:
Supported clock speeds are shown in the menus in descending order of usefulness, ie, the popular clock speeds/sources are at the top, and the weird ones are at the bottom. See the notes for caveats specific to certain clock speeds.

Optiboot is supported only on speeds of 1 MHz or more.

Micronucleus boards have highly constrained options for clock speed, and the exact parameters depend on the part - refer to the part-specific documentation for more information.
Internal:
* 8 MHz
* 1 MHz
* 16 MHz (PLL clock, x5, x61 only)
* 16 MHz (aggressively configured 441/841 only, including for vUSB support)
* 4 MHz*** (except on x313, starts up at 1MHz and immediately switches to 4MHz before setup() is run)
* 2 MHz*** (starts up at 1MHz and switches to 4MHz before setup() is run)
* 16.5MHz † ! (PLL clock, tuned, x5, x61 only, for vUSB support)
* 12 MHz † ‡ (Internal, tuned aggressively, for vUSB support)
* 12.8MHz † ‡ (Internal, tuned aggressively, for vUSB support)
* 0.5 MHz** ‼ (x313 only)
* 512 kHz** ‼ ‡ (ULP - x41 only)
* 256 kHz** ‼ ‡ (ULP - x41 only)
* 128 kHz** ‼ ‡ (watchdog or ULP, all except 1643, 828)
* 64 kHz** ‼ ‡ (ULP - x41 only)
* 32 kHz** ‼ ‡ (ULP - 1634, 828, x41 only)

External crystal (all except 828, 43 and x8-family):
* 20 MHz !
* 18.432 MHz* !
* 16 MHz
* 14.7456 MHz* !
* 12 MHz !
* 11.0592 MHz* !
* 9.216 MHz* !
* 8 MHz
* 7.3728 MHz* !
* 6 MHz !
* 4 MHz
* 3.6864 MHz* !

External Clock:
* 20 MHz !
* 18.432 MHz* !
* 16 MHz
* 14.7456 MHz* !
* 12 MHz !
* 11.0592 MHz* !
* 9.216 MHz* !
* 8 MHz
* 7.3728 MHz* !
* 3.6864 MHz* !


All available clock options for the selected processor will be shown in the Tools -> Clock menu.

`*` These weird crystals are "UART frequencies" - these divide evenly to common baud rates, so you can get a perfect match if this is required for your application - typical UART use cases do not require running at one of these UART frequencies, the usual caveats about software serial and baud rate accuracy on classic (pre-2016) AVR designs notwithstanding. These to not hold particular benefit for either the 87/167, nor parts without hardware serial ports. They are not recommended on parts without applicable hardware serial ports, as the odd clock speed imposes additional overhead.

`**` These options are slow enough that many ISP programmers may not be able to program them. Depending on the ISP programmer (and in some cases the firmware on it), there may be a setting or jumper to slow the SCK frequency down for programming these parts, or it may automatically figure it out. The SCK frequency must be less than 1/6th of the system clock for ISP programming. Before using a such a low clock speed, consider whether you might be able to get lower power consumption by running at a higher base clock while staying in sleep most of the time - this results in fewer programming headaches, and in many (but not all) use cases results in comparable or lower power consumption.

`***` The 4MHz internal option is useful if you are running near the minimum voltage - the lowest voltage for most of these parts is 1.8v, and at that voltage, they are only rated for 4MHz maximum. This starts up at 1 MHz and then switches to 4 MHz. Hence, bootloader performance is very slow, as it doesn't do this itself. The 2 MHz option serves the same purpose for the 841/441, which are within spec down to 1.7v - as long as the clock speed is kept to 2 MHz or below

`†` These speeds support vUSB - 12 MHz and 16 MHz modes may not work reliably with aggressively tuned internal oscillators. 16.5 and 12.8 are much better - but 12.8 requires a very large amount of flash. Note that 64/12.8 is an integer, so this has inherrently better timekeeping capability than other odd speeds, though even the weird speeds do now work correctly. .

`‡` The ULP is the "Ultra Low Power" oscillator that replaced the 128 kHz WDT oscillator on the 441/841/828/1634. Like the WDT oscillator, it is only calibrated very roughly - the spec is +/- 30% (over the whole operating range - so in practice it's usually not quite that bad). On the 1634 and 828, it always runs at 32 KHz, but on the 841, it can apparently clock the system up to 16 times that rate. Though there is a tuning register, and a nice responsive looking tuning curve in the typical properties section of the datasheet, look more closely - there are only 4 points marked on the horizontal axis: Sure enough in the register, there are only 2 bits of tuning for it, and that graph just gives you a false impression otherwise.

`!` Micros takes longer to return on these clocks - the math for micros is easiest if the prescaler used by the millis timer (almost always 64, except at extremely low clock speeds) can be evenly divided by the clock speed in MHz; in that case, we can just rightshift the number of ticks. When it can't, we use bitshift/addition ersatz division, which is slower (we never use actual division - We did VERY briefly, until someone noticed just how long micros() was taking to return)

`‼` `micros()` and `delayMicroseconds()` require a system clock of 1 MHz or higher.

**Warning** Particularly in versions prior to 1.5.0, When using weird clock frequencies (those other than 16MHz, 8MHz, 4MHz, 2MHz, 1MHz, 0.5MHz), micros() is significantly slower (~ 110 clocks) (It reports the time at the point when it was called, not the end, however, and the time it gives is pretty close to reality). This combination of performance and accuracy is the result of hand tuning for these clock speeds. For other clock speeds (for example, if you add your own), it will be slower still - hundreds of clock cycles - though the numbers will be reasonably accurate, and reflect the time when it was called. millis() is not effected, only micros() and delay().

This differs from the behavior of official Arduino core - the "stock" micros() executes equally fast at all clock speeds, and instead simply returns wrong values when  (64/(clock speed in microseconds) is not an integer. 12.8 MHz is a special case and is handled exactly by ATTinyCore (though not by the official one).

Thanks to @cburstedde for his work this his work towards making this suck far less in the 1.5.0 release!

#### Using external CLOCK (instead of crystal) on other parts
All of these parts support using and external clock as clock source. It is the most basic of clock sources - whereas a crystal requires an internal inverting amplifier, typically one of the more demanding parts of the microcontroller, the external clock requires almost nothhing from the chip being clocked that way. What does this mean? On the few parts that do not support a crystal, it means you can get accurate timing. On everything else, it lets you get an extra pin as well. This comes at the cost of needing an additional part, external oscillator. These are available, but even the cheapest AliExpress sources I could find wantesd 40-80 cents each for them, and from reputable supply houses, they startt at around $1.40, likely making them the most expensive part on the board (or #2 if you cheap out and get random ones from china.). Many of them are in a fairly large 7050 (7mm x 5mm) pacakage, though they are available in much smaller ones. They usually need their own decoupling capacitor (0.01uF is the norm) and like any high frequency trace, the path to the clock in pin must be short.
We now offer the option to use an external clock on all parts, at the same frequencies as a crystal is, except that especially slow speeds are dropped. External oscillators are power hogs! Don't use them if you care about power consumption!

##### *oops! I thought my crystal was an external clock and now I can't program my chip!*
Never fear. [Unbricking classic AVR parts bad clock setting](https://github.com/SpenceKonde/AVR-Guidance/tree/master/Troubleshooting/Unbricking)

#### Determining clock speed and source from within the sketch
The clock speed is made available via the F_CPU #define - you can test this using #if macro. Note however that this tells you what speed it was compiled for. If the chip is in fact not running at that speed (because a different speed was selected when tyou last "burned bootloader", it has no way to know shhort of comparing to some other oscillator, (which sure, you could do with the RTC or something).

In version 1.3.3 and later, the clock source is also made available via the CLOCK_SOURCE #define. CLOCK_SOURCE can take one of the following values (as of 1.4.0, it is expanded to cover a few weird clocking situations: the low 4 bits identify the source, and high 4 bits identify special things regarding it:

```text
0 - Internal 8MHz oscillator, not prescaled, or prescaled to 1 MHz (ie, fully set by fuses)
1 - External Crystal
2 - External Clock
3 - Internal WDT oscillator  (not available on the x41, 1634, and 828)
4 - Internal ULP oscillator (available only on the x41, 1634, and 828)
5 - Internal 4MHz oscillator (present only on the x313 parts - if the 8MHz internal oscillator is prescaled to 4MHz, CLOCK_SOURCE will be 0x10, not 5.)
6 - Internal PLL (x5 and x61 only)
16 or 0x10 (ie, 0x10 | 0) - Internal oscillator with prescaling not set by fuses (ie, not 1 MHz or 8 MHz - ie, 2 or 4 MHz)
17 or 0x11 (ie, 0x10 | 1) - External crystal at 16MHz, which may be prescaled to get lower frequencies (for Digispark Pro ATtiny167)
18 or 0x12 (ie, 0x10 | 2) - External clock  at 16MHz, which may be prescaled to get lower frequencies (for MH Tiny ATtiny88)
22 or 0x16 (ie, 0x10 | 6) - Internal PLL prescaled to get a lower frequency (for Digispark et. al.)
```
### Assembler Listing generation

In version 1.2.2 and later, Sketch -> Export compiled binary will generate an assembly listing in the sketch folder; this is particularly useful when attempting to reduce flash usage, as you can see how much flash is used by different functions. In 2.0.0 and later it gemnerates a memory map. Tools submenu options which impact the compiled binary will be encoded in the name that the files are given.

### Link-time Optimization (LTO) support

Link time optimization is enabled by default. If compiling with very old versions of the IDE, this must be disabled. Cases do exist where this setting can change behavior of a sketch; in all cases where this has been observed, a bug in the code was hidden by one setting or the other, and/or undefined behavior was invoked. This is a problem with your code, not the core.

### Makefile Support

For those who prefer to compile with a makefile instead of the IDE, sketches can be compiled with https://github.com/sudar/Arduino-Makefile - See the [makefile documentation](makefile.md) for more information on specific steps needed for this process.

### I2C, SPI and Serial
Most of these parts do not have hardware support for I2C, SPI, and/or UART (Serial) like an ATmega device would. **As much as possible we try to paper over the differences - you can include Wire.h or SPI.h and expect things to just work except as noted below** - this is achieved by a special version of Wire.h and SPI.h which presents the same API, but implements it very differebtly depending on the underlying hardeware. Hence **the use of libraries like USIWire, tinyWire, WireS, and so on is unnecessary** These libraries are also considered unsupported, as they should never be necessary. In the case of Serial/UART, where there is no hardware serial, the SoftwareSerial library can be used, but it is often undesirable because of how it takes over all the PCINTs. To address this, we provide a different software serial implementation which uses the analog comparator interrupt instead of a PCINT, allowing the PCINTs to be used freely. The RX pin is fixed, but the TX pin can be moved around to a limited subset of pins. See the serial section below for more information. The following table shows what hardware interface is available on each of these part.


| Part(s)               | SPI           | I2C Master  | I2C Slave | Serial (TX* , RX) |
|-----------------------|---------------|-------------|-----------|-------------------|
| ATtiny x313           | USI           | USI         | USI       | 1x Hardware       |
| ATtiny 43             | USI           | USI         | USI       | Software PA4, PA5 |
| ATtiny x4             | USI           | USI         | USI       | Software PA1, PA2 |
| ATtiny x5             | USI           | USI         | USI       | Software PB0, PA1 |
| ATtiny 26             | USI           | USI         | USI       | Software PA6, PA7 |
| ATtiny x61            | USI           | USI         | USI       | Software PA6, **  |
| ATtiny x7             | Real SPI      | USI         | USI       | 1x Hardware (LIN) |
| ATtiny x8             | Real SPI      | Real TWI    | Real TWI  | Software PD6, PD7 |
| ATtiny x41            | Real SPI      | Software    | Slave TWI | 2x Hardware       |
| ATtiny1634            | USI           | USI         | Slave TWI | 2x Hardware ***   |
| ATtiny828             | Real SPI      | Software    | Slave TWI | 1x Hardware       |

`*` - TX pin can be moved to any other pin on that port with Serial.setTxBit().
`**` - RX can be on PA5, PA6, or PA7 (default), controlled by the tools submenu.
`***` - Serial1 shares pins with the USI and slave TWI interface, which basically means you have to choose between USI (SPI or I2C master) or I2C slave, or a second serial port.

There are some specific considerations relevant to each of these interfaces, detailed below.

#### SPI
Where real hardware SPI is available, SPI.h will behave identically to that on any classic AVR.

On USI parts, there are a few minor concerns, though most things will work without issue, and it should all be handled transparently via the SPI library.
* **USI does not have MISO/MOSI, it has DI/DO**
  * when operating in master mode, **DI is MISO, and DO is MOSI**.
  * When operating in slave mode, **DI is MOSI and DO is MISO**. Note that like all other versions of SPI.h, slave mode is not supported. You must use a different library for that.
  * The #defines for MISO and MOSI assume master mode, slave mode being unsupported). PIN_USI_DI, PIN_USI_DO, and PIN_USI_SCK are defined and can be used for operation in slave mode with some other library. Be careful to distinguish the MISO/MOSI/SCK pins marked as "Programming Pins" from the pins used for SPI from within the sketch - when programming a part via ISP, it is, after all, acting as a slave.
* The clock dividers are implemented in software (a clock generator is one of the many things that USI lacks). Clock dividers of 2, 4, 8 and >=14 are implemented as separate routines; **call `SPISettings` or `setClockDivider` with a constant value to use less program space**, otherwise, all routines will be included along with 32-bit math. Clock dividers larger than 14 are only approximate because the routine is optimized for size, not exactness.
* Interrupts are not disabled during data transfer. That means that an interrupt could fire in the middle of a byte, and one of the bits in that byte would be very long. This is usually fine. If it isn't, because you're working with devices that require consistent clocking, wrap calls to `transfer` in `ATOMIC_BLOCK` or disable interrupts in the normal ways.
* Be aware that USI-based I2C is not available when USI-based SPI is in use (this should be obvious, as they used the same pins).


#### I2C
The situation regarding I2C is more complicated; The ATtiny48 and ATtiny88 have real hardware I2C, which works like it does on ATmega devices. Like SPI.h, the Wire.h library will handle most of these differences, and most things will work the same way.

Most other devices must use the USI for I2C. In these cases:
* **You must have external pullup resistors installed** - unlike devices with a real hardware TWI port, the internal pullups cannot be used with USI-based I2C to make simple cases (short wires, small number of tolerant slave devices) work.
* The option to set the clock as I2C master does not work. The SCL clock speed is fixed.

A small number of devices have support for hardware slave I2C **but neither a USI nor hardware TWI for master operation**. On THOSE parts, some additional considerations apply:
* I2C slave works great through the included Wire.h library.
  * You can even do an alternate address or masked address (where several of the bits of the incoming address are ignored) by setting the `TWSAM` register. That register works the same way as it does on modern (post-2016) AVRs - however, no wrapper is provided around setting it, unlike the modern AVRs (ex, megaTinyCore and DxCore)
  * On the ATtiny828, you must have the watchdog timer enabled in order for the USI to work in I2C mode; one of the pins in afflicted with one of the nastiest silicon bugs in a pre-2016 tinyAVR, and when the WDT is not enabled (interrupt mode with an empty interrupt is fine),
* Software I2C Master, on the other hand is... a little flaky on these parts, most notably, it's not possible to tell whether a transaction timed out, or if the slave just responded with a bunch of 0's. There is no clock configuration functionality here either. If planning a new project, consider using a different device (might I recommend a [modern tinyAVR](https://github.com/SpenceKonde/megaTinyCore/)?) if I2C master mode is a big part of your application.

Regardless of the implementation, simultaneously acting as both a master and a slave is never supported here. The hardware doesn't support it like it does on modern AVRs.

##### Buffer size
On all parts with more than 128b of SRAM, the buffer size in 32 bytes. On smaller parts, it is 16 bytes, but I'm not sure you could make those work with the Wire library anyway due to flash size constraints so this may not be relevant. All official cores use 32b buffers, and it is for this reason that a 32b buffer is used even on parts where the pair of buffers leads to using a painfully large fraction of the RAM  - libraries implicitly depend on the buffer being at least 32b, often without the author even being aware of that fact.

#### Serial Support
To most of us, the Serial interface is the most important of the big three serial protocols. All parts, whether or not they have hardware serial, will have an object named `Serial` that provides serial interface functionality. Where there is hardware serial, the Serial object is a normal fully featured, full duplex serial port that works just like any other AVR. The lucky chips that have two serial ports will also have Serial1 defined.

Unfortunately, that's only 8 of the 21 parts supported by this core. The rest must use some form of software serial. While this core is fully compatible with the usual SoftwareSerial library, it comes with the usual disadvantages, most notably the fact that it grabs all the PCINT vectors for itself. To address that on the parts not blessed with hardware serial, we include a software serial implementation with a fixed (on the x61, there are three options, selected from a tools menu.) RX pin, and a TX pin with a limited number of options - but which leaves the PCINT vectors available. It uises the analog comparator interrupt, and requires that the RX pin be the AIN1 pin. TX defaults to the AIN0 pin.

Regardless of how you achieve the software serial, however, you can still only transmit or receive on a single software serial instance at a time (SoftwareSerial or the builtin tinySoftSerial). Transmit is always blocking - a call that writes via software serial will not return until the data is sent unlike hardware serial which puts it into a buffer to send in the background.

```c
// In other words
Serial.println("Hello World");
// on a part without hardware serial, which is hence using our builtin software implementation, is equivalent to:
Serial.println("Hello World");
Serial.flush();
```

##### Moving builtin soft-serial TX pin
The builtin software serial port ("Serial" on parts without hardware serial) can be moved to any pin *as long as it is on the same port*. Use the `Serial.setTxBit(bit)` method. The 'bit' argument passed must be a number between 0 and 7, corresponding to the number within the port of the desired TX pin; **this should be called before** `Serial.begin()`. This option is only available on parts with the software serial.

##### TX-only soft serial
Many users have asked for a way to disable the receiving functionality of the builtin soft-serial entirely. New in 2.0.0, you can choose TX only the tools -> Software Serial menu. This will exclude everything except the transmit functionality. read() and peek() will always return -1, and available() will always return 0.

##### Warning: Internal oscillator and Serial
Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. However, in practice, a larger portion of parts work without tuning than would be expected from the spec. That said, for the ATtiny x4, x5, x8, and x61-family I have yet to encounter a chip that was not close enough for serial using the internal oscillator at 3.3-5v at room temperature - This is consistent with the Typical Characteristics section of the datasheet, which indicates that the oscillator is fairly stable w/respect to voltage, but highly dependent on temperature.

The ATtiny x41-family, 1634R, and 828R have an internal oscillator factory calibrated to +/- 2% - but only at operating voltage below 4v. Above 4v, the oscillator gets significantly faster, and is no longer good enough for UART communication if the UART baud rate calculation error is in the same direction - See [the AVR baud rate chart](https://docs.google.com/spreadsheets/d/1uzU_HqWEpK-wQUo4Q7FBZGtHOo_eY4P_BAzmVbKCj6Y/edit?usp=sharing). These parts have a clock speed menu option for the internal oscillator when Vcc > 4v and when it is <= 4V; when the >4V option is selected, we'll takea guess at what the calibration should be lowered to, which should be enough to get serial working.. The 1634 and 828 (non-R) are not as tightly calibrated (so they may need tuning even at 3.3v) and are a few cents less expensive, but suffer from the same problem at higher voltages. Due to these complexities, **it is recommended that those planning to use serial (except on a x41, 1634R or 828R at 2.5~3.3v, or with a Micronucleus bootloader) use an external crystal** until a tuning solution is available.

### ADC Support
ATTinyCore 2.0.0 introduces a major enhancement to the handling of analog and digital pin numbers: Now, in all the #defined constants that refer to an analog channel, the high bit is set. (ie, ADC channel 4, A4, is defined by a line `#define A4 (0x80 | 4));` (actually, we also define ADC_CH() macro as shorthand for the bitwise or with 0x80. This advantage of this that it makes it more obvious why we're doing this to the number; if you see (0x80 | 4) you'd be like "wtf is this for? what does 0x80 have to do with anything?", whereas if you hadn't read this, and you saw ADC_CH(4) - you might not know exactly what's going on, but just from the name you'd know it was something to do with an analog reading, maybe of channel 4). Because all the analog channel number defines are all distinct from things that aren't analog channel numbers, the core's analogRead and digitalRead functions can tell the two apart; digitalRead(A3) will now look up what digital pin analog channel 3 is on, and use digitalRead on that, while analogRead(7) will now go look up what analog channel is on digital pin 7, and use analogRead on that.

#### Digispark Pro warning
On the ATtiny167 Digispark Pro pin mapping, "An" does not mean "Analog Channel N", it means "The analog channel on the pin marked An on the Digispark Pro pinout chart" - which is the same as digital pin n. Use ADC_CH(n) to choose analog inputs by channel number. This inconsistency sucks - but analogRead(A3) reading the pin marked A9 on the pinout chart that everyone uses for the Digispark Pro isn't great behavior either...

#### Differential ADC support
ATTinyCore 2.0.0 includes proper support for using the differential ADC - which on many parts is as good as or better than the differential ADC found on ATmega parts of the same era (and in fact, in some use cases, is better than the one on the new AVR Dx-series parts!). Differential ADCs of varying sophistication are available on the following families of parts: x5, x7, x4, x61, x41. Refer to the part specific documentation for the details on how to use it - generally there will be constants you pass in place of the pin number (listed in a table on the part specific documentation page).

Some parts will additionally have one or two additional configuration functions related to the differential mode:
##### analogGain()
analogGain() is unique to the tiny841/441, which has too many differential channels and gain setting combinations to fit into a single byte. It is documented in the [ATtiny841/441 page](./avr/extras/ATtiny_x41.md). as it only applies to those devices. All others with programmable gain pass the gain setting as part of the constant.

##### setADCBipolarMode(bool bipolar)
This function is available on four families of parts including a total of 11 devices. These parts included an option to configure the differential ADC in either Unipolar mode or Bipolar mode. In bipolar mode, the value returned is signed. If reference was 1.0 V and you were looking at a pair of pins with voltages of 2.75 V and 2.5 V on them, your positive pin is 0.25 times the reference voltage above the negative pin and hence, the value returned would be 128. If the pins were reversed, it would be -128. In Unipolar mode, you gain an additional bit of precision.... if the positive pin is indeed larger. If it's not, you'll just get 0's back This is why some of the part have two options for a pair of pins, differing only in which one is positive vs negative - it's so you can swap the pins and use unipolar mode for higher accuracy. The argument is a boolean value indicating whether bipolar mode should be enabled (true = bipolar mode, false = unipolar mode. )

Not all parts support the switch between unipolar and bipolar mode. The x4, x5, x61. and x7 do. The x41 supports only bipolar mode, while the 26 supports only unipolar mode. And obviously, devices without any differential ADC don't support this function either. Calling it for a part that doesn't support it will generate a compile error saying as much.

##### ADC noise reduction mode
Now that we've talked about ADC gain, which can be up to 100x on the 841 or 32x on the 861, we need to recognize that if you want to be measuring signal, rather than noise, you will need to take more care than usual. Some of this is in hardware - the AVCC pin (if present) should be connected with the inductor like the datasheet recommends rather than just being tied to Vcc like everyone in Arduinoland does normally. You should also probably be using the ADC noise reduction mode. This puts the processor to sleep during the conversion to reduce noise generated internally during that sensitive time.

To use ADC noise reduction mode, simply use `analogRead_NR()` instead of `analogRead()`. There are really only a few caveats here:
* The I/O clock will be stopped. This means timers will lose time (13 ADC clocks per reading; the ADC clock is 100-200 kHz on these so that means between 65 and 130 us of time is lost by millis/micros per ADC reading) and PWM will stop.
* If interrupts are enabled, pin change interrupts and the WDT interrupt will wake the part prematurely. This may impact the quality of the readings. You may wish to disable these interrupts if they are in use.
* Do not disable interrupts globally though, because then the ADC interrupt won't be able to wake the part.
* This uses the ADC interrupt vector. Hence if at other points in time, you wanted to, say, use the ADC in free running mode, and run your own interrupt when a new result is ready, this is not compatible with `analogRead_NR()`. However analogRead_NR is isolated in it's own file, along with the empty ISR, so unless you're using `analogRead_NR()`, you can freely define that vector yourself.

### Timers and PWM
All of the supported parts have hardware PWM (timer with output compare functionality) on at least one pin. See the part-specific documentation pages for a chart showing which pins have PWM. In addition to PWM, the on-chip timers are also used for millis() (and other timekeeping functions) and tone() - as well as by many libraries to achieve other functionality. Typically, a timer can only be used for one purpose at a time.

On all supported parts, timekeeping functions are on timer0. This means that reconfiguring timer0 by manipulating it's registers will break `millis()` and `delay()`; this is not recommended unless millis is disabled entirely.

On all parts except the tiny841/441 `tone()` is on timer1; on 841/441. tone() is on Timer2 to improve compatibility; with Tone moved onto timer2, the many libraries that use timer1 (Servo, TimerOne, and many others) can be used alongside `tone()` on the 841/441. Using `tone()` will prevent PWM from working on PWM pins controlled by Timer1 (Timer2 for 841/441), and manipulating it's registers will break `tone()`.

Most of the ATtiny parts only have two timers. The attiny841 has a third timer, timer2, which is an exact copy of the lovely 16-bit timer1, and completely different from the timer2 that most atmega devices have. Libraries designed to work with "Timer2" will not work on any of these parts, even the 841/441.

### Built-in tinyNeoPixel library

The standard NeoPixel (WS2812/etc) libraries do not support all the clock speeds that this core supports, and some of them only support certain ports. This core includes two libraries for this, both of which are tightly based on the Adafruit_NeoPixel library, tinyNeoPixel and tinyNeoPixel_Static - the latter has a few differences from the standard library (beyond supporting more clocks speeds and ports), in order to save flash. Prior to 2.0.0, a tools submemu was needed to select the port. This is no longer required (the adafruit code was written with zero tolerance for any divergances from ideal timing; allowing for tiny divergences at points where it doesn't matter was all it took to reimplememt this without the need for that submenu. This code is not fully tested at "odd" clock speeds, but definitely works at 8/10/12/16/20 MHz, and will probably work at other speeds, as long as they are 7.3728 MHz or higher. See the [tinyNeoPixel documentation](avr/extras/tinyNeoPixel.md) and included examples for more information.

### Additional configuration options
These are available from tools submenus

#### Pin Remapping (x61, 441, 841 only, new in 2.0.0)
The x61-series can use either PORTA or PORTB pins for the USI. This must be chosen at compiletime - implementing swap() like the megaAVR parts have would impose excess overhead.

The x41-series has two options for USART0 and two options for SPI. They are chosen independently, and the tools menu hence contains four options. (it's better than two menus, right?)

#### Retain EEPROM
All non-bootloader board definitions have a menu option to control whether the contents of the EEPROM are erased when programming. This only applies to ISP programming, and you must "burn bootloader" to set the fuses to apply this. Because it only applies to ISP programming, it is not available for Bootloader board definitions. on Optiboot definitions, EESAVE is never enabled, but since only ISP programming will normally erase the EEPROM, the only time you'd be running into this is if you were "rebootloading" it - which should always return it to a known state - or were writing over the bootloader with a sketch (which you probably shouldn't be doing anyway). Meanwhile if that option was present, it would cause a great deal of confusion since it would apply only to an uploading use case that you shouldn't be doing, and to rebootloading, not to normal uploads.

#### B. O. D. (brown out detect)
Brown-out detection continuously monitors Vcc, and holds the chip in reset state (BOR) if the applied voltage is below a certain threshold. This is a good idea with slow-rising power supplies or where it is expected that the supply voltage could droop below the required operating voltage for the frequency it is running at (see the speed grade specification for the part you're using) - without BOD enabled, this can put the chip into a hung state until manually reset. However, BOD increases power consumption slightly, and hence may be inappropriate in low power applications.

The BOD voltage trigger level can be chosen from the tools -> BOD menu. The ATtiny441, 841, 828, and 1634 support independently configuring the BOD mode (active, sampled, disabled) for active and sleep modes (see the applicable datasheet for details). These are configured via the Tools -> BOD Mode (sleep) and Tools -> BOD Mode (active) menus.

Unless the power consumption of the BOD is a show-stopper, **it is strongly recommended that BOD be enabled for any production system**. The failure mode from insufficient voltage is generally an ungraceful hang.

In all cases, the selected BOD option(s) is/are configured by the fuses, so after changing these, you must "burn bootloader" to set the fuses.

Be aware that we do not check whether the clock speed and BOD threshold you selected make sense together. If you want to run at 16 MHz with a 1.8v BOD (which won't do a damned bit of good), we won't stop you.

#### Option to disable millis()/micros()

The Tools -> millis()/micros() allows you to enable or disable the millis() and micros() timers. If set to enable (the default), millis(), micros() will be available. If set to disable, these will not be available, Serial methods which take a timeout as an argument will not have an accurate timeout (though the actual time will be proportional to the timeout supplied); delay will still work. Disabling millis() and micros() saves flash, and eliminates the millis interrupt every 1-2ms; this is especially useful on parts with very limited flash, as it saves a few hundred bytes. We do not support using alternate timers for millis like megaTinyCore and DxCore do - there, the timers are consistent - The same code on DxCore and megaTiny Core handles both the type A and Type B timers om those parts. Over here, except for the ubiquitous timer 0, there are almost as many versions of timer1 as there are parts.

## Memory Lock Bits, disabling Reset
ATTinyCore will never set lock bits, nor will it set fuses to disable ISP programming (it is intentionally not made available as an option, since after doing that an HV programmer is needed to further reprogram the chip, and inexperienced users would be at risk of bricking their chips this way). The usual workflow when these bits are in use is Set other fuses -> Upload -> Test -> manually set the lockbits and/or fuses. This can be done from the command line using AVRdude. To expedite the process, you can enable "Verbose Upload" in preferences, do "burn bootloader" (the board and/or programmer does not need to be present), scroll to the top of the output window - the first line is the avrdude command used to burn the bootloader, including the paths to all the relevant files. It can be used as a template for the command you execute to set the fuse/lock bits.

Disabling of reset is only an option for boards definitions with a bootloader which uses a sound flash-erase implementation (Optiboot presently does not, while the VUSB bootloaders which disable reset are in widespread use, seemingly without issue). We recommend against it in all cases. The 8 and 14 pin parts can be unbricked with a comparatively simple HVSP programmer (only 4-7 pins - 4 pins + reset for 8-pin, plus 3 more tied low on 14-pin). Everything with more pins needs an HVPP programmer, involving a wire connected to every pin or almost every pin on the chip. The sheer number of connections makes it unlikely that it could ever be unbricked in-system if the "system" is much more than a breakout board. HVPP is extremely exotic within the hobby community, such that I've never heard anyone talk about unbricking with HVPP.

**USE EXTREME CAUTION WHEN USING THE USB UPDATE FOR MICRONUCLEUS** as you can update to a version of the bootloader that will not support your board.


## Pin Mappings


### ATtiny441/841
![x41 pin mapping](avr/extras/Pinout_x41.jpg "Arduino Pin Mapping for ATtiny841/441")

### ATtiny1634
![1634 pin mapping](avr/extras/Pinout_1634.jpg "Arduino Pin Mapping for ATtiny1634")

### ATtiny828
![828 Pin Mapping](avr/extras/Pinout_828.jpg "Arduino Pin Mapping for ATtiny828")

### ATtiny25/45/85
![x5 pin mapping](avr/extras/Pinout_x5.jpg "Arduino Pin Mapping for ATtiny85/45/25")

### ATtiny24/44/84
![x4 Pin Mapping](avr/extras/Pinout_x4.jpg "Arduino Pin Mapping for ATtiny84/44/24")

### ATtiny261/461/861
![x61 Pin Mapping](avr/extras/Pinout_x61.jpg "Arduino Pin Mapping for ATtiny861/461/261")

### ATtiny87/167
![x7 Pin Mapping](avr/extras/Pinout_x7.jpg "Arduino Pin Mapping for ATtiny167/87")

### ATtiny48/88
![x8 SMD Pin Mapping](avr/extras/Pinout_x8.jpg "Arduino Pin Mapping for ATtiny88/48 in TQFP")
![x8 DIP Pin Mapping](avr/extras/Pinout_x8-PU.jpg "Arduino Pin Mapping for ATtiny88/48 in DIP")

### ATtiny2313/4313
![x313 Pin Mapping](avr/extras/Pinout_x313.jpg "Arduino Pin Mapping for ATtiny4313/2313")

### ATtiny43U
![ATtiny43U pin mapping](avr/extras/Pinout_43.jpg "Arduino Pin Mapping for ATtiny43")


Note that two pin mappings are supported for some devices to retain backwards compatibility with other cores - the pin mapping may be chosen from a menu.

Note that analog pin numbers (ex A0 ) cannot be used with digitalWrite()/digitalRead()/analogWrite() - all pins have a digital pin number. Analog pin number should only be used for analogRead() - this represents a departure from the behavior used in the official AVR boards. This enables us to expose the advanced ADC functionality available on some of the ATtiny parts with minimal impact, as clearly written code is unlikely to fall afoul of this anyway.

## Buy Breakout boards
Except for the x5, x4, x61, and x313-family, these are only available in surface mount packages. Breakout boards are available from my Tindie store (these are the breakout boards used for testing this core), which have the pins numbered to correspond with the pin numbers used in this core. Where applicable, all of these assembled boards have the bootloader installed, and all are set to run at the advertised speed (most are available with several speed/voltage combinations).
* 841: [Bare boards](https://www.tindie.com/products/DrAzzy/attiny84184-breakout/) - [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny841-dev-board-woptiboot/ )
* 1634: [Bare boards](https://www.tindie.com/products/DrAzzy/attiny1634-breakout-wserial-header-bare-board/) - [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny1634-dev-board-woptiboot-assembled/ )
* 828: [Bare boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/) [Assembled Boards]( https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/ )
* x61-family (861/461/261): [Bare boards](https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/) [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
* x7-family (167/87): [Bare boards](https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/) [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
* x8-family (48/88): [Bare boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/) [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
* ATtiny43 (including boost converter) [Assembled Boards](https://www.tindie.com/products/16617/)
* SMD/DIP or DIP [ATtiny85 prototyping board](https://www.tindie.com/products/DrAzzy/attiny85-project-board/)
* SMD or DIP [ATtiny84 prototyping board](https://www.tindie.com/products/DrAzzy/attiny84-project-board/)


## Caveats
* Some people have problems programming the 841 and 1634 with USBAsp and TinyISP - but this is not readily reproducible. ArduinoAsISP works reliably. In some cases, it has been found that connecting reset to ground while using the ISP programmer fixes things (particularly when using the USBAsp with eXtremeBurner AVR) - if doing this, you must release reset (at least momentarily) after each programming operation. This may be due to bugs in USBAsp firmware - See this thread on the Arduino forums for information on updated USBAsp firmware: http://forum.arduino.cc/index.php?topic=363772 (Links to the new firmware are on pages 5-6 of that thread - the beginning is largely a discussion of the inadequacies of the existing firmware)
* At >4v, the speed of the internal oscillator on 828, 1634 and 841 parts increases significantly - enough that serial (and hence the bootloader) does not work. Significant enhancements have been made on this front in 1.4.0; reburning bootloader should sort it out. These are further improved in 2.0.0. Avoid using 115200 baud and 57600 baud if using the internal oscillator and running an 828, 1634, or x41 at 4V or higher - those speeds are over 2% off due to baud calculation error in the same direction that the clock speed is off.
* For that matter, don't use 115200 baud or 57600 baud on any classic AVR with a hardware serial port at 8/16 MHz, especially if they or the thing they are communicating with is using an internal oscillator. Don't use 115200 without a hardware serial port at all. and either a "USART crystal" as clock source, or a 12 MHz or 20 MHz clock source. For 8 and 16 MHz with hardware serial, 38400, 76800, and (at 16 MHz) 153600 get much better baud rate accuracy.
* There is a right and a wrong way to perform a software reset.
  * Unless you are using Optiboot and wish to reset *and* have the bootloader run, do not reset from software via `__asm__ __volatile__ (jmp 0)` - that performs a "dirty reset". After a dirty reset, it is expected that all core initialization functions may incorrectly initialize any aspect(s) of core functionality, resulting in failure modes ranging from subtle and confusing to hangs or bootloops. Instead, enable the watchdog timer, set to reset the device on timeout, and then enter an infinite loop and wait for the reset 16ms later.
  * Do not attempt to generate a software reset by connecting an I/O pin to reset and driving it low; this is specifically warned about in the datasheet.
  * If using the WDT reset on an Optiboot board, no additional actions are necessary; The bootloader will see that the reset cause was the WDT, assume that it was the thing that generated the reset, turn off the WDT and start the application.
  * If using the WDT reset on a non-optiboot board definition, you must turn it off at the very start of setup() - the chip will reset with the watchdog still running at the minimum timeout.
  * The only time that a dirty reset is acceptable (it is never recommended) is when Optiboot is in use and you need to make the bootloader run from within the app.



## License
ATTinyCore itself is released under the [LGPL 2.1](LICENSE.md). It may be used, modified, and distributed, and it may be used as part of an application which, itself, is not open source (though any modifications to these libraries must be released under the LGPL as well). Unlike LGPLv3, if this is used in a commercial product, you are not required to provide means for user to update it. A historical investigation has determined that versions of this core have been released under the LGPLv2.1 in the past - it was an oversight on our part that the license file was omitted from this core previously.

The ATTinyCore hardware package (and by extension this repository) contains ATTinyCore as well as libraries and bootloaders. These are released under the same license, *unless specified otherwise*. For example, tinyNeoPixel and tinyNeoPixel_Static, being based on Adafruit's library, is released under GPLv3, as described in the LICENSE.md in those subfolders and within the body of the library files themselves.

Tools and third party programs used by ATTinyCore are governed by their own licenses; this includes tools installed by board manager, such as AVRdude and avr-gcc.

## Acknowledgements

This core was originally based on TCWorld's ATTinyCore, which is in turn based on the arduino-tiny core here: http://code.google.com/p/arduino-tiny/
The ATtiny841 support is based on shimniok's ATtiny x41 core, and the 1634 support on Rambo's ATtiny1634 core.

@per1234, who has been an invaluable resource for myself and others within the Arduino community, specifically having set up the Travis-based CI testing system (which will cease to be used once the other foot drops wrt. Travis terms) and his years of assistance with board manager releases and more.

The pinout diagrams are created by @MCUdude, who maintains a number of excellent cores for virtually every Microcontroller you might want to use with Arduino as as it has "ATmega" in the name - http://github.com/MCUdude

Many individuals have contributed various fixes and improvements via GitHub - see the [contributor list](https://github.com/SpenceKonde/ATTinyCore/graphs/contributors) for the full list.
