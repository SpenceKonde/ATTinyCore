### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release.

* Correct compilation error when using 14.7456MHz and 18.432MHz crystal (which has always been there, which says something about how many people are using these clock options)
* Improve delayMicroseconds() accuracy for 18.432MHz crystal.
* Correct issues with some of the macros for t43.
* Add missing attiny85 9.26MHz external clock menu option.
