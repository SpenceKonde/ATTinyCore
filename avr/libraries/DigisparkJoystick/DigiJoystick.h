/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 * Modified for Digispark by Digistump
 * And now modified by Sean Murphy (duckythescientist) from a keyboard device to a joystick device
 * And now modified by Bluebie to have better code style, not ruin system timers, and have delay() function
 * Most of the credit for the joystick code should go to Raphaël Assénat
 */
#ifndef __DigiJoystick_h__
#define __DigiJoystick_h__
 
#define GCN64_REPORT_SIZE 8

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "usbdrv.h"
//#include "devdesc.h"
#include "oddebug.h"
#include "usbconfig.h"
 
const static uchar *rt_usbHidReportDescriptor=NULL;
static uchar rt_usbHidReportDescriptorSize=0;
const static uchar *rt_usbDeviceDescriptor=NULL;
static uchar rt_usbDeviceDescriptorSize=0;

// TODO: Work around Arduino 12 issues better.
//#include <WConstants.h>
//#undef int()

//typedef uint8_t byte;

/* What was most recently read from the controller */
unsigned char last_built_report[GCN64_REPORT_SIZE];

/* What was most recently sent to the host */
unsigned char last_sent_report[GCN64_REPORT_SIZE];

uchar		 reportBuffer[8];

// report frequency set to default of 50hz
#define DIGIJOYSTICK_DEFAULT_REPORT_INTERVAL 20
static unsigned char must_report = 0;
static unsigned char idle_rate = DIGIJOYSTICK_DEFAULT_REPORT_INTERVAL / 4; // in units of 4ms
// new minimum report frequency system:
static unsigned long last_report_time = 0;


const unsigned char gcn64_usbHidReportDescriptor[] PROGMEM = {
		0x05, 0x01,										 // USAGE_PAGE (Generic Desktop)
		0x09, 0x05,										 // USAGE (Gamepad)
		0xa1, 0x01,										 // COLLECTION (Application)
	
		0x09, 0x01,										 //		USAGE (Pointer)		 
	0xa1, 0x00,										 //		COLLECTION (Physical)
	0x05, 0x01,										 //			USAGE_PAGE (Generic Desktop)
		0x09, 0x30,										 //			USAGE (X)
		0x09, 0x31,										 //			USAGE (Y)
	
	0x09, 0x33,						 //			USAGE (Rx)
	0x09, 0x34,						//		USAGE (Ry)

	0x09, 0x35,						//		USAGE (Rz)	
	0x09, 0x36,						//		USAGE (Slider)	

		0x15, 0x00,										 //			LOGICAL_MINIMUM (0)
		0x26, 0xFF, 0x00,							 //			LOGICAL_MAXIMUM (255)
		0x75, 0x08,										 //			REPORT_SIZE (8)
		0x95, 0x06,										 //			REPORT_COUNT (6)
		0x81, 0x02,										 //			INPUT (Data,Var,Abs)
	0xc0,													 //		END_COLLECTION (Physical)

		0x05, 0x09,										 //		USAGE_PAGE (Button)
		0x19, 0x01,										 //		USAGE_MINIMUM (Button 1)
		0x29, 0x10,										 //		USAGE_MAXIMUM (Button 14)
		0x15, 0x00,										 //		LOGICAL_MINIMUM (0)
		0x25, 0x01,										 //		LOGICAL_MAXIMUM (1)
		0x75, 0x01,										 //		REPORT_SIZE (1)
		0x95, 0x10,										 //		REPORT_COUNT (16)
		0x81, 0x02,										 //		INPUT (Data,Var,Abs)

		0xc0													 // END_COLLECTION (Application)
};

#define USBDESCR_DEVICE					1

const unsigned char usbDescrDevice[] PROGMEM = {		/* USB device descriptor */
		18,					/* sizeof(usbDescrDevice): length of descriptor in bytes */
		USBDESCR_DEVICE,		/* descriptor type */
		0x01, 0x01, /* USB version supported */
		USB_CFG_DEVICE_CLASS,
		USB_CFG_DEVICE_SUBCLASS,
		0,					/* protocol */
		8,					/* max packet size */
		USB_CFG_VENDOR_ID,	/* 2 bytes */
		USB_CFG_DEVICE_ID,	/* 2 bytes */
		USB_CFG_DEVICE_VERSION, /* 2 bytes */
#if USB_CFG_VENDOR_NAME_LEN
		1,					/* manufacturer string index */
#else
		0,					/* manufacturer string index */
#endif
#if USB_CFG_DEVICE_NAME_LEN
		2,					/* product string index */
#else
		0,					/* product string index */
#endif
#if USB_CFG_SERIAL_NUMBER_LENGTH
		3,					/* serial number string index */
#else
		0,					/* serial number string index */
#endif
		1,					/* number of configurations */
};



void gamecubeBuildReport(unsigned char *reportBuf) {
	if (reportBuf != NULL) {
		memcpy(reportBuf, last_built_report, GCN64_REPORT_SIZE);
	}
	
	memcpy(last_sent_report, last_built_report, GCN64_REPORT_SIZE); 
}

