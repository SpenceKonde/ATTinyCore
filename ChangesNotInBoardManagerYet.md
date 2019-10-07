### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release.
* Add support for external CLOCK on 48/88/828. Document how to use external clock on other parts via manual AVRdude step.
* Fix bug with using 32K ULP as clock source on 828.
* A bit of boards.txt cleanup.
* Serial hogged an unnecessary amount of ram on the 841/441/1634. Pulled in the latest version of HardwareSerial from the official AVR core; this sorts out that issue.
