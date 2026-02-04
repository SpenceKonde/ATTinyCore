#!/usr/bin/python

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

    theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)

    print "Found: 0x%04x 0x%04x %s %s" % (theDevice.idVendor, 
                                          theDevice.idProduct,
                                          theDevice.productName,
                                          theDevice.manufacturer)




    import sys
    import time
    import webcolors


    #sequence = [11,12,13]* 20
    #random.shuffle(sequence)

    #print "Look over there, flashing lights!"

    if len(sys.argv)<3:

        color_list = webcolors.name_to_rgb(sys.argv[1].lower())
        color_list = list(color_list)
        color_list.insert(0, 0)
    else:
        color_list = sys.argv




   # while 1:
        #pin = int(pin)



        #if output == "\r":
        #    print line
        #    line =""
        #else:
        #    line += output
        #else:
    
        #    print "Pin response didn't match."

    #byte val = sys.argv[1]
    print color_list

    theDevice.write(ord("s"))

    if color_list[1] == 0:
        theDevice.write(0)
    else:
        theDevice.write(int(color_list[1]))

    if color_list[2] == 0:
        theDevice.write(0)
    else:
        theDevice.write(int(color_list[2]))

    if color_list[3] == 0:
        theDevice.write(0)
    else:
        theDevice.write(int(color_list[3]))

    #theDevice.write(ord("e"))
        #time.sleep(2)

    #print
    
                                

