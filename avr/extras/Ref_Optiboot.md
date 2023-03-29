# Optiboot Support - serial bootloader for ATtiny441/841, 44/84, 45/85, 461/861, 48/88, 828, 1634, 87, 167

The Optiboot bootloader is included for the ATtiny441, 841, 44, 84, 45, 85, 461, 861, 48, 88, 1634, 828, 87 and 167. In other words, not for 2k parts, amd not for the 43 and 4313 - the defining difference is that those do not have a larger version available. Uae of optiboot is

On parts with a hardware serial port, this is used (those lucky parts with two can choose the serial port from a tools submenu), other boards will use a software serial implementation (on these boards TX/RX pins are the same as the builtin software serial, the ones marked AIN0 (TX) and AIN1(RX)). The baud rate used depends on the system clock speed as shown in the table below.  Once the bootloader is programmed, the target can be programmed over serial; the bootloader will run after reset, just like on a normal Arduino. Bootloaders are not included for the 2k-flash versions of these parts - more than a quarter of the available flash would go to the bootloader, and the Arduino abstraction is already severely constrained with just 2k of flash. For these parts, we recommend ISP programming. Clock speeds lower than 1MHz are not supported because the resulting baud rates would be so low that it some serial adapters start having problems talking to them. At 1MHz, uploads are painfully slow. We recommend using the bootloader only at 8MHz or higher; however, we do support use of the 3.686 MHz crystal to support operation at 1.8V while getting an acceptable upload speed.

## Baud Rates used
2.0.0 will use the following baud rates; this represents a change from previous versions, and will require reburning the bootloader (that's why it's a major version).

### Changes from previous versions
Notably, for parts with a hardware serial port that is NOT a LIN port, the 16 MHz and 8 MHz bootloaders were both moved to 76800 baud, which doesn't have as much baud rate calculation error as 115200/57600 baud does (actually, it has less error at 8 MHz than 57600 does at 16 MHz - 57600 and 115200 baud are some of the worst numbers that could have been chosen for the AVR baud rate generator). While less well known than 115200 baud and 57600, it is a standard baud rate `(38400 * 1.5 = 57600, 38400 * 2 = 76800`, following the pattern of baud rates increasing by either a factor of 1.5 or 2x) and is supported by the common serial adapters. On the other hand, hardware serial parts running at 12 MHz previously used 57600 baud. This was unnecessarily conservative - 12 MHz has virtually no calculation error at 115200 baud.

Baud rates used parts with hardware serial ports did not take full advantage of USART crystals where userd. Those are crystals with whack job frequencies that are chosen because they can be evenly divided to the common baud rates, For example, 18.432 MHz (18,432,000 Hz) - 18432000/115200 - 160. Since an AVR needs to divide it'a cloxk to get a speed 8 times the baud rate, it would divide by 20 (or 10 if not using U2X mode), meaning UBRR values of 19 or 9 respectively. 7.372? Divide by 115200 and you get 64. We use 115200 baud all the way down to 7.327 MHz for the USART crystals.

For the odd case of the 3.686 MHz UART clock, new in 2.0.0, we still limit the clock speed to 57600 baud, as we are starting to ask a lot of Optiboot to expect it to get and process a byte that quickly.

On the 87 and 167, which have a much more capable baud rate generator, we don't have to worry about the baud rate accuracy in this sense.

Meanwhile on the parts with software serial, there are also no "magic values" (well, there are sort of, but they are less well defined, and the magnitude of the effect is smaller over relevant conditions. If you think those baud rates look wildly optimistic for software serial, I thought so too, but the optiboot code claims to work at 4800 baud and 1 MHz, and these are all a lower ratio of baud : clock than that). Much lower values were used in the past, skirting the minimum that Optiboot can be built for, at higher speeds. I am uncertain of why such a low baud rate was chosen.

Previously, the baud rate selections were also not always consistent, a consequence of hand-maintaining a boards.txt and makefile far too long for doing by hand.

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
|    3.686 MHz |       56700 |        14400** |                  56700** |
|    7.372 MHz |      115200 |        38400** |                 115200** |
|    9.216 MHz |      115200 |        38400** |                 115200** |
|  11.0592 MHz |      115200 |        38400** |                 115200** |
|  14.7456 MHz |      115200 |        57600** |                 115200** |
|   18.432 MHz |      115200 |        57600** |                 115200** |

`*` On these parts, the chip is started in 1 MHz mode via the CKDIV8 fuse, and that is the speed that Optiboot runs at, and only switched to the specified speed once the sketch starts running. One of the side benefits of this is that in many cases a single binary can be used for all of these speeds, as well as the 8 MHz (internal or xtal) or 4 MHz xtal cases, reducing the absurd number of bootloader binaries that we had to distribute.

