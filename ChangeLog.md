### This document lists significant changes and bugfixes, including those not in yet in a release.
1.3.4
* Fix pulseIn(), both pulse length, and timeout (the latter is broken in official avr core too) (#384)
* Add pulseInLong()
* (untested) Don't generate .lst unless told to 'export compiled binary' (#379 - though note that that issue also appears to involve a problem with their compiler, or maybe just an extremely perverse username that's breaking things)

1.3.3
* Add support for external CLOCK on 48/88/828. Document how to use external clock on other parts via manual AVRdude step (#355).
* Fix bug with using 32K ULP as clock source on 828.
* A bit of boards.txt cleanup.
* Serial hogged an unnecessary amount of ram on the 841/441/1634. Pulled in the latest version of HardwareSerial from the official AVR core; this sorts out that issue.
* Add printf support to printable class.
* Add CLOCK_SOURCE define to identify clock source (#358)
* Add support for 4MHz internal on all boards (#358)
* Add support for 16.5MHz on x5 and x61 (#349)
* Documentation improvements (#373, #375)
* Burn Bootloader now executes as a single command; this should fix issues with some programmers. (#372)

Prior to 1.3.3, a proper changelog was not kept. See the releases for information about changes introduced during that timeframe.
