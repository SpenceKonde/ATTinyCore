
#ifndef _CYWM6935_H_
#define _CYWM6935_H_

	#if ARDUINO >= 100
	  #include "Arduino.h"
	#else
	  #include "WProgram.h"
	#endif

    #include "TinySoftwareSPI.h"
    #include "Stream.h"

    typedef enum {
        REG_ID               = 0x00,
        REG_CONTROL          = 0x03,
        REG_DATA_RATE        = 0x04,
        REG_CONFIG           = 0x05,
        REG_SERDES_CTL       = 0x06,
        REG_RX_INT_EN        = 0x07,
        REG_RX_INT_STAT      = 0x08,
        REG_RX_DATA_A        = 0x09,
        REG_RX_VALID_A       = 0x0A,
        REG_RX_DATA_B        = 0x0B,
        REG_RX_VALID_B       = 0x0C,
        REG_TX_INT_EN        = 0x0D,
        REG_TX_INT_STAT      = 0x0E,
        REG_TX_DATA          = 0x0F,
        REG_TX_VALID         = 0x10,
        REG_THRESHOLD_L      = 0x19,
        REG_THRESHOLD_H      = 0x1A,
        REG_WAKE_EN          = 0x1C,
        REG_WAKE_STAT        = 0x1D,
        REG_ANALOG_CTL       = 0x20,
        REG_CHANNEL          = 0x21,
        REG_RSSI             = 0x22,
        REG_PA               = 0x23,
        REG_CRYSTAL_ADJ      = 0x24,
        REG_VCO_CAL          = 0x26,
        REG_PWR_CTL          = 0x2E,
        REG_CARRIER_DETECT   = 0x2F,
        REG_CLOCK_MANUAL     = 0x32,
        REG_CLOCK_ENABLE     = 0x33,
        REG_SYN_LOCK_CNT     = 0x38
    } CYWM6935Address;

    typedef enum {
        REG_PN_CODE          = 0x11,
        REG_MID              = 0x3C
    } CYWM6935ExtendedAddress;

    typedef enum {
        Read = 0,
        Write = 1
    } ReadWrite_t;

    typedef enum {
        Disabled = 0,
        Enabled = 1
    } Enabled_t;

    typedef enum {
        DeviceReset = 1,
        NoReset = 0
    } Reset_t;

    typedef enum {
        MSB32 = 1,
        LSB32 = 0
    } PNCode_t;

    typedef enum {
        CodeWidth32bit = 1,
        CodeWidth64bit = 0
    } CodeWidth_t;

    typedef enum {
        NormalDataRate = 0,
        DoubleDataRate = 1
    } DataRate_t;

    typedef enum {
        Oversampling6x = 0,
        Oversampling12x = 1
    } SampleRate_t;

    typedef enum {
        CMOSInverted = 0b00,
        CMOS = 0b01,
        OpenDrain = 0b10,
        OpenSource = 0b11
    } IRQPin_t;

    typedef enum {
        AmpAuto = 0b00,
        AmpEnabled = 0b11,
        AmpDisabled = 0b10
    } PAControl_t;

    typedef enum {
        MaxAdjust = 0b11,
        MinAdjust = 0b10,
        NoAdjust = 0b00
    } VCOAdjust_t;

    typedef union CYWM6935RegisterData_t{
        byte __private_data;
        struct {
            byte productID:4;
            byte siliconID:4;
        } REG_ID;
        struct {
            unsigned:2;
            PAControl_t interalPA:2;
            Enabled_t synLockBypass:1;
            PNCode_t noiseCodeSelect:1;
            Enabled_t transmitter:1;
            Enabled_t reciever:1;
        } REG_CONTROL;
        struct {
            SampleRate_t sampleRate:1;
            DataRate_t dataRate:1;
            CodeWidth_t codeWidth:1;
            unsigned:5;
        } REG_DATA_RATE;
        struct {
            IRQPin_t pinSelect:2;
            unsigned:6;
        } REG_CONFIG;
        struct {
            byte EOFLength:3;
            Enabled_t SERDES:1;
            unsigned:4;
        } REG_SERDES_CTL;
        struct {
            Enabled_t fullA:1;
            Enabled_t EOFA:1;
            Enabled_t overflowA:1;
            Enabled_t underflowA:1;
            Enabled_t fullB:1;
            Enabled_t EOFB:1;
            Enabled_t overflowB:1;
            Enabled_t underflowB:1;
        } REG_RX_INT_EN;
        struct {
            boolean fullA:1;
            boolean EOFA:1;
            boolean flowViolationA:1;
            boolean validA:1;
            boolean fullB:1;
            boolean EOFB:1;
            boolean flowViolationB:1;
            boolean validB:1;
        } REG_RX_INT_STAT;
        byte REG_RX_DATA;
        byte REG_RX_VALID;
        byte REG_RX_DATA_A;
        byte REG_RX_VALID_A;
        byte REG_RX_DATA_B;
        byte REG_RX_VALID_B;
        struct {
            Enabled_t empty:1;
            Enabled_t done:1;
            Enabled_t overflow:1;
            Enabled_t underflow:1;
            unsigned:4;
        } REG_TX_INT_EN;
        struct {
            boolean empty:1;
            boolean done:1;
            boolean overflow:1;
            boolean underflow:1;
            unsigned:4;
        } REG_TX_INT_STAT;
        byte REG_TX_DATA;
        byte REG_TX_VALID;
        struct {
            byte low:7;
            unsigned:1;
        } REG_THRESHOLD_L;
        struct {
            byte high:7;
            unsigned:1;
        } REG_THRESHOLD_H;
        struct {
            Enabled_t wakeup:1;
            unsigned:7;
        } REG_WAKE_EN;
        struct {
            byte wakePending:1;
            unsigned:7;
        } REG_WAKE_STAT;
        struct {
            Reset_t reset:1;
            Enabled_t PACTLInvert:1;
            Enabled_t PAOutput:1;
            unsigned:2;
            Enabled_t MIDReadAccess:1;
            Enabled_t PWRWriteAccess:1;
            unsigned:1;
        } REG_ANALOG_CTL;
        struct {
            byte channel:7;
            unsigned:1;
        } REG_CHANNEL;
        struct {
            byte RSSI:5;
            boolean valid:1;
            unsigned:2;
        } REG_RSSI;
        struct {
            byte transmitPower:3; //7=MAX, 0=MIN
            unsigned:5;
        } REG_PA;
        struct {
            byte calibrateValue:6;
            Enabled_t clockOut:1;
            unsigned:1;
        } REG_CRYSTAL_ADJ;
        struct {
            unsigned:6;
            VCOAdjust_t VCOSlope:2;
        } REG_VCO_CAL;
        struct {
            unsigned:7;
            Enabled_t powerSaveMode:1;
        } REG_PWR_CTL;
        struct {
            unsigned:7;
            Enabled_t carrierDetectOverride:1;
        } REG_CARRIER_DETECT;
        byte REG_CLOCK_MANUAL;
        byte REG_CLOCK_ENABLE;
        byte REG_SYN_LOCK_CNT;
        
        CYWM6935RegisterData_t() {
            clear();
        }
        inline void clear() {
            __private_data = 0; //clear the union.
        }
    } CYWM6935RegisterData;
    
    typedef union {
        union {
            byte bytes[8];
            unsigned long long value;
        } REG_PN_CODE;
        struct {
            union {
                byte bytes[4];
                unsigned long value;
            };
            //Encased in struct to ensure alignment!
            unsigned long:32;//byte __empty__[4];
        } REG_MID;
        unsigned long long __private_data;
    } CYWM6935ExtendedRegisterData;

    typedef struct {
        CYWM6935Address address:6;
        Enabled_t increment:1;
        ReadWrite_t direction:1;
    } CYWM6935Register;

    typedef struct {
        CYWM6935ExtendedAddress address:6;
        Enabled_t increment:1;
        ReadWrite_t direction:1;
    } CYWM6935ExtendedRegister;

    typedef union CYWM6935CommandMake{
        CYWM6935Register reg;
        CYWM6935ExtendedRegister extReg;
        byte __private_data;
        CYWM6935CommandMake() {
            clear();
        }
        CYWM6935CommandMake(CYWM6935ExtendedAddress cmd) {
            extReg.address = cmd;
        }
        CYWM6935CommandMake(CYWM6935Address cmd) {
            reg.address = cmd;
        }
        inline void clear() {
            __private_data = 0; //clear the union.
        }
    } CYWM6935Command;

    #if (RAMEND < 250)
      #define BUFFER_SIZE 8
    #elif (RAMEND < 500)
      #define BUFFER_SIZE 16
    #elif (RAMEND < 1000)
      #define BUFFER_SIZE 32
    #else
      #define BUFFER_SIZE 128
    #endif
    typedef struct
    {
      unsigned char buffer[BUFFER_SIZE];
      int head;
      int tail;
    } SoftRingBuffer;

    class CYWM6935 : public Stream
    {
    public:
    #if ARDUINO >= 100
        virtual size_t write(const unsigned char character);//
    #else
        virtual void write(const unsigned char character);//
    #endif
        using Print::write;
        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        
        
        CYWM6935(SoftRingBuffer* __rx_buffer);
        ~CYWM6935();
        
        void begin(byte channel, byte SCK_, byte MOSI_, byte MISO_, byte SS_);
#if defined(MOSI) && defined(MISO) && defined(SCK)
        void begin(byte channel,byte SS_) ;
#endif
#if defined(SS) && defined(MOSI) && defined(MISO) && defined(SCK)
        void begin(byte channel);
#endif
        void resume();
        void end();
        
        void setChannel(byte channel);
        byte currentChannel();
        
        boolean isTransmitting();
        void setTransmitting(boolean transmitting);
        boolean isReceiving();
        void setReceiving(boolean receiving);
        
        void writeExtendedRegister(CYWM6935Command command, CYWM6935ExtendedRegisterData data);
        CYWM6935ExtendedRegisterData readExtendedRegister(CYWM6935Command command);
        
        void writeRegister(CYWM6935Command command, CYWM6935RegisterData data);
        void writeRegister(CYWM6935Command command, CYWM6935RegisterData* data, byte count);
        CYWM6935RegisterData readRegister(CYWM6935Command command);
        void readRegister(CYWM6935Command command, CYWM6935RegisterData* data, byte count);
        
        void setTransmitter(Enabled_t status);
        Enabled_t transmitterIs();
        void setReciever(Enabled_t status);
        Enabled_t recieverIs();
        
#ifdef INCLUDE_RSSI
        const uint8_t RSSI_peak(const uint8_t channel, const uint8_t count);
#endif
        byte _tx_buffer;
    private:
        
        void _begin(byte channel);
        
        void __private_writeRegister(byte command, byte* data, byte count);
        void __private_readRegister(byte command, byte* data, byte count);
        
        
        SoftRingBuffer* _rx_buffer;
        
        byte _SS;
        byte _SCLK;
        byte _MISO;
        byte _MOSI;
        
        byte _channel;
        
        Enabled_t _transmitterEn;
        Enabled_t _recieverEn;
        
        boolean _transmitting;
        boolean _receiving;
        
        CYWM6935RegisterData _intRX;
        CYWM6935RegisterData _intTX;
    };
    
    extern CYWM6935 radio;
    
#endif //  _CYWM6935_H_
