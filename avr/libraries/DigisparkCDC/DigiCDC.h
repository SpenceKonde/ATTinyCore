/*

CDC Arduino Library by Ihsan Kehribar (kehribar.me) 
and Digistump LLC (digistump.com) 
- all changes made under the same license as V-USB


 */
#ifndef __DigiCDC_h__
#define __DigiCDC_h__
#include "usbdrv.h"



#include "Stream.h"
#include "ringBuffer.h"


#define HW_CDC_TX_BUF_SIZE     32
#define HW_CDC_RX_BUF_SIZE     32
#define HW_CDC_BULK_OUT_SIZE     8
#define HW_CDC_BULK_IN_SIZE      8



/* library functions and variables start */
static uint8_t tmp[HW_CDC_BULK_IN_SIZE];
static uint8_t index = 0;

static RingBuffer_t rxBuf;
static uint8_t      rxBuf_Data[HW_CDC_RX_BUF_SIZE];

static RingBuffer_t txBuf;
static uint8_t      txBuf_Data[HW_CDC_TX_BUF_SIZE];


class DigiCDCDevice  : public Stream {
    public:
        DigiCDCDevice();
        void begin(), begin(unsigned long x);
        void end();
        void refresh();
        void task();
        void delay(long milli);
        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        virtual size_t write(uint8_t);
        using Print::write;
        operator bool();
    private:
        void usbBegin();
        void usbPollWrapper();
 };


extern DigiCDCDevice SerialUSB;


#endif // __DigiCDC_h__
