# Optiboot bootloader technical details

This directory contains the Optiboot small bootloader for AVR ATTiny classic devices, as distributed with ATTinyCore.

Optiboot is more fully described here: [http://github.com/Optiboot/optiboot](http://github.com/Optiboot/optiboot)

## Credit where credit is due
Optiboot is the work of **Peter Knight** (aka Cathedrow), building on work of **Jason P** **Kyle**, **Spiff**, and **Ladyada**.

More recent maintenance and modifications are by **Bill Westfield** (aka WestfW).

Adaptation of Optiboot_x to Optiboot_dx by **Spence Konde** (aka DrAzzy, Azduino).

## Reporting issues
This is being maintained as part of ATTinyCore, so issues be reported to:
[https://github.com/SpenceKonde/ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)

### Other resources

Arduino-specific issues are tracked as part of the Arduino project
at [http://github.com/arduino/Arduino](http://github.com/arduino/Arduino)

There is additional information in the wiki - however, the wiki does not reflect the changes specific to Optiboot for ATTinyCore
[https://github.com/Optiboot/optiboot/wiki](https://github.com/Optiboot/optiboot/wiki)


## To set up this environment in windows
This is how the files ATTinyCore ships with are built. I can provide no guarantee that any other method will still work after what I've done to the makefile.
1. Download and extract the Arduino 1.0.6 .zip package.
* Make sure it is not in a "protected" location. I normally put them in the root of C:.
* Rename the folder so you remember what it's for; I have it in `C:\arduino-1.0.6-5.4.0compiler` (this location will be used in the examples below)
* You may delete everything except the hardware and tools subdirectories
2. Download the 5.4.0 toolchain - or use the latest 7.3.0 one if brave.
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-armhf-pc-linux-gnu.tar.bz2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-armhf-pc-linux-gnu.tar.bz2)
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-aarch64-pc-linux-gnu.tar.bz2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-aarch64-pc-linux-gnu.tar.bz2)
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i386-apple-darwin11.tar.bz2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i386-apple-darwin11.tar.bz2)
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i686-w64-mingw32.zip](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i686-w64-mingw32.zip)
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i686-pc-linux-gnu.tar.bz2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-i686-pc-linux-gnu.tar.bz2)
* [http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-x86_64-pc-linux-gnu.tar.bz2](http://downloads.arduino.cc/tools/avr-gcc-5.4.0-atmel3.6.1-arduino2-x86_64-pc-linux-gnu.tar.bz2)
* For the brave:
* [https://spencekondetoolchains.s3.amazonaws.com/avr-gcc-7.3.0-atmel3.6.1-azduino7-i686-w64-mingw32.tar.bz2](https://spencekondetoolchains.s3.amazonaws.com/avr-gcc-7.3.0-atmel3.6.1-azduino7-i686-w64-mingw32.tar.bz2)
3. Extract the toolchain, and locate the top level 'avr' folder inside.
4. Copy the AVR folder to the `C:\arduino-1.0.6-5.4.0compiler\hardware\tools` directory in the 1.0.6 install. Be careful not to put it inside the avr folder that is already there (if you make a mistake, delete the mess and go back to step 1 - trying to unpick the mess is hopeless, because there are like 4 nested folders named avr, 2 folders in different locations named bin, and several 'include' folders. Don't look at me, I didn't choose that directory structure!
5. Copy the `optiboot` folder from `(core install location)/megaavr/bootloaders` to `C:\arduino-1.0.6-5.4.0compiler\hardware\arduino\bootloaders`.
6. You should now be able to open a command window in `C:\arduino-1.0.6-5.4.0compiler\hardware\arduino\bootloaders\optiboot` and run `omake <target>`

This can be done the same way in linux except that you would download the version of the toolchain aoppropriate to your OS, and you can expect make to actually be there, ignore omake, and just use the shell scripts instead of batch files. which If i had time to work on, could be mde a lot smoother, but that's not a priority (I think the main thing we'd want to to is pass the avr-size output through grep or something, but I don't know makefile or shellscripting :-) )


## Previous build instructions
These may or may not work. I am not versed in the ways of makefiles and am not competent to fix any issues that may be encountered. Thus, if you want to use one of these methods, you'll have to fix the makefile yourself (sorry - my brain uses 16-bit addressing. You'll notice I never talk about working with an ATmega2560 - that would require an extra bit, ya see? So figuring out how to wrangle a makefile on a 64-bit desktop computer is well beyond my ability). I will gladly accept pull requests that fix these things - as long as they do not break the method detailed above, as that would leave me unable to build DxCore bootloaders without toolchain wrangling, which as I said, is outside my zone of competence. If you find that these do in fact work as written, please let me know so I can update this file.

### Building optiboot for Arduino
Production builds of optiboot for Arduino are done on a Mac in "unix mode"
using CrossPack-AVR-20100115.  CrossPack tracks WINAVR (for windows), which
is just a package of avr-gcc and related utilities, so similar builds should
work on Windows or Linux systems.

One of the Arduino-specific changes is modifications to the makefile to
allow building optiboot using only the tools installed as part of the
Arduino environment, or the Arduino source development tree.  All three
build procedures should yield identical binaries (.hex files) (although
this may change if compiler versions drift apart between CrossPack and
the Arduino IDE.)


### Building Optiboot in the Arduino IDE Install
Work in the .../hardware/arduino/bootloaders/optiboot/ and use the
`omake <targets>` command, which just generates a command that uses
the arduino-included "make" utility with a command like:
    `make OS=windows ENV=arduino <targets>`
or  `make OS=macosx ENV=arduino <targets>`
On windows, this assumes you're using the windows command shell.  If
you're using a cygwin or mingw shell, or have one of those in your
path, the build will probably break due to slash vs backslash issues.
On a Mac, if you have the developer tools installed, you can use the
Apple-supplied version of make.
The makefile uses relative paths (`../../../tools/` and such) to find
the programs it needs, so you need to work in the existing optiboot
directory (or something created at the same "level") for it to work.



### Building Optiboot in the Arduino Source Development Install
In this case, there is no special shell script, and you're assumed to
have "make" installed somewhere in your path.
Build the Arduino source ("ant build") to unpack the tools into the
expected directory.
