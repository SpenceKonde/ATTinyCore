#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#
# Includes functionality to retrieve string descriptors
#
# Author: follower@rancidbacon.com
#
# Version: 20091021
#

#
# Assumes 'UsbStreamDemo1.pde' is loaded on Arduino and 
# LEDs are present on pins 11, 12 and 13.
#

import usb # 1.0 not 0.4


import sys
sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice


if __name__ == "__main__":
    try:
        theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)

        print("Found: 0x%04x 0x%04x %s %s" % (theDevice.idVendor, 
                                              theDevice.idProduct,
                                              theDevice.productName,
                                              theDevice.manufacturer))
    except:
        pass




    import sys
    import time

    while 1 == 1:
        try:
            theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
            try:
                sys.stdout.write(chr(theDevice.read()))
                sys.stdout.flush()
            except:
                # TODO: Check for exception properly
                time.sleep(0.5)
                
        except:
            # TODO: Check for exception properly
            time.sleep(1)
            
       
        
