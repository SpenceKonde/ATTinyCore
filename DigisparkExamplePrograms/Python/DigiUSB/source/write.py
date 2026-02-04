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


import sys, time
sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice


if __name__ == "__main__":
    try:
        theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)

        print "Found: 0x%04x 0x%04x %s %s" % (theDevice.idVendor, 
                                              theDevice.idProduct,
                                              theDevice.productName,
                                              theDevice.manufacturer)
    except:
        pass

    while 1 == 1:
        user_input = raw_input("DigiUSB#")
        user_input = user_input+"\n"
        theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
        for c in user_input:
            theDevice.write(ord(c))
            time.sleep(0.1)

            
       
        
