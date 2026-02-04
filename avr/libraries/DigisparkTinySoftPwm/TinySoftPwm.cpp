// a Tiny optimized Software PWM Manager (all pins must be part of the same port)
// Only resources RAM/Program Memory of used pins are declared in the code at compilation time.
// based largely on Atmel's AVR136: Low-Jitter Multi-Channel Software PWM Application Note:
// http://www.atmel.com/dyn/resources/prod_documents/doc8020.pdf
// RC Navy 2013-2015
// http://p.loussouarn.free.fr
// 11/01/2015: Multi port support added for ATtiny167

#include <TinySoftPwm.h> 

#if (TINY_SOFT_PWM_CH_MAX == 0)
#error At least one PWM pin shall be declared in TinySoftPwm.h
#endif

#if defined(TINY_SOFT_PWM_DDR1) && defined(TINY_SOFT_PWM_DDR0)
#define TINY_SOFT_PWM_DECLARE_PIN(Pin)          do{                                                        \
                                                  if(digitalPinToPortIdx(Pin))                             \
                                                  {                                                        \
                                                    TINY_SOFT_PWM_DDR1 |= (1 << digitalPinToPortBit(Pin)); \
                                                    Port1_PwmMask |= (1 << digitalPinToPortBit(Pin));      \
                                                  }                                                        \
                                                  else                                                     \
                                                  {                                                        \
                                                    TINY_SOFT_PWM_DDR0 |= (1 << digitalPinToPortBit(Pin)); \
                                                    Port0_PwmMask |= (1 << digitalPinToPortBit(Pin));      \
                                                  }                                                        \
                                                }while(0)

#define TINY_SOFT_PWM_CLEAR_PIN(RamIdx)         GET_PWM_PIN_PORT(RamIdx)                    \
                                                ? (Port1_PwmTo1 &= GET_PWM_INV_MSK(RamIdx)) \
                                                : (Port0_PwmTo1 &= GET_PWM_INV_MSK(RamIdx))
#else
#if defined(TINY_SOFT_PWM_DDR1)
#define TINY_SOFT_PWM_DECLARE_PIN(Pin)          TINY_SOFT_PWM_DDR1 |= (1 << digitalPinToPortBit(Pin)); Port1_PwmMask |= (1 << digitalPinToPortBit(Pin))
#define TINY_SOFT_PWM_CLEAR_PIN(RamIdx)         Port1_PwmTo1 &= GET_PWM_INV_MSK(RamIdx)
#else
#define TINY_SOFT_PWM_DECLARE_PIN(Pin)          TINY_SOFT_PWM_DDR0 |= (1 << digitalPinToPortBit(Pin)); Port0_PwmMask |= (1 << digitalPinToPortBit(Pin))
#define TINY_SOFT_PWM_CLEAR_PIN(RamIdx)         Port0_PwmTo1 &= GET_PWM_INV_MSK(RamIdx)
#endif
#endif

#define GET_PWM_PIN_ID(RamIdx)                  ((uint8_t)pgm_read_byte(&PwmPin[(RamIdx)].Id))
#define GET_PWM_PIN_PORT(RamIdx)                ((uint8_t)pgm_read_byte(&PwmPin[(RamIdx)].Port))
#define GET_PWM_INV_MSK(RamIdx)                 ((uint8_t)pgm_read_byte(&PwmPin[(RamIdx)].InvMsk))


typedef struct {
  uint8_t Id;
  uint8_t Port;
  uint8_t InvMsk;
}SoftPwmPinSt_t;

