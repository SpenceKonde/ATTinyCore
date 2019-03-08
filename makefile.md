#Using ATTinyCore with Makefiles#

The highly popular [Sudar
makefile](https://github.com/sudar/Arduino-Makefile) works with
ATTinyCore once properly installed and configured. The Sudar
makefile suite consists of two main parts – a file named
“Arduino.mk” that resides in a folder named Arduino-Makefile and a
Makefile that you create for your project and locate in your
project's folder.  You don't need to change anything in the Arduino
sketches. What compiles with the Arduion IDE will also compile with
the Sudar makefile suite.

##Prerequisite:##

If you have not already do so, **install the Arduino IDE** according
to the instructions at the [Arduino.cc
website](https://www.arduino.cc/en/Main/Software).

If you have not already do so, **install SpenceKonde/ATTinyCore**
according to the instructions in the
[SpenceKonde/ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)
GitHub file *Installation.md*.

**Install the Sudar makefile suite** by following the installation
instructions at the [sudar/Arduino-Makefile
Github](https://github.com/sudar/Arduino-Makefile).

The most complex part of installation is creating your project
Makefile. You need to know the installation  pathnames of
SpenceKonde/ATTinyCore and  sudar/Arduino-Makefile. These locations
can vary depending on your operating system and system
configuration.

Examining the Makefile below as well as its
console output, also below, will provide one example to help guide
you.

Assuming that your installation of ATTinyCore resides at
~/Arduino/hardware then entering this grep command will provide a
useful list of ATTinyCore options for your chosen processor (selected from the list below).
**NOTE:** this Linux command requires [Cygwin](https://www.cygwin.com/) on Microsoft Windows.

**grep attinyx5 ~/Arduino/hardware/ATTinyCore/avr/boards.txt**

At the time this was written, these are the ATTiny chips supported by ATTinyCore:

* attinyx4
* attinyx5
* attinyx8
* attinyx7
* attinyx7opti
* attinyx61
* attinyx41
* attiny841opti
* attiny828
* attiny828opti
* attiny1634
* attiny1634opti
* attinyx313

You can obtain a new copy of the list above by entering this command in a serial console:

**awk -F. '/att/ {print $1}' ~/Arduino/hardware/ATTinyCore/avr/boards.txt|uniq**


An example of a working Makefile is shown below.
<br>
```
### DISCLAIMER
### This Makefile is designed for use with the sudar Arduino.mk
### Refer to https://github.com/sudar/Arduino-Makefile
### Currently sudar version 1.5.2 (2017-01-11)
###
### Enter make help for all runtime options

### PROJECT_DIR
### This is the base path to where you have created/cloned your project
PROJECT_DIR       = /home/username/Arduino

### AVR_GCC_VERSION
### Check if the version is equal or higher than 4.9
AVR_GCC_VERSION  := $(shell expr `avr-gcc -dumpversion | cut -f1` \>= 4.9)

### ARDMK_DIR
### Path to the Arduino-Makefile directory.
ARDMK_DIR         = $(PROJECT_DIR)/Arduino-Makefile

### ARDUINO_DIR
### Path to the Arduino application and resources directory.
ARDUINO_DIR       = /home/username/arduino-1.6.11

### USER_LIB_PATH
### Path to where the your project's libraries are stored.
USER_LIB_PATH     :=  $(realpath $(PROJECT_DIR)/arduino-1.6.10/hardware/tools/avr/avr/include)

### BOARD_TAG & BOARD_SUB
### For Arduino IDE 1.0.x Only BOARD_TAG is needed.
### BOARD_TAG         = mega2560 (example)
### For Arduino IDE 1.6.x Both BOARD_TAG and BOARD_SUB are needed.
### They must be set to the board you are currently using.
### Note: for the Arduino Uno, only BOARD_TAG is mandatory
### BOARD_TAG         = mega (example)
### BOARD_SUB         = atmega2560 (example)
BOARD_TAG         = attinyx5
BOARD_SUB         = 85
### For ATTinyCore
ALTERNATE_CORE    = ATTinyCore
F_CPU=8000000L

### MONITOR_BAUDRATE
### It must be set to Serial baudrate value you are using.
MONITOR_BAUDRATE  = 115200

### AVRDUDE
ISP_PORT          = /dev/ttyS0
AVRDUDE_ARD_PROGRAMMER = usbasp
AVRDUDE_ARD_BAUDRATE = 9600
### Path to avrdude directory.
AVRDUDE          = /usr/bin/avrdude
### Path to the AVR tools directory such as avr-gcc, avr-g++, etc.
AVR_TOOLS_DIR     = /home/username/arduino-1.6.11/hardware/tools/avr

### CFLAGS_STD
### Set the C standard to be used during compilation. Documentation \ (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cflags_std)
CFLAGS_STD        = -std=gnu11

### CXXFLAGS_STD
### Set the C++ standard to be used during compilation. Documentation \ (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cxxflags_std)
CXXFLAGS_STD      = -std=gnu++11

### CXXFLAGS
### Flags you might want to set for debugging purpose. Comment to stop.
CXXFLAGS         += -pedantic -Wall -Wextra

### MONITOR_PORT
### The port your board is connected to. Using an '*' tries all the ports and finds the right one.
MONITOR_PORT      = /dev/ttyS0

### CURRENT_DIR
### Do not touch - used for binaries path
CURRENT_DIR       = $(shell basename $(CURDIR))

### OBJDIR
### This is where you put the binaries you just compile using 'make'
OBJDIR            = $(PROJECT_DIR)/$(CURRENT_DIR)/$(BOARD_TAG)/bin

### Do not touch - the path to Arduino.mk, inside the ARDMK_DIR
include $(ARDMK_DIR)/Arduino.mk
```
<br>
**Below, is the console output from this Makefile:**
<br>

```
$ make
-------------------------
Arduino.mk Configuration:
- [AUTODETECTED]       CURRENT_OS = LINUX
- [USER]               ARDUINO_DIR = /home/username/arduino-1.6.11
- [USER]               ARDMK_DIR = /home/username/Arduino/Arduino-Makefile
- [AUTODETECTED]       ARDUINO_VERSION = 1611
- [DEFAULT]            ARCHITECTURE = avr
- [DEFAULT]            ARDMK_VENDOR = arduino
- [AUTODETECTED]       ARDUINO_PREFERENCES_PATH = /home/username/.arduino15/preferences.txt
- [AUTODETECTED]       ARDUINO_SKETCHBOOK = /home/username/Arduino (from arduino preferences file)
- [USER]               AVR_TOOLS_DIR = /home/username/arduino-1.6.11/hardware/tools/avr
- [COMPUTED]           ARDUINO_LIB_PATH = /home/username/arduino-1.6.11/libraries (from ARDUINO_DIR)
- [COMPUTED]           ARDUINO_PLATFORM_LIB_PATH = /home/username/arduino-1.6.11/hardware/arduino/avr/libraries (from ARDUINO_DIR)
- [USER]               ALTERNATE_CORE = ATTinyCore
- [COMPUTED]           ALTERNATE_CORE_PATH = /home/username/Arduino/hardware/ATTinyCore/avr  (from ARDUINO_SKETCHBOOK and ALTERNATE_CORE)
- [COMPUTED]           ARDUINO_VAR_PATH = /home/username/Arduino/hardware/ATTinyCore/avr/variants (from ALTERNATE_CORE_PATH)
- [COMPUTED]           BOARDS_TXT = /home/username/Arduino/hardware/ATTinyCore/avr/boards.txt (from ALTERNATE_CORE_PATH)
- [DEFAULT]            USER_LIB_PATH = /home/username/Arduino/libraries (in user sketchbook)
- [DEFAULT]            PRE_BUILD_HOOK = pre-build-hook.sh
- [USER]               BOARD_SUB = 85
- [USER]               BOARD_TAG = attinyx5
- [COMPUTED]           CORE = tiny (from build.core)
- [COMPUTED]           VARIANT = tinyX5 (from build.variant)
- [USER]               OBJDIR = /home/username/Arduino/ULPFlasher3/attinyx5/bin
- [COMPUTED]           ARDUINO_CORE_PATH = /home/username/Arduino/hardware/ATTinyCore/avr/cores/tiny (from ALTERNATE_CORE_PATH, BOARD_TAG and boards.txt)
- [USER]               MONITOR_BAUDRATE = 115200
- [DEFAULT]            OPTIMIZATION_LEVEL = s
- [DEFAULT]            MCU_FLAG_NAME = mmcu
- [USER]               CFLAGS_STD = -std=gnu11
- [USER]               CXXFLAGS_STD = -std=gnu++11
- [COMPUTED]           DEVICE_PATH = /dev/ttyS0 (from MONITOR_PORT)
- [DEFAULT]            FORCE_MONITOR_PORT =
- [AUTODETECTED]       Size utility: AVR-aware for enhanced output
- [COMPUTED]           BOOTLOADER_PARENT = /home/username/arduino-1.6.11/hardware/arduino/avr/bootloaders (from ARDUINO_DIR)
- [COMPUTED]           ARDMK_VERSION = 1.5
- [COMPUTED]           CC_VERSION = 4.9.2 (avr-gcc)
-------------------------
mkdir -p /home/username/Arduino/ULPFlasher3/attinyx5/bin
/home/username/arduino-1.6.11/hardware/tools/avr/bin/avr-g++ -x c++ -include Arduino.h -MMD -c -mmcu=attiny85 -DF_CPU=8000000L -DARDUINO=1611 -DARDUINO_ARCH_AVR -D__PROG_TYPES_COMPAT__ -I/home/username/Arduino/hardware/ATTinyCore/avr/cores/tiny -I/home/username/Arduino/hardware/ATTinyCore/avr/variants/tinyX5    -Wall -ffunction-sections -fdata-sections -Os -pedantic -Wall -Wextra -fpermissive -fno-exceptions -std=gnu++11 ULPFlasher3.ino -o /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.ino.o
/home/username/arduino-1.6.11/hardware/tools/avr/bin/avr-gcc -mmcu=attiny85 -Wl,--gc-sections -Os -flto -fuse-linker-plugin -o /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.elf /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.ino.o /home/username/Arduino/ULPFlasher3/attinyx5/bin/libcore.a   -lc -lm
/home/username/arduino-1.6.11/hardware/tools/avr/bin/avr-objcopy -j .eeprom --set-section-flags=.eeprom='alloc,load' \
		--no-change-warnings --change-section-lma .eeprom=0 -O ihex /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.elf /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.eep
/home/username/arduino-1.6.11/hardware/tools/avr/bin/avr-objcopy -O ihex -R .eeprom /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.elf /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.hex

/home/username/arduino-1.6.11/hardware/tools/avr/bin/avr-size --mcu=attiny85 -C --format=avr /home/username/Arduino/ULPFlasher3/attinyx5/bin/ULPFlasher3.elf
AVR Memory Usage
----------------
Device: attiny85

Program:    1018 bytes (12.4% Full)
(.text + .data + .bootloader)

Data:         13 bytes (2.5% Full)
```
