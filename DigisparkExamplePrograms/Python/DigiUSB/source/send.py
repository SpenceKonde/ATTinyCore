#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#


import usb, sys # 1.0 not 0.4
sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice


if __name__ == "__main__":
   try:
      theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
   except:
      sys.exit("No DigiUSB Device Found")



   try:
     user_input = sys.argv[1]+"\n"
   except:
     exit("No data to send")
        
   for c in user_input:
     theDevice.write(ord(c))
