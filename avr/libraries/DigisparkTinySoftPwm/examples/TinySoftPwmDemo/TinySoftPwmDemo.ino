#include <TinySoftPwm.h>

/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr

         ****************************************
         *      <TinySoftPwm> library Demo      *
         ****************************************

This sketch increases the luminosity of the built-in LED of the Digispark.
When the luminosity reaches its maximum, the luminosity decreases.
When the luminosity reaches its minimum, the luminosity increases, and so on...

Note:
====
Declare the Pin(s) used in "librarie/TinySoftPwm/TinySoftPwm.h"
In this sketch, #define TINY_SOFT_PWM_USES_P1 must be enabled (not commented) since it uses the DigiSpark built-in LED wired on P1.

In this basic example, TinySoftPwm_process() is called periodically using micros(), but it is recommanded to call it from a timer ISR
to ensure a better periodicity.

*/

#define BUILT_IN_LED_PIN  1 /* Digispark Model A (Rev2) built-in LED pin number (Change it to 2 for Model B) */

void setup()
{
  TinySoftPwm_begin(128, 0); /* 128 x TinySoftPwm_process() calls before overlap (Frequency tuning), 0 = PWM init for all declared pins */
}

void loop()
{
static uint32_t StartUs=micros();
static uint32_t StartMs=millis();
static uint8_t Pwm=0;
static int8_t  Dir=1;

  /***********************************************************/
  /* Call TinySoftPwm_process() with a period of 60 us       */
  /* The PWM frequency = 128 x 60 # 7.7 ms -> F # 130Hz      */
  /* 128 is the first argument passed to TinySoftPwm_begin() */
  /***********************************************************/
  if((micros() - StartUs) >= 60)
  {
    /* We arrived here every 60 microseconds */
    StartUs=micros();
    TinySoftPwm_process(); /* This function shall be called periodically (like here, based on micros(), or in a timer ISR) */
  }
  
  /*************************************************************/
  /* Increment/decrement PWM on LED Pin with a period of 10 ms */
  /*************************************************************/
  if((millis()-StartMs) >= 10)
  {
    /* We arrived here every 10 milliseconds */
    StartMs=millis();
    Pwm+=Dir; /* increment or decrement PWM depending of sign of Dir */
    TinySoftPwm_analogWrite(BUILT_IN_LED_PIN, Pwm); /* Update built-in LED for Digispark */
    if(Pwm==255) Dir=-1; /* if PWM reaches the maximum: change direction */
    if(Pwm==0)   Dir=+1; /* if PWM reaches the minimum: change direction */
  }
}


