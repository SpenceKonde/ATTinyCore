/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |     
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013/2014

                http://p.loussouarn.free.fr

         *******************************************************
         *              <TinyPinChange> library Demo           *
         *            with display capabilities using          *
         * <SoftSerial> object as single wire serial interface *
         *******************************************************

This "Tiny RC Scope" sketch demonstrates how to use <TinyPinChange> and <SoftSerial> libraries.
"Tiny RC Scope" acts as a simple real time ASCII oscilloscope for displaying one RC Channel in the serial console.
The displayed measurement (in µs) are: pulse width and RC period.

Trick: By connecting Pin#1 to Pin#0, through a 1K resistor, you can measure the RC Signal provided by the built-in RC generator for testing purpose.
Output results are sent to a software serial port. If a real RC signal is connected to Pin0, the trace is displayed in real time in the terminal.

And the great thing is: using a <SoftSerial> object as a bi-directionnal software serial port (half-duplex) on a single pin to communicate with the outside world!

To display the sketch results on a PC (in a Terminal):
1)  Build the "Serial One Wire Debug Cable" and plug it to the regular RS232 port as depicted below.
2)  Open your favorite Terminal at 57600,n,8,1: HyperTerminal, Teraterm (Windows) or Minicom, GtkTerm (Linux) and CoolTerm (MAC) does the trick.
3)  You can also use the Serial Monitor of the arduino IDE: Tools->Serial Port and select your RS232 port (may be an USB virtual port), Rate=57600.
4)  To test "Tiny RC Scope", connect Pin1 to Pin0, and look at the Terminal (57600,n,8,1) connected to Pin2 through a debug cable ()
5)  The wave form should be displayed in the Terminal,
6)  Type - to decrease the pulse width (-10us),
7)  Type + to increase the pulse width (+10us),
8)  Type m to set the pulse width to its minimum (500us),
9)  Type n or N to set the pulse width to its Neutral value (1500us),
10) Type M to set the pulse width to its Maximum (2500us),
11) To measure real RC signals, disconnect the Pin1 from Pin0 and connect a RC receiver output to Pin0,
12) The Terminal will display in real time the pulse width of the connected RC channel.
13) If the channel is not connected, a flat line is displayed.

                        SERIAL ONE WIRE
                        DEBUGGING CABLE
            _______________          ________________
           /               \___/\___/                \
                                              ____
 .--------.                                  |    \
 |    GND |--------------------------------+---o5  \
 |        |                           47K  | |   9o |
 |        |                        .--###--' | o4   |
 |  DEBUG |                  4.7K  |         |   8o |
 |  TX_RX |-------------------###--+--|<|------o3   |    ---> To regular RS232 SubD 9 pins Male of PC or Serial/USB adapter
 |   PIN  |        ^               | 1N4148  |   7o |
 |        |        |               '-----------o2   |
 '--------'        |                         |   6o |
  ATtiny85       Single                      | o1  /
 (Digispark)      I/O                        |____/
   (pro)                                  SubD 9 pins
                                            Female
*/
#include <TinyPinChange.h>
#include <SoftSerial.h>

#define RC_CHANNEL_PIN          0 /* RC Channel is connected to pin 0 */
#define RC_GEN_PIN              1 /* Pin used as internal RC generator for test purpose */

#define RC_PINS_MSK             (_BV(RC_CHANNEL_PIN) | _BV(RC_GEN_PIN))

#define DEBUG_TX_RX_PIN         2

#define DEF_TEST_RC_CH_WIDTH_US 1500 /* This value can be change via the Terminal */
#define TEST_RC_PERIOD_US       20000

#define PULSE_MAX_US            2500
#define PULSE_MIN_US            500
#define STEP_US                 10

#define ERR_MARGIN_US           150

#define SERIAL_BAUD_RATE        57600 /* 57600 is the maximum for Receiving commands from the serial port: 1 char -> #200us */
SoftSerial MySerial(DEBUG_TX_RX_PIN, DEBUG_TX_RX_PIN, true); /* Tx/Rx on a single Pin !!! (Pin#2) */

#define ONE_CHAR_TX_TIME_US	200 /* @ 57600 bauds */

enum {PULSE_SRC_INTERNAL, PULSE_SRC_EXTERNAL};

typedef struct {
  uint32_t RisingStartUs;
  uint32_t RcWidth_us;
  uint32_t LastRxPulseMs;
  boolean  FallingEdgeFound;
}RcChSt_t;

