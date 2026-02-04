#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#


import usb # 1.0 not 0.4


import sys
sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice


if __name__ == "__main__":
    try:
        theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
    except:
        sys.exit("No DigiUSB Device Found")




    import sys
    import time

    while 1 == 1:
        try:
            lastChar = chr(theDevice.read())
            if(lastChar == "\n"):
                break
            sys.stdout.write(lastChar)
            sys.stdout.flush()

            
        except:
            # TODO: Check for exception properly
            time.sleep(0.1)