`**` These are USART crystals (they divide perfectly to common baud rates), and on parts with a hardware serial port, they eliminate calculation error in the baud rates. That's why can run the bootloader on parts with hardware serial at such high baud rates vs their clock speed - all the common baud rates come out perfectly (the limit on baud rate is from the calculation accuracy, not the speed of the target chip). The cost is that all other timekeeping is worse (takes longer to return and/or is less accurate). **there is no benefit to using a USART crystal on parts without hardware serial** as the math there doesn't have the factor-of-eight reduction in precision that comes with the hardware serial port. Meanwhile, on the 167/87, the baud rate generator is much better and can achieve high calculation accuracy without a USART crystal.

16.5 and 12.8 MHz are not supported for Optiboot. Those speeds are achieved by tuning performed by the bootloader (if micronucleus, which of course isn't optiboot) or by the initialization code that runs before setup (but after the bootloader). Even when the target speed is 16.5 or 12.8 via tuning, Optiboot will run at 16.0 or 8.0 MHz.

## Warning - never rely on optiboot not failing
Only the ATtiny828 has support for a hardware bootloader section. For everything else, "trick" was used. To make a bootloader work, it needs to do two key things (well, in addition to the detail of accepting a program over serial and writing it to the flash - but a sketch could do that too): It must run after reset even after an application has been uploaded, and it must be able to start the application when it has either written a new sketch or determined that one is not forthcoming. These are handled automatically where there is hardware bootloader support - there's a `BOOTRST` fuse to tell it to go to the bootloader section at the end of the flash, and a `BOOTSZ` fuse to say how large that section is (and hence how far from the end of the flash it should jump to). And then it need only get program execution to 0x0000 to start the application, either by jumping to it directly, or even just letting the program counter roll over from the very end of the flash.

This is much harder without the `BOOTRST` fuse. The bootloader still gets located at the end of flash, but now, you have to figure out some way to ensure that instead of the application running at startup, the bootloader does instead. There is only one way to achieve that, and it is be modifying the sketch being uploaded on the the fly. This is called "Virtual Boot" The instruction at 0x0000 (along with the interrupt "vector table" that follows it) is always either an `rjmp` ("relative jump") on parts with 8k or less flash or a `jmp` ("Jump") on parts with more. For a bootloader which starts N words before the end of the flash, an rjmp of N+1 words backwards (it starts counting from the start of the following word) can replace the reset vector to solve the first problem on 8k parts - and on 16k parts, it's even easier: jmp is a two word instruction, and the second word is simply the destination of the jump, so that can be directly replaced with the address of the start of the bootloader. Obviously, we need to retain the original reset vector, otherwise we couldn't start the application. And it is here that Optiboot made a terrible error: They picked another vector in the table of interrupt vectors, we have it set to use the EEPROM ready interrupt (`EE_RDY`) which is not used by any arduino library or code that I am aware of. The problem comes when erasing the flash to upload a new program.

Optiboot erases pages when using virtual boot the same way as it does when it has the insurance of a hardware bootloader (ie, the simplest way possible): Whenever it is fed the first byte of a page, it first erases that page, and then proceeds with programming. That all is fine as long as you have a BOOTRST fuse to guarantee that you can get back to the bootloader. A failure of some sort at this state would simply require reuploading the sketch. But with virtual boot, if it has erased the first page, but not rewritten it, it neither knows how to run the bootloader or start the application. It is bricked until it can be reprogrammed with an ISP programmer. Bitter experience has shown that this does happen in the field, particularly when running on batteries (where a "brownout", where weak batteries drop below the minimum voltage required (by BOD or physics) from the current of the erase process), from a mishandled programming attempt (where an attempt is started, abandoned with the port being closed, and retried) on a board with autoreset, or from other unknown adverse events. Murphy's law dictates that the greater the cost and inconvenience of trudging out there to reprogram the device, the higher the chance of it failing like this.

**There is a well known and extensively tested (Micronucleus uses it) solution**: The moment that the booloader is told to write anything, you erase every page of flash outside the bootloader section, starting with the end and finishing with the first page (the one with the reset vector, which if you recall is pointed to the bootloader), and the application vector must be placed not in the vector table at the start of the flash, but rather as the last word of the application section. Thus, if an erase is incomplete, the application will be hosed, but the bootloader will still run, unless the erase completed, in which case there is nothing but empty flash between the reset vector and the bootloader. Empty flash holds all 1's. `0xFFFF`, despite being an invalid opcode, is interpreted as a "skip [next instruction] if bit 7 in 31 is set" (sbrs r31, 7) (opcode 0xFFF7) - so depending on the value of r31, it will skid along the empty flash executing and skipping alternating words, or executing every word. Since the only instructions running are `sbrs r31,7` sbrs will either skip every other instruction, or not, as there's no way for r31's value to change until it reaches non-empty flash. Since the bootloader must be an integer number of pages in size, and pages are always even numbers of words in size (typical page sizes are 32b to 128b), it will never miss the first instruction of the bootloader. Thus the case of a complete erase with no write will reach the bootloader. And if a failure occurs after writing the first page of flash, but before finishing the last, the bootloader will jump to the last location in the application section to try to start the app. That will be empty flash, from whence it will proceed to the bootloader and the upload could be reattempted.

Optiboot needs to be adapted to do that. I wasn't able to see a clear route towards doing that when I last looked into that adaptation.

**Because of this issue, Optiboot should not be used for production systems except on the ATtiny828 or other non-ATTinyCore supported parts with hardware bootloader support.**
This is also why we do not provide support for disabling reset on Optiboot board definitions: Sooner or later, it'll end up bricked.

## Entering the Bootloader
Two versions of every configuration are supplied - one for use with auto-reset, which waits 1 second for commuinication and does not run at power on reset, and one which waits 8 seconds and does run on power on reset, for use with manual reset schemes.

## Optiboot and MCUSR (reset cause)
In order to make the entry modes work correctly - regardless of sketch behavior - Optiboot for ATTinyCore must reset MCUSR prior to exiting. It stashes the value of MCUSR in the GPIOR0 register - in the unlikely event that your sketch needs to know the reset cause, the most efficient wat to do this is shown below.

```
uint8_t resetcause = GPIOR0;  // in reg, GPIOR0
GPIOR0 = 0;                   // out GPIOR0, __zero_reg__
if(resetcause != 0) {         // cpse reg, __zero_reg__
  resetcause = MCUSR;         // in reg, MCUSR
}
MCUSR=0;                      // out MCUSR, __zero_reg__
// 10 bytes, 5 words. Putting MCUSR = 0 inside the conditional is unnecessary, and costs an extra instruction or two.
```
If both GPIOR0 and MCUSR are 0 at the start of setup, and you use code like that shown above in your setup, that means that MCUSR has been cleared since the last reset and not stored to GPIOR. This likely happened because code execution ended up back at 0x0000 either intentionally or unintentionally (it commonly is the end result when the stack is corrupted leading to code attempting to 'return' to a bogus address - this is frequently what causes the chip to get into a bad state from which it needs a manual reset to fix, since no reset cycle occurs, so the peripheral registers are not in the expected state at startup after the "dirty reset". People also sometimes use that as a method of software reset (not recommended, because it doesn't actually reset the current state.).

In the even more unlikely event that your sketch uses GPIOR0, be sure to set it to 0 in setup. [What is GPIOR0?](https://github.com/SpenceKonde/AVR-Best-Practices/blob/master/LowLevelNotes.md)

## Number of included binaries
Each clock speed requires it's own bootloader binary, and has two entry conditions, and in addition to each of the parts requiring their own bootloader binaries, there are several parts that get more than one: The 1634 can use either USART0 or USART1, the 441/841 can use either USART0, USART1, or USART0 in it's alternate pin mapping, and the 461 and 861 have 3 options for the software serial to match the tinySoftSerial configuration options. The number of bootloaders required borders on the absurd: ATTinyCore 2.0.0 includes no less than 586 hex files (though in the past it was already close to 200). Makefile.tiny should not be hand-edited - it is automatically generated along with makeall.tiny.bat when create_boards_txt.py is run.

### Building Optiboot - with apologies to Linux and sane developers
I like programming embedded systems. I do not like setting up build environments. My build environment is created as follows:
1. Unzip the 1.0.6 version of the Arduino IDE (zip package) in a location outside of program files.
2. Delete all contents except for the `hardware` and `tools` folders. Within the the hardware folder, delete everything except `Arduino` and `tools`. In `hardware/tools`, delete everything except `avr`. In `hardware/Arduino`, delete everything except `bootloaders` (this greatly shrinks the package size allowing the build environment to be copied between machines quickly)
3. Download [the Arduino toolchain for Windows platforms, v5.4.0 arduino2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i686-w64-mingw32.zip) (I suspect a more recent toolchain would work, but I see little possible upside to that, and it could cause unforeseen problems - zero gain non-zero risk = bad deal)
4. Extract it, there will be a folder called 'avr' - copy it into tools, and you should be prompted to replace several thousand files. Do so. There are several nested folders named AVR, and you will still match some file names if you copy these 1 layer too deep or too shallow. Start over if you do - trying to fix the resulting mess is hopeless.
5. Copy (repository root)/avr/bootloaders/optiboot/source to a location under (1.0.6 arduino skeleton)/hardware/arduino/bootloaders.
6. If create_boards_txt was modified, be sure to run it and copy the new Makefile.tiny and makeall.tiny.bat from (repository root)/avr/extras/development (copying boards.txt to (repository root)/avr is required as well - this is to permit manual inspection.)

If it wasn't clear from the above, I am unlikely to be able to answer questions about building on other platforms, or setting up a build environment that differs in any way. That is pretty much the extent of my knowledge of setting up the build environment.
