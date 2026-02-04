#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#
# Includes functionality to retrieve string descriptors
#
# Author: follower@rancidbacon.com
#
# Version: 20091022
#

import usb # 1.0 not 0.4


def getStringDescriptor(device, index):
    """
    """
    response = device.ctrl_transfer(usb.util.ENDPOINT_IN,
                                    usb.legacy.REQ_GET_DESCRIPTOR,
                                    (usb.util.DESC_TYPE_STRING << 8) | index,
                                    0, # language id
                                    255) # length

    # TODO: Refer to 'libusb_get_string_descriptor_ascii' for error handling
    
    return response[2:].tostring().decode('utf-16')


REQUEST_TYPE_SEND = usb.util.build_request_type(usb.util.CTRL_OUT,
                                                usb.util.CTRL_TYPE_CLASS,
                                                usb.util.CTRL_RECIPIENT_DEVICE)

REQUEST_TYPE_RECEIVE = usb.util.build_request_type(usb.util.CTRL_IN,
                                                usb.util.CTRL_TYPE_CLASS,
                                                usb.util.CTRL_RECIPIENT_DEVICE)

USBRQ_HID_GET_REPORT = 0x01
USBRQ_HID_SET_REPORT = 0x09
USB_HID_REPORT_TYPE_FEATURE = 0x03


class ArduinoUsbDevice(object):
    """
    """
    
    def __init__(self, idVendor, idProduct):
        """
        """
        self.idVendor = idVendor
        self.idProduct = idProduct

        # TODO: Make more compliant by checking serial number also.
        self.device = usb.core.find(idVendor=self.idVendor,
                                    idProduct=self.idProduct)

        if not self.device:
            raise Exception("Device not found")


    def write(self, byte):
        """
        """
        # TODO: Return bytes written?
        #print "Write:"+str(byte)
        self._transfer(REQUEST_TYPE_SEND, USBRQ_HID_SET_REPORT,
                       byte,
                       []) # ignored


    def read(self):
        """
        """
        response = self._transfer(REQUEST_TYPE_RECEIVE, USBRQ_HID_GET_REPORT,
                              0, # ignored
                              1) # length

        if not response:
            raise Exception("No Data")
        
        return response[0]
    

    def _transfer(self, request_type, request, index, value):
        """
        """
        return self.device.ctrl_transfer(request_type, request,
                                        (USB_HID_REPORT_TYPE_FEATURE << 8) | 0,
                                         index,
                                         value)


    @property
    def productName(self):
        """
        """
        return getStringDescriptor(self.device, self.device.iProduct)

    
    @property
    def manufacturer(self):
        """
        """
        return getStringDescriptor(self.device, self.device.iManufacturer)


    
                                

