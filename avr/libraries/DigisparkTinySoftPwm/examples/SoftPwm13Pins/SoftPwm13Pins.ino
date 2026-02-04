#include <TinySoftPwm.h>

/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2015

                http://p.loussouarn.free.fr

         ****************************************
         *      <TinySoftPwm> library Demo      *
         ****************************************

This sketch generates simultaneously PWM signals on 13 pins (Pin 0 to pin 12 of the Digispark pro).
It also increases the luminosity of the built-in LED of the Digispark whilst the duty cycle remains constant for all other pins.
When the luminosity reaches its maximum, the luminosity decreases.
When the luminosity reaches its minimum, the luminosity increases, and so on...

Note:
====
Declare the Pin(s) used in "librarie/TinySoftPwm/TinySoftPwm.h"
In this sketch, #define TINY_SOFT_PWM_USES_PINO to TINY_SOFT_PWM_USES_PIN12 must be enabled (not commented) since it uses the first 13 pins of the DigiSpark pro.

In this basic example, TinySoftPwm_process() is called periodically using micros(), but it is recommanded to call it from a timer ISR
to ensure a better periodicity.

*/

#define BUILT_IN_LED_PIN  1 /* Digispark Model A (Rev2) built-in LED pin number (Change it to 2 for Model B) */

void setup()
{
  TinySoftPwm_begin(255, 0); /* 255 x TinySoftPwm_process() calls before overlap (Frequency tuning), 0 = PWM init for all declared pins */
  for(uint8_t PinIdx = 0; PinIdx <= 12; PinIdx++)
  {
    TinySoftPwm_analogWrite(PinIdx, (PinIdx + 1) * 19); /* Low to high duty cycle for pin 0 to 12 */
  }
}

void loop()
{
static uint32_t StartUs = micros();
static uint32_t StartMs = millis();
static uint8_t  Pwm = 0;
static int8_t   Dir = 1;

  /***********************************************************/
  /* Call TinySoftPwm_process() with a period of 40 us       */
  /* The PWM frequency = 255 x 40 # 10.2 ms -> F # 100Hz     */
  /* 255 is the first argument passed to TinySoftPwm_begin() */
  /***********************************************************/
  if((micros() - StartUs) >= 40)
  {
    /* We arrived here every 40 microseconds */
    StartUs = micros();
    TinySoftPwm_process(); /* This function shall be called periodically (like here, based on micros(), or in a timer ISR) */
  }
  
  /*************************************************************/
  /* Increment/decrement PWM on LED Pin with a period of 10 ms */
  /*************************************************************/
  if((millis() - StartMs) >= 10)
  {
    /* We arrived here every 10 milliseconds */
    StartMs = millis();
    Pwm += Dir; /* increment or decrement PWM depending of sign of Dir */
    TinySoftPwm_analogWrite(BUILT_IN_LED_PIN, Pwm); /* Update built-in LED for Digispark */
    if(Pwm == 255) Dir = -1; /* if PWM reaches the maximum: change direction */
    if(Pwm == 0)   Dir = +1; /* if PWM reaches the minimum: change direction */
  }
}