volatile RcChSt_t Ch; /* volatile, since value are used in ISR and in the loop() */
volatile uint32_t RcPeriod_us = TEST_RC_PERIOD_US;
uint32_t TestRcWidth_us = DEF_TEST_RC_CH_WIDTH_US;

/* The different states of the display state machine */
enum {DISP_COMPUTE, DISP_FIRST_LINE, DISP_PREP_SEC_LINE, DISP_SECOND_LINE, DISP_PREP_THIRD_LINE, DISP_THIRD_LINE, DISP_WAIT};

#define LINE_LEN 38

typedef struct{
  char    Line[LINE_LEN];
  uint8_t Idx;
  uint8_t State;
  uint8_t HighNb;
}DispSt_t;

static DispSt_t Disp;

volatile uint8_t IntRcSynch = 0;
uint8_t VirtualPortIdx;
/*
RC Signal
  ____                              ____ 
_|    |____________________________|    |_
 <---->
Width_us
 <-------------------------------->
             Period_us
*/
void setup()
{
  TinyPinChange_Init();

  MySerial.begin(SERIAL_BAUD_RATE); /* Trick: use a "high" data rate (less time wasted in ISR and for transmitting each character) */

  Disp.State = DISP_COMPUTE;
  
  VirtualPortIdx = TinyPinChange_RegisterIsr(RC_CHANNEL_PIN,  InterruptFunctionToCall); /* As all pins are on the same port, a single ISR is needed */
  pinMode(RC_CHANNEL_PIN, INPUT);
  digitalWrite(RC_CHANNEL_PIN, HIGH); /* Enable Pull-up to avoid floating inputs in case of nothing connected to them */
  TinyPinChange_EnablePin(RC_CHANNEL_PIN);

  Ch.RcWidth_us = 0;
  Ch.FallingEdgeFound = 0;
  
  MySerial.txMode();
  MySerial.println(F("\n -- Tiny RC Scope V1.0 (C) RC Navy 2014 --\n"));
  MySerial.rxMode(); /* Switch to Rx Mode */
  
  pinMode(RC_GEN_PIN, OUTPUT);
}