const SoftPwmPinSt_t PwmPin[] PROGMEM ={
#if (TINY_SOFT_PWM_USES_PIN0 == 1)
  {0,  digitalPinToPortIdx(0),  ~(1 << digitalPinToPortBit(0))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN1 == 1)
  {1,  digitalPinToPortIdx(1),  ~(1 << digitalPinToPortBit(1))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN2 == 1)
  {2,  digitalPinToPortIdx(2),  ~(1 << digitalPinToPortBit(2))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN3 == 1)
  {3,  digitalPinToPortIdx(3),  ~(1 << digitalPinToPortBit(3))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN4 == 1)
  {4,  digitalPinToPortIdx(4),  ~(1 << digitalPinToPortBit(4))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN5 == 1)
  {5,  digitalPinToPortIdx(5),  ~(1 << digitalPinToPortBit(5))},
#endif
#if defined (__AVR_ATtiny167__)
#if (TINY_SOFT_PWM_USES_PIN6 == 1)
  {6,  digitalPinToPortIdx(6),  ~(1 << digitalPinToPortBit(6))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN7 == 1)
  {7,  digitalPinToPortIdx(7),  ~(1 << digitalPinToPortBit(7))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN8 == 1)
  {8,  digitalPinToPortIdx(8),  ~(1 << digitalPinToPortBit(8))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN9 == 1)
  {9,  digitalPinToPortIdx(9),  ~(1 << digitalPinToPortBit(9))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN10 == 1)
  {10, digitalPinToPortIdx(10), ~(1 << digitalPinToPortBit(10))},
#endif  
#if (TINY_SOFT_PWM_USES_PIN11 == 1)
  {11, digitalPinToPortIdx(11), ~(1 << digitalPinToPortBit(11))},
#endif
#if (TINY_SOFT_PWM_USES_PIN12 == 1)
  {12, digitalPinToPortIdx(12), ~(1 << digitalPinToPortBit(12))},
#endif
#endif
};

static   uint8_t Compare[TINY_SOFT_PWM_CH_MAX];
volatile uint8_t PwmOrder[TINY_SOFT_PWM_CH_MAX];
#ifdef TINY_SOFT_PWM_PORT0
static   uint8_t Port0_PwmMask = 0;
volatile uint8_t Port0_PwmTo1  = 0x00;
volatile uint8_t Port0_PwmTo0  = 0xFF;
#endif
#ifdef TINY_SOFT_PWM_PORT1
static   uint8_t Port1_PwmMask = 0;
volatile uint8_t Port1_PwmTo1  = 0x00;
volatile uint8_t Port1_PwmTo0  = 0xFF;
#endif
static   uint8_t _TickMax = 255;

static uint8_t PwmToPwmMax(uint8_t Pwm);

void TinySoftPwm_begin(uint8_t TickMax, uint8_t PwmInit)
{
uint8_t oldSREG = SREG;
  cli();
  // set the direction of the used ports and update PortPwmMask
#if (TINY_SOFT_PWM_USES_PIN0 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(0);
#endif
#if (TINY_SOFT_PWM_USES_PIN1 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(1);
#endif
#if (TINY_SOFT_PWM_USES_PIN2 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(2);
#endif
#if (TINY_SOFT_PWM_USES_PIN3 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(3);
#endif
#if (TINY_SOFT_PWM_USES_PIN4 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(4);
#endif
#if (TINY_SOFT_PWM_USES_PIN5 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(5);
#endif
#if (TINY_SOFT_PWM_USES_PIN6 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(6);
#endif
#if (TINY_SOFT_PWM_USES_PIN7 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(7);
#endif
#if (TINY_SOFT_PWM_USES_PIN8 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(8);
#endif
#if (TINY_SOFT_PWM_USES_PIN9 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(9);
#endif
#if (TINY_SOFT_PWM_USES_PIN10 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(10);
#endif
#if (TINY_SOFT_PWM_USES_PIN11 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(11);
#endif
#if (TINY_SOFT_PWM_USES_PIN12 == 1)
  TINY_SOFT_PWM_DECLARE_PIN(12);
#endif
  _TickMax = TickMax;
#ifdef TINY_SOFT_PWM_PORT0
  Port0_PwmTo1 = Port0_PwmMask;
#endif
#ifdef TINY_SOFT_PWM_PORT1
  Port1_PwmTo1 = Port1_PwmMask;
#endif
  // initialise all channels
  for(uint8_t i = 0; i < TINY_SOFT_PWM_CH_MAX; i++)
  {
    Compare[i]  = PwmToPwmMax(PwmInit);  // set default PWM values
    PwmOrder[i] = Compare[i]; // set default PWM values
  }
  SREG = oldSREG;
}

