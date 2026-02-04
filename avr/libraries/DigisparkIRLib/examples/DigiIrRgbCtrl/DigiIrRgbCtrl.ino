#include <TinySoftPwm.h>
#include <IRLib.h>
/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr

         *************************************************
         *   Optimized <IRLib> library Controller Demo   *
         *************************************************

This sketch allows you to use the Digispark as an IR RGB Controller.
This sketch is designed to used low cost 24 keys IR Remote Control for RGB strip LED,
but you can adapt it to your own IR Remote Control.

 Sensor wiring: (Warning: the wiring may vary depending of the model of IR sensor)
 =============
        .-------.
        |  ___  |
        | /   \ | InfraRed
        | \___/ |  Sensor
        |       |
        '+--+--+'
         |  |  |      100
 P5 <----'  |  '--+---###--- +5V
            |     |
            |   '==='4.7uF
            |     |
            '--+--'
               |
              GND

/* P0, P1 and P2 shall be declared in Digispark-Arduino-1.0.x/libraries/TinySoftPwm.h */
#define LED_GREEN_PIN  0
#define LED_RED_PIN    1
#define LED_BLUE_PIN   2

#define IR_RX_PIN      5

#define CODE_OFF           0xF740BF
#define CODE_ON            0xF7C03F
#define CODE_BRIGHT_MINUS  0xF7807F
#define CODE_BRIGHT_PLUS   0xF700FF

#define CODE_FLASH         0xF7D02F
#define CODE_STROBE        0xF7F00F
#define CODE_FADE          0xF7C837
#define CODE_SMOOTH        0xF7E817

#define CODE_RED           0xF720DF
#define CODE_GREEN         0xF7A05F
#define CODE_BLUE          0xF7609F
#define CODE_WHITE         0xF7E01F

#define CODE_ORANGE        0xF710EF
#define CODE_ORANGE_LIGTH  0xF730CF
#define CODE_BROWN         0xF708F7
#define CODE_YELLOW        0xF728D7

#define CODE_GREEN_LIGTH   0xF7906F
#define CODE_GREEN_BLUE1   0xF7B04F
#define CODE_GREEN_BLUE2   0xF78877
#define CODE_GREEN_BLUE3   0xF7A857

#define CODE_BLUE_LIGTH    0xF750AF
#define CODE_PURPLE_DARK   0xF7708F
#define CODE_PURPLE_LIGTH  0xF748B7
#define CODE_PINK          0xF76897

#define BRIGTH_STEP        10
#define CODE_REPEAT        0xFFFFFFFF

IRrecv   My_Receiver(IR_RX_PIN);//Receive on pin IR_RX_PIN
IRdecode My_Decoder; 

uint8_t PwmRed=0, PwmGreen=0, PwmBlue=0;

void setup() 
{ 
  My_Receiver.enableIRIn(); // Start the receiver
  pinMode(LED_RED_PIN, OUTPUT);
  TinySoftPwm_begin(255,0);
}

void loop() 
{ 
static uint32_t StartUs=micros(), LastIrCode=0;

  if(My_Receiver.GetResults(&My_Decoder))
  {
    My_Decoder.decode();
    if(My_Decoder.value==REPEAT && LastIrCode)
    {
      My_Decoder.value=LastIrCode;
    }
    switch(My_Decoder.value)
    {
      case CODE_BRIGHT_MINUS:
        Tune(&PwmRed,   -BRIGTH_STEP);
        Tune(&PwmGreen, -BRIGTH_STEP);
        Tune(&PwmBlue,  -BRIGTH_STEP);
        LastIrCode=CODE_BRIGHT_MINUS;
        break;
      case CODE_BRIGHT_PLUS:
        if(PwmRed)   Tune(&PwmRed,   BRIGTH_STEP);
        if(PwmGreen) Tune(&PwmGreen, BRIGTH_STEP);
        if(PwmBlue)  Tune(&PwmBlue,  BRIGTH_STEP);
        LastIrCode=CODE_BRIGHT_PLUS;
        break;
      default:
        LastIrCode=0; /* No repeat for the following codes */
        switch(My_Decoder.value)
        {
        case CODE_OFF:
          RGB(0x00, 0x00, 0x00);
          break;
        case CODE_ON:
          RGB(0x7A, 0x00, 0xBF);
          break;
        case CODE_RED:          RGB(0xFF, 0x00, 0x00);break;
        case CODE_GREEN:        RGB(0x00, 0xFF, 0x00);break;
        case CODE_BLUE:         RGB(0x00, 0x00, 0xFF);break;
        case CODE_WHITE:        RGB(0xFF, 0xFF, 0xFF);break;
        
        case CODE_ORANGE:       RGB(0xFF, 0x7F, 0x00); break;
        case CODE_ORANGE_LIGTH: RGB(0xFF, 0xAA, 0x00); break;
        case CODE_BROWN:        RGB(0xFF, 0xD4, 0x00); break;
        case CODE_YELLOW:       RGB(0xFF, 0xFF, 0x00); break;
    
        case CODE_GREEN_LIGTH:  RGB(0x00, 0xFF, 0xAA); break;
        case CODE_GREEN_BLUE1:  RGB(0x00, 0xFF, 0xFF); break;
        case CODE_GREEN_BLUE2:  RGB(0x00, 0xAA, 0xFF); break;
        case CODE_GREEN_BLUE3:  RGB(0x00, 0x55, 0xFF); break;
     
        case CODE_BLUE_LIGTH:   RGB(0x00, 0x00, 0x80); break;
        case CODE_PURPLE_DARK:  RGB(0x3F, 0x00, 0x80); break;
        case CODE_PURPLE_LIGTH: RGB(0x7A, 0x00, 0xBF); break;
        case CODE_PINK:         RGB(0xFF, 0x00, 0xFF); break;

        case CODE_FLASH:        /* to be implemented */break;
        case CODE_STROBE:       /* to be implemented */break;
        case CODE_FADE:         /* to be implemented */break;
        case CODE_SMOOTH:       /* to be implemented */break;
       
        default: 
          break;
        }
      break;
    }
    My_Receiver.resume();
    TinySoftPwm_analogWrite(LED_RED_PIN,   GammaCorrection(PwmRed));
    TinySoftPwm_analogWrite(LED_GREEN_PIN, GammaCorrection(PwmGreen));
    TinySoftPwm_analogWrite(LED_BLUE_PIN,  GammaCorrection(PwmBlue));
  }
  /***********************************************************/
  /* Call TinySoftPwm_process() with a period of 60 us       */
  /* The PWM frequency = 255 x 60 # 15 ms -> F # 65Hz        */
  /* 255 is the first argument passed to TinySoftPwm_begin() */
  /***********************************************************/
  if((micros() - StartUs) >= 60)
  {
    /* We arrived here every 60 microseconds */
    StartUs=micros();
    TinySoftPwm_process(); /* This function shall be called periodically (like here, based on micros(), or in a timer ISR) */
  }
}

void RGB(uint8_t Red, uint8_t Green, uint8_t Blue)
{
  PwmRed=Red;
  PwmGreen=Green;
  PwmBlue=Blue;
}

uint8_t GammaCorrection(uint8_t Pwm)
{
  return((Pwm*Pwm)>>8);
}

void Tune(uint8_t* Color, int8_t Offset)
{
    if (Offset > 0) {
        if ( *Color + Offset <= 255) {
            *Color += Offset;
        } else {
            *Color = 255;
        }
    } else {
        if (*Color + Offset >= 0) {
            *Color += Offset;
        } else {
 //           *Color = 0;
        }
  }
}

