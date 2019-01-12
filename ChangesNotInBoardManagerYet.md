### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release. 

1.2.3:
* Fix problem with macro for finding PCINT bits with new pinout of ATTiny841 (this would among other things cause SoftwareSerial library to fail to compile) (#279)
* Fixed Wire.end() not turning off pullup resistors on USI devices (#278)
* Added defines for DDAn on x313 - the io.h defines for these were misspelled (didn't match other DDRx registers or datasheet) (#280)
* Remove entry from avrdude.conf that would cause some versions of avrdude to reject the configuration file (found & fixed during internal testing)
* Fix issue with DI/DO pin defines on tiny x61 series (#282)
