This is the Readme file for the libs-device directory. This directory contains
code snippets which may be useful for USB device firmware.


WHAT IS INCLUDED IN THIS DIRECTORY?
===================================

osccal.c and osccal.h
  This module contains a function which calibrates the AVR's built-in RC
  oscillator based on the USB frame clock. See osccal.h for a documentation
  of the API.

osctune.h
  This header file contains a code snippet for usbconfig.h. With this code,
  you can keep the AVR's internal RC oscillator in sync with the USB frame
  clock. This is a continuous synchronization, not a single calibration at
  USB reset as with osccal.c above. Please note that this code works only
  if D- is wired to the interrupt, not D+.

----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
