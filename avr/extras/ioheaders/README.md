# I/O header files
These are the I/O header files from the latest ATpack available at the time of writing, the Atmel ATtiny Series Device Support v1.10.348 package, corresponding to all classic ATtiny parts supported by ATTinyCore. These are where you look to when you need to know how some register name is spelled, because the way it was presented it the datasheet is undefined, or when you forget what the name of the constants specifying particulars of flash, RAM, or other hardware features are called. They're also straight text... so you can search them for a register name when you see some block of code like the one shown below - you can see that they do one set of things on parts with a "TCCR1E" register, and a different set of things presumabkly to achieve a compatible endstate on parts with a "TCCR1A".... you want to understand what it's doing better than you do, but you don't even know what parts those two blocks of code are meant for!

```c
#if defined(TCCR1E)
  //(half-dozen or so lines of direct writes of uncommented hexadecimal values to peripheral registers)
#elif defined(TCCR1A)
 // (more lines of inscrutable register writes)
#else
  #error "No usable timer found"
#endif
```
Searching the IO headers would tell you, in this case, that TCCR1E is present only the x61 series, while TCCR1A is present on just about everything (but not the 85). A similar application is when you use some obscure sub-feature on a peripheral, and want to make it work on a different chip (at the time of writing, in early 2022, most microcontrollers are on 6-12 month backorder, and everyone is trying to "make it work" on whatever devices they can lay their hands on, so this is more common than usual).

I am redistributing these solely as a convenience to users; particularly for less experienced users in the Arduino world, it turns out that being told to look at the header file is a bit daunting ("I found a bunch of iosomething.h files, but not the one for the chip I'm using" "In the Arduino folder? Oh no that's the version that the IDE came with, the core installed a new one..." "well where is it installed to?" "Okay so its in c:/users/(yourusername)/AppData/Arduino15/packages (so on and so forth), but AppData is hidden..." "Uhhhh.... okay can't I just go download it" "Oh of course, you can download it from Microchip - download the file from this page. Then change the file extension to .zip - the download has the .atpack extension, even though it's just a zip file, and then it's in avr/avr/include/avr... Hmm? Where'd he go?")

