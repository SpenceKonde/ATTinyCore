# Makefile for ATmegaBOOT
# E.Lins, 18.7.2005
# $Id$
#
# Instructions
#
# To make bootloader .hex file:
# make diecimila
# make lilypad
# make ng
# etc...
#
# To burn bootloader .hex file:
# make diecimila_isp
# make lilypad_isp
# make ng_isp
# etc...
#
# Edit History
# 201406xx: WestfW: More Makefile restructuring.
#                   Split off Makefile.1284, Makefile.extras, Makefile.custom
#                   So that in theory, the main Makefile contains only the
#                   official platforms, and does not need to be modified to
#                   add "less supported" chips and boards.
# 201303xx: WestfW: Major Makefile restructuring.
#                   Allows options on Make command line "make xx LED=B3"
#                   (see also pin_defs.h)
#                   Divide into "chip" targets and "board" targets.
#                   Most boards are (recursive) board targets with options.
#                   Move isp target to separate makefile (fixes m8 EFUSE)
#                   Some (many) targets will now be rebuilt when not
#                     strictly necessary, so that options will be included.
#                     (any "make" with options will always compile.)
#                   Set many variables with ?= so they can be overridden
#                   Use arduinoISP settings as default for ISP targets
#

#----------------------------------------------------------------------
#
# program name should not be changed...
PROGRAM    = optiboot

# The default behavior is to build using tools that are in the users
# current path variables, but we can also build using an installed
# Arduino user IDE setup, or the Arduino source tree.
# Uncomment this next lines to build within the arduino environment,
# using the arduino-included avrgcc toolset (mac and pc)
# ENV ?= arduino
# ENV ?= arduinodev
# OS ?= macosx
# OS ?= windows

# export symbols to recursive makes (for ISP)
export

# defaults
MCU_TARGET = atmega168
LDSECTIONS  = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe

# Build environments
# Start of some ugly makefile-isms to allow optiboot to be built
# in several different environments.  See the README.TXT file for
# details.

# default
fixpath = $(1)
SH := bash