void loop()
{
  uint32_t RcGeneStartUs  = micros();
  static uint32_t ProcessStartUs = micros();
  static uint32_t DisplayStartMs = millis();
  uint32_t ProcessDurationUs;
  uint32_t HalfRemaingLowUs;
  char RxChar;
  
  /* Blink the built-in LED (Built-in RC Signal generator) */
  if(IsInternalRcSrc())
  {
    RcGeneStartUs  = micros();
    digitalWrite(RC_GEN_PIN, HIGH);
    while(micros() - RcGeneStartUs < TestRcWidth_us);
    digitalWrite(RC_GEN_PIN, LOW);
  }
  /********************/
  /* Start of process */
  /********************/
  ProcessStartUs = micros();
  DisplayRcMeasurement(5000U);/* Gives 5000 us to display a part of the trace */
  /* Get command from single wire SoftSerial (to tune the built-in generator) */
  if(MySerial.available() > 0)
  {
    RxChar = MySerial.read();MySerial.txMode();MySerial.println("");MySerial.rxMode(); /* Carriage return after the echo */
    switch(RxChar)
    {
      case '-': /* Decrease Built-in RC Pulse */
      if((TestRcWidth_us - STEP_US) >= PULSE_MIN_US)
      {
	TestRcWidth_us -= STEP_US;
      }
      break;
      
      case '+': /* Increase Built-in RC Pulse */
      if(TestRcWidth_us + STEP_US <= PULSE_MAX_US)
      {
	TestRcWidth_us += STEP_US;
      }
      break;

      case 'm': /* Set Built-in RC Pulse o min value: 500 */
      TestRcWidth_us = PULSE_MIN_US;
      break;

      case 'N': /* Set Built-in RC Pulse to Neutral: 1500 */
      case 'n':
      TestRcWidth_us = DEF_TEST_RC_CH_WIDTH_US;
      break;

      case 'M': /* Set Built-in RC Pulse to Max value: 2500 */
      TestRcWidth_us = PULSE_MAX_US;
      break;

      default:
      /* Ignore */
      break;
    }
  }
  /********************/
  /*  End of process  */
  /********************/
  ProcessDurationUs = micros() - ProcessStartUs; //Compute how many us took the previous instructions
  if(IsInternalRcSrc())
  {  
    HalfRemaingLowUs = TEST_RC_PERIOD_US - (ProcessDurationUs + TestRcWidth_us + 100UL);
    RcGeneStartUs  = micros();
    while((micros() - RcGeneStartUs) < HalfRemaingLowUs);
  }
}
/*
RC Signal
  ____                              ____ 
_|    |____________________________|    |_
 <---->
Width_us
 <-------------------------------->
             Period_us
      <-------------------->
    Display is processed here

Explanation: the display of the trace is performed just after the falling edge of the RC pulse
As the duration between 2 pulses is too short to display the full trace, the trace is displayed part by part.
A full trace is composed of around 100 characters:
At 57600 bauds, one character takes (1/57600) * 10 = 174 us. As there are some overhead, it is closer than 200us.
So, the full trace takes 100 x 200 = 20000 us = 20 ms. It's impossible to display the full trace between 2 consecutive pulses.
DisplayRcMeasurement() function is the very tricky part of this sketch: it has as argument an amount of time and exits before
it exceeds it. A state machine is used to memorize where the display was arrived.
*/
uint8_t DisplayRcMeasurement(uint16_t FreetimeUs)
{
  static uint32_t LocalRcWidth_us;
  static uint32_t LocalRcPeriod_us;
  static uint32_t StartWaitMs;
  char *Ptr;

  uint32_t ProcessStart_us;
  uint32_t Elapsed_us;
  uint8_t  StartIdx, Idx;
  uint8_t  Ret = 0;

  switch(Disp.State)
  {
    case DISP_COMPUTE:
DispCompute:
    if(Ch.FallingEdgeFound)
    {
DispComputeNoSignal:
      ProcessStart_us = micros();
      Ch.FallingEdgeFound = 0;
      noInterrupts(); /* Mandatory since RcWidth_us and RcPeriod_us are 32 bits */
      LocalRcWidth_us  = Ch.RcWidth_us;
      LocalRcPeriod_us = RcPeriod_us;
      interrupts();
    
      if(LocalRcWidth_us < (PULSE_MIN_US - ERR_MARGIN_US) || LocalRcWidth_us > (PULSE_MAX_US + ERR_MARGIN_US)) LocalRcWidth_us = 0; /* Out of Range */
      Disp.HighNb = (LocalRcWidth_us + 50UL) / 100;
      strcpy_P(Disp.Line, PSTR("Ch(P0)__"));
      if(LocalRcWidth_us)
      {
	Ptr = Disp.Line + 8;
	for(Idx = 0; Idx < Disp.HighNb; Idx++) *Ptr++='_';*Ptr++='\n';*Ptr=0;
      }
      else Disp.Line[6] = 0;
      Elapsed_us = micros() - ProcessStart_us;
      FreetimeUs -= (uint16_t)Elapsed_us;
      Disp.Idx=0;
      Disp.State = DISP_FIRST_LINE;
      if(FreetimeUs >= ONE_CHAR_TX_TIME_US)
      {
	goto DispFirstLine;
      }
    }
    else
    {
      if(millis() - StartWaitMs >= 1000UL)
      {
	LocalRcWidth_us = 0;
	goto DispComputeNoSignal;
      }
    }
    break;

    case DISP_PREP_SEC_LINE:
DispPrepSecLine:
    /* Prepare second line */
    ProcessStart_us = micros();
    if(LocalRcWidth_us)
    {
      strcpy_P(Disp.Line, PSTR("_____/"));
      for(Idx = 0; Idx < (2 + Disp.HighNb); Idx++) Disp.Line[6 + Idx] = ' ';
      itoa(LocalRcWidth_us, Disp.Line + 6, 10);StartIdx = 9; if(LocalRcWidth_us >= 1000) StartIdx++;Disp.Line[StartIdx++] = 'u';Disp.Line[StartIdx++] = 's';
      StartIdx = 6 + 2 + Disp.HighNb;Disp.Line[StartIdx++] = '\\';
    }
    else StartIdx = 0;
    for(Idx = 0; (StartIdx + Idx ) < (LINE_LEN - 2); Idx++) Disp.Line[StartIdx + Idx] = '_';Disp.Line[StartIdx + Idx] = '\n';Disp.Line[++StartIdx + Idx] = 0;
    Elapsed_us = micros() - ProcessStart_us;
    FreetimeUs -= (uint16_t)Elapsed_us;
    Disp.Idx=0;
    Disp.State = DISP_SECOND_LINE;
    if(FreetimeUs >= ONE_CHAR_TX_TIME_US)
    {
      goto DispSecondLine;
    }
    break;

    case DISP_PREP_THIRD_LINE:
DispPrepThirdLine:
    /* Prepare third line */
    ProcessStart_us = micros();
    strcpy_P(Disp.Line, PSTR("         RC Period: "));
    if(LocalRcWidth_us)
    {
      itoa(LocalRcPeriod_us, Disp.Line + 20, 10);
      strcat_P(Disp.Line, PSTR("us"));
    }
    else strcat_P(Disp.Line, PSTR("???"));
    Elapsed_us = micros() - ProcessStart_us;
    FreetimeUs -= (uint16_t)Elapsed_us;
    Disp.Idx=0;
    Disp.State = DISP_THIRD_LINE;
    if(FreetimeUs >= ONE_CHAR_TX_TIME_US)
    {
      goto DispThirdLine;
    }
    break;
    
    case DISP_FIRST_LINE:
    case DISP_SECOND_LINE:
    case DISP_THIRD_LINE:
DispFirstLine:
DispSecondLine:
DispThirdLine:
    ProcessStart_us = micros();
    MySerial.txMode();
    while(1)
    {
      if(Disp.Line[Disp.Idx])
      {
	MySerial.print(Disp.Line[Disp.Idx++]);
	if (micros() - ProcessStart_us >= (FreetimeUs - ONE_CHAR_TX_TIME_US))
	{
	  MySerial.rxMode();
	  break;  /* exit while(1) and stay in the current state */
	}
      }
      else
      {
	switch(Disp.State)
	{
	  case DISP_FIRST_LINE:
	  /* First line is fully displayed */
	  Disp.State = DISP_PREP_SEC_LINE;
	  if(FreetimeUs - Elapsed_us >= 200)
	  {
	    FreetimeUs -= (uint16_t)Elapsed_us;
	    goto DispPrepSecLine;
	  }
	  else
	  {
	    /* Not enough time: just change state */
	    MySerial.rxMode();
	  }
	  break;

	  case DISP_SECOND_LINE:
	  /* Second line is fully displayed */
	  Disp.State = DISP_PREP_THIRD_LINE;
	  if(FreetimeUs - Elapsed_us >= 200)
	  {
	    FreetimeUs -= (uint16_t)Elapsed_us;
	    goto DispPrepThirdLine;
	  }
	  else
	  {
	    /* Not enough time: just change state */
	    MySerial.rxMode();
	  }
	  break;

	  case DISP_THIRD_LINE:
	  /* Third line is fully displayed */
	  MySerial.print('\n');
	  StartWaitMs = millis();
	  Disp.State = DISP_WAIT;
	  MySerial.rxMode();
	  break;
	}
	break; /* exit while(1) */
      }
    }
    break;
    
    case DISP_WAIT:
    if(millis() - StartWaitMs >= 500UL) Disp.State = DISP_COMPUTE; /* Give some time to enter commands via the terminal (in internal source mode) */
    else Ch.FallingEdgeFound = false;
    break;
  }
  return(Ret);
}