The ATpack files are all available from [http://packs.download.atmel.com/](http://packs.download.atmel.com/) They are licensed by Microchip under the Apache license.

The files in this location are NOT used by the core in any way, nor are they included in the board manager releases (the copy of the headers that are used are in `(toolchain root)/avr/include/avr`, where the root of the toolchain installation depends on your OS, how Arduino was installed, whether your toolchain has been updated by any board manager releases). The extras folder is not included in board manager releases at all. Nobody wants to have to dig up files installed via board manager, whether it's to find a header used by the compiler, a pinout chart, or the core documentation (the .md files are best read on github, anyway)

## Header file license
**These files are not part of the core and I had no hand in their development**

They are distributed in their original, unmodified form, and are not subject to spelling or code style checking like the rest of the core.

This is the license notice displayed when downloading these files. It (and only this licence, subject to all terms described in the files themselves) apply to all files in this directory with the exception of this file (README.md) itself.

```text
Copyright (c) 2020 Microchip Technology Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the Licence at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```


## Headers, by usefulness
Popular parts, largest flash size only, in descending order of general desirability.

* [ATtiny841    -  iotn841.h](iotn841.h)
* [ATtiny1634   - iotn1634.h](iotn1634.h)
* [ATtiny85     -   iotn85.h](iotn85.h)
* [ATtiny84a    -  iotn84a.h](iotn84a.h)
* [ATtiny167    -  iotn167.h](iotn167.h)
* [ATtiny861a   -  iotn861a.h](iotn861a.h)
* [ATtiny88     -    iotn88.h](iotn88.h)
* [ATtiny4313   -  iotn4313.h](iotn4313.h)

## Headers, by product series
Each series is in order of ascending flash size, series are organized in order of the first digit after the flash size (2313 counts as 3, 841 comes under 4, and so on except that series' of parts that are the older versions of a more recent series and which have been almost entirely displaced by their more recent versions and are largely absent from the market - those get pushed to the bottom.

* [ATtiny2313a  - iotn2313a.h](iotn2313a.h)
* [ATtiny4313   -  iotn4313.h](iotn4313.h)
* [ATtiny1634   -  iotn1634.h](iotn1634.h)
* [ATtiny43u    -   iotn43u.h](iotn43u.h)
* [ATtiny24a    -   iotn24a.h](iotn24a.h)
* [ATtiny44a    -   iotn44a.h](iotn44a.h)
* [ATtiny84a    -   iotn84a.h](iotn84a.h)
* [ATtiny441    -   iotn441.h](iotn441.h)
* [ATtiny841    -   iotn841.h](iotn841.h)
* [ATtiny25     -    iotn45.h](iotn25.h)
* [ATtiny45     -    iotn45.h](iotn45.h)
* [ATtiny85     -    iotn85.h](iotn85.h)
* [ATtiny261a   -  iotn261a.h](iotn261a.h)
* [ATtiny461a   -  iotn461a.h](iotn461a.h)
* [ATtiny861a   -  iotn861a.h](iotn861a.h)
* [ATtiny87     -    iotn87.h](iotn87.h)
* [ATtiny167    -   iotn167.h](iotn167.h)
* [ATtiny48     -   niotn48.h](iotn48.h)
* [ATtiny88     -   niotn88.h](iotn88.h)
* [ATtiny828    -   iotn828.h](iotn828.h)
* [ATtiny2313   -  iotn2313.h](iotn2313.h)
* [ATtiny24     -    iotn24.h](iotn24.h)
* [ATtiny44     -    iotn44.h](iotn44.h)
* [ATtiny84     -    iotn84.h](iotn84.h)
* [ATtiny26     -    iotn26.h - evolved to the x61](iotn26.h)
* [ATtiny461    -   iotn461.h](iotn461.h)
* [ATtiny861    -   iotn861.h](iotn861.h)

## Headers omitted
We have only included headers for parts that this core supports. Meaning:
* No modern tinyAVRs (see below for definition).
* Nothing with less than 2k of flash (this includes some AVRe parts, as well as all AVRrc parts)
* Nothing with no RAM
* Nothing that is no longer in production. Note that no such parts qualified for support by this core.

## From the Manufactuer's Changelog
Note that most of these changes, since they post-date the release of most classic tinies, contain mostly information relevant only to modern AVRs. "AVR8X" devices are modern AVRs. The modern AVRs include all tinyAVR devices with 3 or 4 digit part numbers where a 0, 1, or 2 is in the tens place, a 2, 4, 5, or 7 is in the ones place, and the remaining 1 or 2 digits are equal to a power of two indicating flash size. Those parts have all new peripherals and a different instruction set variant and are supported by [megaTinyCore](https://github.org/SpenceKonde/megaTinyCore) not ATTinyCore. Most of them could also beat the stuffing out of not only classic tinyAVRs, but entry level ATmegas like the 328p.

We're now using the latest versions of the headers as of end of Q1 2023. The version history for pre 2.x.x versions has been formatted to match the formatting of the 2.0.368 changelog for improved readability. Really, there's not that much of note in these changes for the classic tinyAVRs (at least that we know of - they have a history of occasionally airbrushing out embarrasing oversights, like leaving the addresses and bitfield names for self-test peripherals, or revealing information about features that were "removed" only after the start of production, and hence only removed from the datasheet. They generally still leave the old versions of the headers up, but they don't call attention to the changes). This has revealed small secrets about the Dx-series, EA-series, and tinyAVR 2-series for certain, and likely happened with older parts in the past. Unfortunately the ATpacks only go back to 2015.


```text
2.0.368 (2022-03-02)
- Multiple updates to AVR8X ATDFs, including new mode description and updated ATDF schema to version 4.5.
- Updated FUSES initvals for AVR8X devices.
- Removed deprecated SIG-defines from ATtiny11/12/15.
- For AVR8X devices, added HV_IMPLEMENTATION property to indicate available UPDI HV option.
- For AVR8X devices, added an underscore between bitfield name and bitfield bit-number for bitfield defines on bitfields with more than one bit for both C-header and assembler .inc files.
- For AVR8X devices, changed LOCKBIT struct define name to LOCK_BIT to avoid redefine issue.
- For AVR8X devices, replaced master/slave with host/client in captions.
1.10.348 (2021-07-13)
- Corrected AVR-LibC device macro for ATtiny84A.
- Corrected SRAM on ATtiny214 simulator model.
1.9.337 (2021-03-01)
- Added support for ATtiny322x.
- Changed nomenclature for serial communication.
- Fixed mapping of RAM addresses 0x3C00-0x3FFF for ATtiny1617/1616/1614 RTL-based simulator model.
- Removed external 32kHz crystal option from ATtiny160x/80x/40x/20x.
- Added SYSCFG0.TOUTDIS for ATtiny42x/82x/162x.
- Added value NONE (0x00) to TCA0.SINGLE.EVCTRL EVACTB.
- Added CCL TRUTH bitfields for several devices.
- Joined CCL_SEQSEL0 and CCL_SEQSEL1 into one value-group, CCL_SEQSEL, for ATtiny42x/82x/162x.
1.8.332 (2020-10-30)
- Updated conditions in .pdsc for XC8 compiler.
1.7.330 (2020-09-18)
- Added support for ATtiny42x and ATtiny82x.
- Added missing CCL INSEL options for ATtiny161x and ATtiny321x.
- Corrected AC DACREF bitfield name for ATtiny162x.
- Added PAGE_SIZE for memories in several assembler include files.
1.6.326 (2020-08-27)
- Added XC8 support files.
- Updated copyright notice in c-header files.
- Corrected ADC MUXPOS and MUXNEG values, corrected PORTMUX.TCAROUTEA and PORTMUX.TCAROUTEB bitfields, added more values to other PORTMUX bitfields and renamed the ADC DAC source to DACREF0 for ATtiny162x devices.
1.5.315 (2020-06-04)
- Corrected MAPPED_ROGMEM rw-attribute for AVR8X devices.
- Timer/Counter caption correction.
- Corrected PORTMUX.EVSYSROUTEA bitfield names for ATtiny162x devices.
- Updated ATtiny162x simulator model.
1.4.310 (2020-04-03)
- Corrected analog functionality for PB1 and PB0 in ATtiny1627/1626/1624 ATDF files.
1.4.308 (2020-03-16)
- Corrected ADC modes for tiny1627/1626/1624.
- Corrected USART CTRLC reset value for ATtiny161x, ATtiny4xx and ATtiny2xx.
- Added LOCKBITS_DEFAULT for AVR8X devices.
1.4.301 (2020-01-28)
- Added ATtiny1627/1626/1624.
- Updates and corrections to CORE_VERSION.
- Removed unqualified BOD level values.
- Remove TCD0 from Attiny1604/1606/1607
1.4.283 (2019-10-18)
- Corrected TC2 instance in ATtiny441.
- Corrected defines for ATtiny1634.
- Refactored away family header files,
- now there is one header file per device.
- Corrected FUSE_MEMORY_SIZE for ATtiny4/5/9/10/20/40.
- Added simulator support for ATmega1609/1608/809/808.
- Added missing TCCR1B.PWMX bit for ATtiny261/261A/461/461A/861/861A.
- Updated fuse information for ATtiny-0/1 families.
- Added TCB1 in PORTMUX for ATtiny1614/1616/1617/3216/3217.
- In assembly include files list addresses with HEX and corrected interrupt vector offsets.
- Removed duplication of PROGMEM defines.
- Added support for pseudo ISR instruction in avr8-gcc.
- Group adjacent reserved bytes in arrays in the header files.
- Corrected PA1 and PA2 pad connection for ATtiny202/212/402/412.
1.3.229 (2018-09-17)
- Corrected signatures for ATtiny402 and ATtiny412.
- Typo corrections in captions.
- Corrected RW status on MCLKSTATUS register.
- Added initval-attribute for most registers in several devices.
- Made headerfile size defines unsigned.
- Corrected flash pagesizes in assembler include files for several devices.
1.3.172 (2018-02-19)
- Added TWSSRA bitfields for ATtiny20 and ATtiny40.
- Corrected Fuse register group size for ATtiny1607-family, ATtiny1617-family and ATtiny3217-family.
1.3.169 (2017-12-18)
- Added support for ATtiny1607, ATtiny1606, ATtiny1604, ATtiny807, ATtiny806, ATtiny804, ATtiny406, ATtiny404, ATtiny402, ATtiny204 and ATtiny202.
1.3.147 (2017-10-02)
- Updated AVR8X simulator models.
1.3.132 (2017-07-04)
- Added CCP signature for ATtiny102 and ATtiny104.
- Renamed interrupt ADC_ADC to ADC for ATtiny40.
- Renamed interrupt ANA_COMP to ANALOG_COMP for ATtiny48 and ATtiny88.
- Added Hi-/Lo-byte-address defines for 16-bit registers in assembler include files for the new tiny's.
- Added simulator model for ATtiny3217, ATtiny3216 and ATtiny3214.
1.2.118 (2017-02-21)
- Added support for ATtiny3217, ATtiny3216, ATtiny3214, ATtiny1617, ATtiny1616, ATtiny1614, ATtiny416, ATtiny414, ATtiny412, ATtiny214 and ATtiny212.
1.2.112 (2016-12-06)
- Updated header files for ATtiny417, ATtiny814, ATtiny816 and ATtiny817. Simulator support for the same devices.
- Added support for ATtiny1617 and ATtiny1616.
1.1.102 (2016-09-29)
- Added ATtiny417, ATtiny814, ATtiny816, and ATtiny817.
- Added GCC assembler support for ATtiny11, ATtiny12, and ATtiny15.
1.0.79 (2016-04-11)
- Added ATtiny102, ATtiny104, ATtiny80 and ATtiny840. Updated documentation links.
1.0.78 (2016-02-18)
- Added ATtiny102, ATtiny104, ATtiny80 and ATtiny840. Updated documentation links.
1.0.71 (2015-11-06)
- Fix simulator load issue for ATtiny85, ATtiny43U, ATtiny88, ATtiny1634, ATtiny45, ATtiny48, ATtiny40, ATtiny25, and ATtiny20.
1.0.70 (2015-10-28)
- Initial release.
```