void TinySoftPwm_analogWrite(uint8_t Pin, uint8_t Pwm)
{
uint8_t RamIdx;

  for(RamIdx = 0; RamIdx < TINY_SOFT_PWM_CH_MAX; RamIdx++)
  {
    if(GET_PWM_PIN_ID(RamIdx) == Pin) break;
  }
  if(RamIdx < TINY_SOFT_PWM_CH_MAX)
  {
    PwmOrder[RamIdx] = PwmToPwmMax(Pwm);
  }
}

static uint8_t PwmToPwmMax(uint8_t Pwm)
{
  uint16_t Pwm16;
  Pwm16 = map(Pwm, 0, 255, 0, _TickMax);
  Pwm16 = constrain(Pwm16, 0, _TickMax);
  return((uint8_t)Pwm16);
}

void TinySoftPwm_process(void) 
{
static uint8_t OvfCount=0xFF;
 
#ifdef TINY_SOFT_PWM_PORT0
  Port0_PwmTo0 = (~Port0_PwmTo1) ^ Port0_PwmMask;
  TINY_SOFT_PWM_PORT0 |= Port0_PwmTo1; // update ONLY used outputs to 1 without disturbing the others
  TINY_SOFT_PWM_PORT0 &= Port0_PwmTo0; // update ONLY used outputs to 0 without disturbing the others
#endif
#ifdef TINY_SOFT_PWM_PORT1
  Port1_PwmTo0 = (~Port1_PwmTo1) ^ Port1_PwmMask;
  TINY_SOFT_PWM_PORT1 |= Port1_PwmTo1; // update ONLY used outputs to 1 without disturbing the others
  TINY_SOFT_PWM_PORT1 &= Port1_PwmTo0; // update ONLY used outputs to 0 without disturbing the others
#endif
  if(++OvfCount == _TickMax)
  { // increment modulo 256 counter and update
    // the compare values only when counter = 0.
    OvfCount=0;
#if (TINY_SOFT_PWM_CH_MAX >= 1)
    Compare[0] = PwmOrder[0]; // verbose code for speed
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 2)
    Compare[1] = PwmOrder[1];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 3)
    Compare[2] = PwmOrder[2];
#endif   
#if (TINY_SOFT_PWM_CH_MAX >= 4)
    Compare[3] = PwmOrder[3];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 5)
    Compare[4] = PwmOrder[4];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 6)
    Compare[5] = PwmOrder[5];
#endif   
#if (TINY_SOFT_PWM_CH_MAX >= 7)
    Compare[6] = PwmOrder[6]; // verbose code for speed
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 8)
    Compare[7] = PwmOrder[7];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 9)
    Compare[8] = PwmOrder[8];
#endif   
#if (TINY_SOFT_PWM_CH_MAX >= 10)
    Compare[9] = PwmOrder[9];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 11)
    Compare[10] = PwmOrder[10];
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 12)
    Compare[11] = PwmOrder[11];
#endif   
#if (TINY_SOFT_PWM_CH_MAX >= 13)
    Compare[12] = PwmOrder[12];
#endif   
#ifdef TINY_SOFT_PWM_PORT0
    Port0_PwmTo1 = Port0_PwmMask; // set all port used pins high
#endif
#ifdef TINY_SOFT_PWM_PORT1
    Port1_PwmTo1 = Port1_PwmMask; // set all port used pins high
#endif
  }
  // clear port pin on compare match (executed on next interrupt)
#if (TINY_SOFT_PWM_CH_MAX >= 1)
  if(Compare[0] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(0);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 2)
  if(Compare[1] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(1);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 3)
  if(Compare[2] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(2);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 4)
  if(Compare[3] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(3);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 5)
  if(Compare[4] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(4);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 6)
  if(Compare[5] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(5);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 7)
  if(Compare[6] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(6);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 8)
  if(Compare[7] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(7);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 9)
  if(Compare[8] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(8);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 10)
  if(Compare[9] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(9);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 11)
  if(Compare[10] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(10);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 12)
  if(Compare[11] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(11);
#endif
#if (TINY_SOFT_PWM_CH_MAX >= 13)
  if(Compare[12] == OvfCount) TINY_SOFT_PWM_CLEAR_PIN(12);
#endif
}