int getGamepadReport(unsigned char *dstbuf) {
	gamecubeBuildReport(dstbuf);
	return GCN64_REPORT_SIZE;
}

 
class DigiJoystickDevice {
 public:
	DigiJoystickDevice () {
		cli();
		usbDeviceDisconnect();
		_delay_ms(250);
		usbDeviceConnect();
	
		rt_usbHidReportDescriptor = gcn64_usbHidReportDescriptor;
		rt_usbHidReportDescriptorSize = sizeof(gcn64_usbHidReportDescriptor);
		rt_usbDeviceDescriptor = usbDescrDevice;
		rt_usbDeviceDescriptorSize = sizeof(usbDescrDevice);
		
		usbInit();
		
		sei();
		
		last_report_time = millis();
	}
	
	void update() {
		usbPoll();
		
		// instead of above code, use millis arduino system to enforce minimum reporting frequency
		unsigned long time_since_last_report = millis() - last_report_time;
		if (time_since_last_report >= (idle_rate * 4 /* in units of 4ms - usb spec stuff */)) {
			last_report_time += idle_rate * 4;
			must_report = 1;
		}
		
		// if the report has changed, try force an update anyway
		if (memcmp(last_built_report, last_sent_report, GCN64_REPORT_SIZE)) {
			must_report = 1;
		}
	
		// if we want to send a report, signal the host computer to ask us for it with a usb 'interrupt'
		if (must_report) {
			if (usbInterruptIsReady()) {
				must_report = 0;
				
				gamecubeBuildReport(reportBuffer);
				usbSetInterrupt(reportBuffer, GCN64_REPORT_SIZE);
			}
		}
	}
	
	// delay while updating until we are finished delaying
	void delay(long milli) {
		unsigned long last = millis();
	  while (milli > 0) {
	    unsigned long now = millis();
	    milli -= now - last;
	    last = now;
	    update();
	  }
	}
	
	void setX(byte value) {
		last_built_report[0] = value;
	}
	
	void setY(byte value) {
		last_built_report[1] = value;
	}
	
	void setXROT(byte value) {
		last_built_report[2] = value;
	}
	
	void setYROT(byte value) {
		last_built_report[3] = value;
	}
	
	void setZROT(byte value) {
		last_built_report[4] = value;
	}
	
	void setSLIDER(byte value) {
		last_built_report[5] = value;
	}
	
	void setX(char value) {
		setX(*(reinterpret_cast<byte *>(&value)));
	}
	
	void setY(char value) {
		setY(*(reinterpret_cast<byte *>(&value)));
	}
	
	void setXROT(char value) {
		setXROT(*(reinterpret_cast<byte *>(&value)));
	}
	
	void setYROT(char value) {
		setYROT(*(reinterpret_cast<byte *>(&value)));
	}
	
	void setZROT(char value) {
		setZROT(*(reinterpret_cast<byte *>(&value)));
	}
	void setSLIDER(char value) {
		setSLIDER(*(reinterpret_cast<byte *>(&value)));
	}
	
	void setButtons(unsigned char low, unsigned char high) {
		last_built_report[6] = low;
		last_built_report[7] = high;
	}
	
	void setButtons(char low,char high) {
		setButtons(*reinterpret_cast<unsigned char *>(&low),*reinterpret_cast<unsigned char *>(&high));
	}
	
	void setValues(unsigned char values[]) {
		memcpy(last_built_report, values, GCN64_REPORT_SIZE);
	}
	
	void setValues(char values[]) {
		unsigned char *foo = reinterpret_cast<unsigned char *>(values);//preserves bit values in cast
		memcpy(last_built_report, foo, GCN64_REPORT_SIZE);
	}
};

// Create global singleton object for users to make use of
DigiJoystickDevice DigiJoystick = DigiJoystickDevice();





#ifdef __cplusplus
extern "C"{
#endif 
	// USB_PUBLIC uchar usbFunctionSetup
	
	uchar usbFunctionSetup(uchar data[8]) {
		usbRequest_t *rq = (usbRequest_t *)data;

		usbMsgPtr = reportBuffer;
		if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) { // class request type
			if (rq->bRequest == USBRQ_HID_GET_REPORT){ // wValue: ReportType (highbyte), ReportID (lowbyte)
				// we only have one report type, so don't look at wValue
				//curGamepad->buildReport(reportBuffer);
				//return curGamepad->report_size;
				return GCN64_REPORT_SIZE;
			} else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
				usbMsgPtr = &idle_rate;
				return 1;
			} else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
				idle_rate = rq->wValue.bytes[1];
			}
		} else {
			/* no vendor specific requests implemented */
		}
		return 0;
	}

	uchar usbFunctionDescriptor(struct usbRequest *rq) {
		if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_STANDARD) {
			return 0;
		}

		if (rq->bRequest == USBRQ_GET_DESCRIPTOR) {
			// USB spec 9.4.3, high byte is descriptor type
			switch (rq->wValue.bytes[1]) {
				case USBDESCR_DEVICE:
					usbMsgPtr = rt_usbDeviceDescriptor;
					return rt_usbDeviceDescriptorSize;
					break;
					
				case USBDESCR_HID_REPORT:
					usbMsgPtr = rt_usbHidReportDescriptor;
					return rt_usbHidReportDescriptorSize;
					break;
					
			}
		}
		
		return 0;
	}
	
#ifdef __cplusplus
} // extern "C"
#endif


#endif // __DigiKeyboard_h__