ifeq ($(ENV), arduino)
# For Arduino, we assume that we're connected to the optiboot directory
# included with the arduino distribution, which means that the full set
# of avr-tools are "right up there" in standard places.
# (except that in 1.5.x, there's an additional level of "up")
TESTDIR := $(firstword $(wildcard ../../../tools/*))
ifeq (,$(TESTDIR))
# Arduino 1.5.x tool location compared to optiboot dir
  TOOLROOT = ../../../../tools
else
# Arduino 1.0 (and earlier) tool location
  TOOLROOT = ../../../tools
endif
GCCROOT = $(TOOLROOT)/avr/bin/

ifeq ($(OS), windows)
# On windows, SOME of the tool paths will need to have backslashes instead
# of forward slashes (because they use windows cmd.exe for execution instead
# of a unix/mingw shell?)  We also have to ensure that a consistent shell
# is used even if a unix shell is installed (ie as part of WINAVR)
fixpath = $(subst /,\,$1)
SHELL = cmd.exe
SH = sh
endif

else ifeq ($(ENV), arduinodev)
# Arduino IDE source code environment.  Use the unpacked compilers created
# by the build (you'll need to do "ant build" first.)
ifeq ($(OS), macosx)
TOOLROOT = ../../../../build/macosx/work/Arduino.app/Contents/Resources/Java/hardware/tools
endif
ifeq ($(OS), windows)
TOOLROOT = ../../../../build/windows/work/hardware/tools
endif

GCCROOT = $(TOOLROOT)/avr/bin/
AVRDUDE_CONF = -C$(TOOLROOT)/avr/etc/avrdude.conf

else
GCCROOT =
AVRDUDE_CONF =
endif

STK500 = "C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe"
STK500-1 = $(STK500) -e -d$(MCU_TARGET) -pf -vf -if$(PROGRAM)_$(TARGET).hex \
           -lFF -LFF -f$(HFUSE)$(LFUSE) -EF8 -ms -q -cUSB -I200kHz -s -wt
STK500-2 = $(STK500) -d$(MCU_TARGET) -ms -q -lCF -LCF -cUSB -I200kHz -s -wt
#
# End of build environment code.


OBJ        = $(PROGRAM).o
OPTIMIZE = -Os -fno-split-wide-types -mrelax

DEFS       = 

#
# platforms support EEPROM and large bootloaders need the eeprom functions that
# are defined in libc, even though we explicity remove it with -nostdlib because
# of the space-savings.
LIBS       =  -lc

CC         = $(GCCROOT)avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = $(LDSECTIONS) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections

OBJCOPY        = $(GCCROOT)avr-objcopy
OBJDUMP        = $(call fixpath,$(GCCROOT)avr-objdump)

SIZE           = $(GCCROOT)avr-size

#
# Make command-line Options.
# Permit commands like "make atmega328 LED_START_FLASHES=10" to pass the
# appropriate parameters ("-DLED_START_FLASHES=10") to gcc
#

ifdef BAUD_RATE
BAUD_RATE_CMD = -DBAUD_RATE=$(BAUD_RATE)
dummy = FORCE
else
BAUD_RATE_CMD = -DBAUD_RATE=115200
endif

ifdef LED_START_FLASHES
LED_START_FLASHES_CMD = -DLED_START_FLASHES=$(LED_START_FLASHES)
dummy = FORCE
else
LED_START_FLASHES_CMD = -DLED_START_FLASHES=3
endif

# BIG_BOOT: Include extra features, up to 1K.
ifdef BIGBOOT
BIGBOOT_CMD = -DBIGBOOT=1
dummy = FORCE
endif

ifdef SOFT_UART
SOFT_UART_CMD = -DSOFT_UART=1
dummy = FORCE
endif

ifdef LED_DATA_FLASH
LED_DATA_FLASH_CMD = -DLED_DATA_FLASH=1
dummy = FORCE
endif

ifdef LED
LED_CMD = -DLED=$(LED)
dummy = FORCE
endif

ifdef SINGLESPEED
SSCMD = -DSINGLESPEED=1
endif

COMMON_OPTIONS = $(BAUD_RATE_CMD) $(LED_START_FLASHES_CMD) $(BIGBOOT_CMD)
COMMON_OPTIONS += $(SOFT_UART_CMD) $(LED_DATA_FLASH_CMD) $(LED_CMD) $(SSCMD)

#UART is handled separately and only passed for devices with more than one.
ifdef UART
UARTCMD = -DUART=$(UART)
endif

# Not supported yet
# ifdef SUPPORT_EEPROM
# SUPPORT_EEPROM_CMD = -DSUPPORT_EEPROM
# dummy = FORCE
# endif

# Not supported yet
# ifdef TIMEOUT_MS
# TIMEOUT_MS_CMD = -DTIMEOUT_MS=$(TIMEOUT_MS)
# dummy = FORCE
# endif
#

#.PRECIOUS: %.elf

#---------------------------------------------------------------------------
# "Chip-level Platform" targets.
# A "Chip-level Platform" compiles for a particular chip, but probably does
# not have "standard" values for things like clock speed, LED pin, etc.
# Makes for chip-level platforms should usually explicitly define their
# options like: "make atmega1285 AVR_FREQ=16000000L LED=D0"
#---------------------------------------------------------------------------
#
# Note about fuses:
# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits, see:
# http://tinker.it/now/2007/02/24/the-tale-of-avrdude-atmega168-and-extended-bits-fuses/
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x2f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.
#---------------------------------------------------------------------------



attiny167at16: TARGET = attiny167
attiny167at16: MCU_TARGET = attiny167
attiny167at16: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' 
attiny167at16: AVR_FREQ ?= 16000000L
attiny167at16: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny167at16: $(PROGRAM)_attiny87_16000000L.hex
attiny167at16: $(PROGRAM)_attiny87_16000000L.lst


attiny167at8: TARGET = attiny167
attiny167at8: MCU_TARGET = attiny167
attiny167at8: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DBAUD_RATE=57600'
attiny167at8: AVR_FREQ ?= 8000000L
attiny167at8: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny167at8: $(PROGRAM)_attiny167_8000000L.hex
attiny167at8: $(PROGRAM)_attiny167_8000000L.lst



attiny167at12: TARGET = attiny167
attiny167at12: MCU_TARGET = attiny167
attiny167at12: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' 
attiny167at12: AVR_FREQ ?= 12000000L
attiny167at12: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny167at12: $(PROGRAM)_attiny167_12000000L.hex
attiny167at12: $(PROGRAM)_attiny167_12000000L.lst


attiny87at16: TARGET = attiny87
attiny87at16: MCU_TARGET = attiny87
attiny87at16: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' 
attiny87at16: AVR_FREQ ?= 16000000L
attiny87at16: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny87at16: $(PROGRAM)_attiny87_16000000L.hex
attiny87at16: $(PROGRAM)_attiny87_16000000L.lst


attiny87at8: TARGET = attiny87
attiny87at8: MCU_TARGET = attiny87
attiny87at8: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DBAUD_RATE=57600'
attiny87at8: AVR_FREQ ?= 8000000L
attiny87at8: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny87at8: $(PROGRAM)_attiny87_8000000L.hex
attiny87at8: $(PROGRAM)_attiny87_8000000L.lst



attiny87at12: TARGET = attiny87
attiny87at12: MCU_TARGET = attiny87
attiny87at12: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' 
attiny87at12: AVR_FREQ ?= 12000000L
attiny87at12: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny87at12: $(PROGRAM)_attiny87_12000000L.hex
attiny87at12: $(PROGRAM)_attiny87_12000000L.lst



#-----------------------
# ATtiny 1634, UART 0 
#-----------------------

attiny1634at12: TARGET = attiny1634
attiny1634at12: MCU_TARGET = attiny1634
attiny1634at12: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny1634at12: AVR_FREQ ?= 12000000L
attiny1634at12: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at12: $(PROGRAM)_attiny1634at12.hex
attiny1634at12: $(PROGRAM)_attiny1634at12.lst



attiny1634at16: TARGET = attiny1634
attiny1634at16: MCU_TARGET = attiny1634
attiny1634at16: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny1634at16: AVR_FREQ ?= 16000000L
attiny1634at16: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at16: $(PROGRAM)_attiny1634at16.hex
attiny1634at16: $(PROGRAM)_attiny1634at16.lst

attiny1634at8: TARGET = attiny1634
attiny1634at8: MCU_TARGET = attiny1634
attiny1634at8: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny1634at8: AVR_FREQ ?= 8000000L
attiny1634at8: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at8: $(PROGRAM)_attiny1634at8.hex
attiny1634at8: $(PROGRAM)_attiny1634at8.lst

attiny1634at8_5v: TARGET = attiny1634
attiny1634at8_5v: MCU_TARGET = attiny1634
attiny1634at8_5v: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny1634at8_5v: AVR_FREQ ?= 8200000L
attiny1634at8_5v: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at8_5v: $(PROGRAM)_attiny1634at8_5v.hex
attiny1634at8_5v: $(PROGRAM)_attiny1634at8_5v.lst

attiny1634at737: TARGET = attiny1634
attiny1634at737: MCU_TARGET = attiny1634
attiny1634at737: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny1634at737: AVR_FREQ ?= 7372800L
attiny1634at737: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at737: $(PROGRAM)_attiny1634at737.hex
attiny1634at737: $(PROGRAM)_attiny1634at737.lst

attiny1634at921: TARGET = attiny1634
attiny1634at921: MCU_TARGET = attiny1634
attiny1634at921: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny1634at921: AVR_FREQ ?= 9216000L
attiny1634at921: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at921: $(PROGRAM)_attiny1634at921.hex
attiny1634at921: $(PROGRAM)_attiny1634at921.lst

attiny1634at110: TARGET = attiny1634
attiny1634at110: MCU_TARGET = attiny1634
attiny1634at110: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  
attiny1634at110: AVR_FREQ ?= 11059200L
attiny1634at110: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at110: $(PROGRAM)_attiny1634at110.hex
attiny1634at110: $(PROGRAM)_attiny1634at110.lst

attiny1634at147: TARGET = attiny1634
attiny1634at147: MCU_TARGET = attiny1634
attiny1634at147: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny1634at147: AVR_FREQ ?= 14745600L
attiny1634at147: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at147: $(PROGRAM)_attiny1634at147.hex
attiny1634at147: $(PROGRAM)_attiny1634at147.lst


#-----------------------
# ATtiny 1634, UART 1 
#-----------------------

attiny1634at12ser1: TARGET = attiny1634
attiny1634at12ser1: MCU_TARGET = attiny1634
attiny1634at12ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny1634at12ser1: AVR_FREQ ?= 12000000L
attiny1634at12ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at12ser1: $(PROGRAM)_attiny1634at12ser1.hex
attiny1634at12ser1: $(PROGRAM)_attiny1634at12ser1.lst

attiny1634at16ser1: TARGET = attiny1634
attiny1634at16ser1: MCU_TARGET = attiny1634
attiny1634at16ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny1634at16ser1: AVR_FREQ ?= 16000000L
attiny1634at16ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at16ser1: $(PROGRAM)_attiny1634at16ser1.hex
attiny1634at16ser1: $(PROGRAM)_attiny1634at16ser1.lst

attiny1634at8ser1: TARGET = attiny1634
attiny1634at8ser1: MCU_TARGET = attiny1634
attiny1634at8ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DUART=1' '-DBAUD_RATE=57600'
attiny1634at8ser1: AVR_FREQ ?= 8000000L
attiny1634at8ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at8ser1: $(PROGRAM)_attiny1634at8ser1.hex
attiny1634at8ser1: $(PROGRAM)_attiny1634at8ser1.lst

attiny1634at737ser1: TARGET = attiny1634
attiny1634at737ser1: MCU_TARGET = attiny1634
attiny1634at737ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DUART=1' '-DBAUD_RATE=57600'
attiny1634at737ser1: AVR_FREQ ?= 7372800L
attiny1634at737ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at737ser1: $(PROGRAM)_attiny1634at737ser1.hex
attiny1634at737ser1: $(PROGRAM)_attiny1634at737ser1.lst

attiny1634at921ser1: TARGET = attiny1634
attiny1634at921ser1: MCU_TARGET = attiny1634
attiny1634at921ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DUART=1' '-DBAUD_RATE=57600'
attiny1634at921ser1: AVR_FREQ ?= 9216000L
attiny1634at921ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at921ser1: $(PROGRAM)_attiny1634at921ser1.hex
attiny1634at921ser1: $(PROGRAM)_attiny1634at921ser1.lst

attiny1634at110ser1: TARGET = attiny1634
attiny1634at110ser1: MCU_TARGET = attiny1634
attiny1634at110ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny1634at110ser1: AVR_FREQ ?= 11059200L
attiny1634at110ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at110ser1: $(PROGRAM)_attiny1634at110ser1.hex
attiny1634at110ser1: $(PROGRAM)_attiny1634at110ser1.lst

attiny1634at147ser1: TARGET = attiny1634
attiny1634at147ser1: MCU_TARGET = attiny1634
attiny1634at147ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny1634at147ser1: AVR_FREQ ?= 14745600L
attiny1634at147ser1: LDSECTIONS  = -Wl,--section-start=.text=0x3d80 -Wl,--section-start=.version=0x3ffe
attiny1634at147ser1: $(PROGRAM)_attiny1634at147ser1.hex
attiny1634at147ser1: $(PROGRAM)_attiny1634at147ser1.lst


#-----------------------
# ATtiny 841, UART 0 
#-----------------------

attiny841at184: TARGET = attiny841
attiny841at184: MCU_TARGET = attiny841
attiny841at184: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at184: AVR_FREQ ?= 18432000L
attiny841at184: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at184: $(PROGRAM)_attiny841at184.hex
attiny841at184: $(PROGRAM)_attiny841at184.lst




attiny841at147: TARGET = attiny841
attiny841at147: MCU_TARGET = attiny841
attiny841at147: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny841at147: AVR_FREQ ?= 14745600L
attiny841at147: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at147: $(PROGRAM)_attiny841at147.hex
attiny841at147: $(PROGRAM)_attiny841at147.lst

attiny841at110: TARGET = attiny841
attiny841at110: MCU_TARGET = attiny841
attiny841at110: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny841at110: AVR_FREQ ?= 11059200L
attiny841at110: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at110: $(PROGRAM)_attiny841at110.hex
attiny841at110: $(PROGRAM)_attiny841at110.lst

attiny841at921: TARGET = attiny841
attiny841at921: MCU_TARGET = attiny841
attiny841at921: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DBAUD_RATE=57600'
attiny841at921: AVR_FREQ ?= 9216000L
attiny841at921: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at921: $(PROGRAM)_attiny841at921.hex
attiny841at921: $(PROGRAM)_attiny841at921.lst

attiny841at737: TARGET = attiny841
attiny841at737: MCU_TARGET = attiny841
attiny841at737: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DBAUD_RATE=57600'
attiny841at737: AVR_FREQ ?= 7372800L
attiny841at737: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at737: $(PROGRAM)_attiny841at737.hex
attiny841at737: $(PROGRAM)_attiny841at737.lst

attiny841at20: TARGET = attiny841
attiny841at20: MCU_TARGET = attiny841
attiny841at20: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny841at20: AVR_FREQ ?= 20000000L
attiny841at20: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at20: $(PROGRAM)_attiny841at20.hex
attiny841at20: $(PROGRAM)_attiny841at20.lst

attiny841at16: TARGET = attiny841
attiny841at16: MCU_TARGET = attiny841
attiny841at16: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny841at16: AVR_FREQ ?= 16000000L
attiny841at16: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at16: $(PROGRAM)_attiny841at16.hex
attiny841at16: $(PROGRAM)_attiny841at16.lst

attiny841at12: TARGET = attiny841
attiny841at12: MCU_TARGET = attiny841
attiny841at12: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'
attiny841at12: AVR_FREQ ?= 12000000L
attiny841at12: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at12: $(PROGRAM)_attiny841at12.hex
attiny841at12: $(PROGRAM)_attiny841at12.lst

attiny841at8: TARGET = attiny841
attiny841at8: MCU_TARGET = attiny841
attiny841at8: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny841at8: AVR_FREQ ?= 8000000L
attiny841at8: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at8: $(PROGRAM)_attiny841at8.hex
attiny841at8: $(PROGRAM)_attiny841at8.lst

attiny841at8_5v: TARGET = attiny841
attiny841at8_5v: MCU_TARGET = attiny841
attiny841at8_5v: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600'
attiny841at8_5v: AVR_FREQ ?= 8200000L
attiny841at8_5v: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at8_5v: $(PROGRAM)_attiny841at8_5v.hex
attiny841at8_5v: $(PROGRAM)_attiny841at8_5v.lst
#-----------------------
# ATtiny 841, UART 1
#-----------------------

attiny841at184ser1: TARGET = attiny841
attiny841at184ser1: MCU_TARGET = attiny841
attiny841at184ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DUART=1'
attiny841at184ser1: AVR_FREQ ?= 18432000L
attiny841at184ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at184ser1: $(PROGRAM)_attiny841at184ser1.hex
attiny841at184ser1: $(PROGRAM)_attiny841at184ser1.lst




attiny841at147ser1: TARGET = attiny841
attiny841at147ser1: MCU_TARGET = attiny841
attiny841at147ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at147ser1: AVR_FREQ ?= 14745600L
attiny841at147ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at147ser1: $(PROGRAM)_attiny841at147ser1.hex
attiny841at147ser1: $(PROGRAM)_attiny841at147ser1.lst

attiny841at110ser1: TARGET = attiny841
attiny841at110ser1: MCU_TARGET = attiny841
attiny841at110ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at110ser1: AVR_FREQ ?= 11059200L
attiny841at110ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at110ser1: $(PROGRAM)_attiny841at110ser1.hex
attiny841at110ser1: $(PROGRAM)_attiny841at110ser1.lst

attiny841at921ser1: TARGET = attiny841
attiny841at921ser1: MCU_TARGET = attiny841
attiny841at921ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DBAUD_RATE=57600' '-DUART=1'
attiny841at921ser1: AVR_FREQ ?= 9216000L
attiny841at921ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at921ser1: $(PROGRAM)_attiny841at921ser1.hex
attiny841at921ser1: $(PROGRAM)_attiny841at921ser1.lst

attiny841at737ser1: TARGET = attiny841
attiny841at737ser1: MCU_TARGET = attiny841
attiny841at737ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DBAUD_RATE=57600' '-DUART=1'
attiny841at737ser1: AVR_FREQ ?= 7372800L
attiny841at737ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at737ser1: $(PROGRAM)_attiny841at737ser1.hex
attiny841at737ser1: $(PROGRAM)_attiny841at737ser1.lst

attiny841at20ser1: TARGET = attiny841
attiny841at20ser1: MCU_TARGET = attiny841
attiny841at20ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at20ser1: AVR_FREQ ?= 20000000L
attiny841at20ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at20ser1: $(PROGRAM)_attiny841at20ser1.hex
attiny841at20ser1: $(PROGRAM)_attiny841at20ser1.lst

attiny841at16ser1: TARGET = attiny841
attiny841at16ser1: MCU_TARGET = attiny841
attiny841at16ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at16ser1: AVR_FREQ ?= 16000000L
attiny841at16ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at16ser1: $(PROGRAM)_attiny841at16ser1.hex
attiny841at16ser1: $(PROGRAM)_attiny841at16ser1.lst

attiny841at12ser1: TARGET = attiny841
attiny841at12ser1: MCU_TARGET = attiny841
attiny841at12ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DUART=1'
attiny841at12ser1: AVR_FREQ ?= 12000000L
attiny841at12ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at12ser1: $(PROGRAM)_attiny841at12ser1.hex
attiny841at12ser1: $(PROGRAM)_attiny841at12ser1.lst

attiny841at8ser1: TARGET = attiny841
attiny841at8ser1: MCU_TARGET = attiny841
attiny841at8ser1: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE'  '-DBAUD_RATE=57600' '-DUART=1'
attiny841at8ser1: AVR_FREQ ?= 8000000L
attiny841at8ser1: LDSECTIONS  = -Wl,--section-start=.text=0x1d80 -Wl,--section-start=.version=0x1ffe
attiny841at8ser1: $(PROGRAM)_attiny841at8ser1.hex
attiny841at8ser1: $(PROGRAM)_attiny841at8ser1.lst

#-----------------------
# ATtiny 841, UART 0, no LED
# There's no noLED definitions for 1634 because the larger page size
# on the 1634 means that you don't gain any usable flash from it. 
#-----------------------

attiny841at20noLED: TARGET = attiny841
attiny841at20noLED: MCU_TARGET = attiny841
attiny841at20noLED: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DLED_START_FLASHES=0'
attiny841at20noLED: AVR_FREQ ?= 20000000L
attiny841at20noLED: LDSECTIONS  = -Wl,--section-start=.text=0x1dc0 -Wl,--section-start=.version=0x1ffe
attiny841at20noLED: $(PROGRAM)_attiny841at20noLED.hex
attiny841at20noLED: $(PROGRAM)_attiny841at20noLED.lst

attiny841at16noLED: TARGET = attiny841
attiny841at16noLED: MCU_TARGET = attiny841
attiny841at16noLED: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DLED_START_FLASHES=0'
attiny841at16noLED: AVR_FREQ ?= 16000000L
attiny841at16noLED: LDSECTIONS  = -Wl,--section-start=.text=0x1dc0 -Wl,--section-start=.version=0x1ffe
attiny841at16noLED: $(PROGRAM)_attiny841at16noLED.hex
attiny841at16noLED: $(PROGRAM)_attiny841at16noLED.lst

attiny841at8noLED: TARGET = attiny841
attiny841at8noLED: MCU_TARGET = attiny841
attiny841at8noLED: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION' '-DFOURPAGEERASE' '-DLED_START_FLASHES=0'  '-DBAUD_RATE=57600'
attiny841at8noLED: AVR_FREQ ?= 8000000L
attiny841at8noLED: LDSECTIONS  = -Wl,--section-start=.text=0x1dc0 -Wl,--section-start=.version=0x1ffe 
attiny841at8noLED: $(PROGRAM)_attiny841at8noLED.hex
attiny841at8noLED: $(PROGRAM)_attiny841at8noLED.lst

#--------------------
# ATtiny828 - only one speed since it doesn't support crystal. 
# If you want to use an external clock 
#--------------------


attiny828at8: TARGET = attiny828
attiny828at8: MCU_TARGET = attiny828
attiny828at8: CFLAGS += $(COMMON_OPTIONS) '-DBAUD_RATE=57600'
attiny828at8: AVR_FREQ ?= 8000000L
attiny828at8: LDSECTIONS  = -Wl,--section-start=.text=0x1E00 -Wl,--section-start=.version=0x1ffe
attiny828at8: $(PROGRAM)_attiny828at8.hex
attiny828at8: $(PROGRAM)_attiny828at8.lst


attiny828at8_5v: TARGET = attiny828
attiny828at8_5v: MCU_TARGET = attiny828
attiny828at8_5v: CFLAGS += $(COMMON_OPTIONS) '-DBAUD_RATE=57600'
attiny828at8_5v: AVR_FREQ ?= 8200000L
attiny828at8_5v: LDSECTIONS  = -Wl,--section-start=.text=0x1E00 -Wl,--section-start=.version=0x1ffe
attiny828at8_5v: $(PROGRAM)_attiny828at8_5v.hex
attiny828at8_5v: $(PROGRAM)_attiny828at8_5v.lst
# Test platforms
# Virtual boot block test
virboot328: TARGET = atmega328
virboot328: MCU_TARGET = atmega328p
virboot328: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT_PARTITION'
virboot328: AVR_FREQ ?= 16000000L
virboot328: LDSECTIONS  = -Wl,--section-start=.text=0x7d80 -Wl,--section-start=.version=0x7ffe
virboot328: $(PROGRAM)_atmega328_virboot.hex
virboot328: $(PROGRAM)_atmega328_virboot.lst


#
# Include additional platforms
include Makefile.extras
include Makefile.1284
include Makefile.custom


#---------------------------------------------------------------------------
#
# Generic build instructions
#

FORCE:

baudcheck: FORCE
	- @$(CC) --version
	- @$(CC) $(CFLAGS) -E baudcheck.c -o baudcheck.tmp.sh
	- @$(SH) baudcheck.tmp.sh

isp: $(TARGET)
	$(MAKE) -f Makefile.isp isp TARGET=$(TARGET)

isp-stk500: $(PROGRAM)_$(TARGET).hex
	$(STK500-1)
	$(STK500-2)

%.elf: $(OBJ) baudcheck $(dummy)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(SIZE) $@

clean:
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex *.tmp.sh

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
