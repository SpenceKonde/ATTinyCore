### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release.

* Correct compilation error when using 14.7456MHz and 18.432MHz crystal (which has always been there, which says something about how many people are using these clock options) (#290)
* Improve delayMicroseconds() accuracy for 18.432MHz crystal. (#292)
* Fix issue with macros in pins_arduino.h for t1634 that could cause problems with some compiler (well, io.h) versions
* Correct issues with some of the macros for t43. (#298)
* Add missing attiny85 9.26MHz external clock menu option. (#296)
* Fix PWM on pins A3 and A6 on Tiny841/441 (#302)
* Correct hardware specs for Tiny24/44/84 and Tiny1634 (#304/305)
* On the attiny25/45/85, tone() will now use hardware output compare for pin 1 and 4, not just pin 1. (#289)
* Add missing low frequency internal (128kHz WDT or 32kHz ULP) for all parts where this wasn't already an option (#310)
* Various documentation corrections and clarifications (#307 and others)
* Remove U2X exception for 57600 baud (#309)
* Give #error when compiling attiny841/441 on versions of the IDE where the cached core filename would exceed MAX_PATH under common conditions on windows.
* Add 1MHz optiboot bootloaders due to persistent requests for this feature.