/* The following function checks if the received signal is the image of the one of the built-in RC generator */
uint8_t IsInternalRcSrc()
{
  uint8_t Ret;
  static uint32_t SampleStartMs = millis();
  if(millis() - Ch.LastRxPulseMs >= 300UL) IntRcSynch = 100; /* Kick off Internal RC generator */
  Ret = (IntRcSynch >= 100);
  if(millis() - SampleStartMs >= 10UL)
  {
    SampleStartMs = millis();
    IntRcSynch = 99;
  }
  return(Ret);
}

/* Function called in interruption in case of change on RC pins: pulse width and RC period measurement */
void InterruptFunctionToCall(void)
{
  if(TinyPinChange_RisingEdge(VirtualPortIdx, RC_CHANNEL_PIN)) /* Check for RC Channel rising edge */
  {
    RcPeriod_us = micros() - Ch.RisingStartUs;
    Ch.RisingStartUs = micros();
  }
  else
  {
    Ch.RcWidth_us = micros() - Ch.RisingStartUs;
    Ch.FallingEdgeFound = true;
    Ch.LastRxPulseMs = millis();
    if(!(PINB & RC_PINS_MSK)) /* Check if RC_CHANNEL_PIN and RC_GEN_PIN are both to 0 */
    {
      if(IntRcSynch < 100) IntRcSynch++; /* if IntRcSynch reaches 100, it means the RC source is internal (synchronized) */
    }else IntRcSynch = 0; /* Not synchronized */
  }
}
