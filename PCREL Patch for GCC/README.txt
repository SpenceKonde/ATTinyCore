This patch replaces the ld.exe file (or just ld for Mac) in WinAVR/Crosspack supplied with the Arduino IDE. The supplied version has a bug which causes weird errors when files larger than 4kB are compiled for AVR25 processors which include those in the Tiny series.

This Bug was fixed in a later version of the linker, and thankfully it is a simple case of replacing one file to fix the Arduino version.

How to Apply:

FOR MAC OSX (Works on Snow Leopard, Lion, Mountain Lion)
---------------------------------------------------------
The following is how to apply the fix for MacOSX. This is based of AVR crosspack 20090319.

For Mac you only need the single extensionless file "ld" which is included in this folder, or can be aquired from the downloads page under then named "MAC ld.zip".

Right-Click on Arduino.app
Click "Show Package Contents"
Due to MacOSx inability to merge folders, you will have to navigate to the correct folder yourself:
/Contents/Resources/Java/hardware/tools/avr/avr/bin/

Copy and replace "ld" with the version from this folder or the aforementioned the zip archive.

Restart the IDE and that is all.

FOR WINDOWS USERS (Works on all version starting with XP)
---------------------------------------------------------
The following is how to apply the fix for Windows. The issue was fixed as of WinAVR 20090313

For windows, the ld.exe file needs replacing. It is buried deep within a chain of folders, so I have built the directory structure for you. You can either navigate through the folders starting with 'hardware' (following the structure in this repository) and replace the ld.exe file with the one from this git repo, or you can just go to the downloads page and download "WINDOWS ld.zip".

If you extract the zip file, you will find a folder called 'hardware'. Just copy this into the <arduino> directory and when Windows asks, merge existing folders, and replace the one file - by creating the directory structure for you the file will be copied into the correct place.

Restart the IDE and that is all.