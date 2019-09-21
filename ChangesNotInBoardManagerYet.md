### This document lists significant changes and bugfixes that are not yet in a release available via board manager. It is used to compose the list of new features that accompanies a release.

* Serial hogged an unnecessary amount of ram on the 841/441/1634. Pulled in the latest version of HardwareSerial from the official AVR core; this sorts out that issue.
