### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release. 

* Correct missing bootloader for Tiny828. 
* Universal Servo library now supports Tinyx5 and Tinyx61. This should cover everything people want to drive servos with. 
* Added missing #defines like NUM_DIGITAL_PINS from some parts. 
* Improved part-specific documentation. 
* Support for ATTiny43, which has builtin boost converter and can run off of a single alkaline battery
* Fix delay at extremely low clock speeds (under 1MHz)
* Pinout image corrections
* Master Only / Slave Only / Both menu for x41 and 828 to try to save space with I2C (pending)
* Removed two unused I2C related files
* Added support for new clockwise pinout for x41 series and menu options for it. 
* Output assembler listing
