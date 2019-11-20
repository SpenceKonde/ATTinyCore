### This document lists significant changes and bugfixes, including those not in yet in a release.

1.3.3
* Add support for external CLOCK on 48/88/828. Document how to use external clock on other parts via manual AVRdude step.
* Fix bug with using 32K ULP as clock source on 828.
* A bit of boards.txt cleanup.
* Serial hogged an unnecessary amount of ram on the 841/441/1634. Pulled in the latest version of HardwareSerial from the official AVR core; this sorts out that issue.
* Add printf support to printable class.
* Add CLOCK_SOURCE define to identify clock source

Prior to 1.3.3, a proper changelog was not kept. See the releases for information about changes introduced during that timeframe.
