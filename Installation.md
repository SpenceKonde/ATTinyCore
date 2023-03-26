# Supported IDE versions

ATtinyCore (Universal) works with all versions of the official IDE (from arduino.cc ) from version 1.6.11 onwards (we have dropped support for versions prior to the fixes in 1.6.11 in 2.0.0), and it is known that manual installation would never work on older versions because they don't come with a toolchain that supports LTO, and the process of manually updating the toolchain is non-trivial. Plus - those versions are old as the hills now). Version 1.8.13 or newer is recommended. Version 1.9.x versions mostly did not work, however.  Version 2.0.4 might work, 2.0.3 does not.

Note also that version 1.7.x versions are from the "other" arduino when arduino.cc and arduino.org got into a some sort of dustup, and arduino.org forked the IDE and increased the minor version to make theirs look newer. No 1.7.x version is supported.

## Boards Manager Installation

This core can be installed using the boards manager. The boards manager URL is:

`http://drazzy.com/package_drazzy.com_index.json`

1. File->Preferences on a PC, or Arduino->Preferences on a Mac, enter the above URL in "Additional Boards Manager URLs
2. Tools -> Boards -> Boards Manager...
3. Select "ATTinyCore by Spence Konde" and click "Install".

## Manual Installation

Option 1: Download the .zip, extract, and place in the hardware folder inside your sketchbook folder (if there is no hardware folder, create it). You can find/set the location of the sketchbook folder in the Arduino IDE at File > Preferences -> Sketchbook location.

Option 2: Download the github client, and sync this repo to the hardware subfolder of your sketchbook folder.


![core installation](http://drazzy.com/e/img/coreinstall.jpg "You want it to look like this")
