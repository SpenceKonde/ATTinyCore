/* Tone.cpp

  A Tone Generator Library

  Written by Brett Hagman

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Version Modified By Date     Comments
------- ----------- -------- --------
0001    B Hagman    09/08/02 Initial coding
0002    B Hagman    09/08/18 Multiple pins
0003    B Hagman    09/08/18 Moved initialization from constructor to begin()
0004    B Hagman    09/09/26 Fixed problems with ATmega8
0005    B Hagman    09/11/23 Scanned prescalars for best fit on 8 bit timers
                    09/11/25 Changed pin toggle method to XOR
                    09/11/25 Fixed timer0 from being excluded
0006    D Mellis    09/12/29 Replaced objects with functions
0007    B Cook      10/05/03 Rewritten to only work with Timer1 and support direct hardware output
0008    B Cook      10/05/03 Rewritten so the timer can be selected at compile time

*************************************************/

#define DEBUG_TONE 0

#include <avr/interrupt.h>
#include "core_build_options.h"
#include "ToneTimer.h"
#include "pins_arduino.h"
#include "wiring.h"

#if (TONETIMER_NUMBER_PRESCALERS != 5) && (TONETIMER_NUMBER_PRESCALERS != 15)
#error Only five or fifteen prescalers are supported.  Update the code to support the number of actual prescalers.
#endif

#if TONETIMER_NUMBER_PRESCALERS == 15
#define TONETIMER_MAXIMUM_DIVISOR  ( (unsigned long)(TONETIMER_(PRESCALER_VALUE_15)) * (1L + (unsigned long)(TONETIMER_(MAXIMUM_OCR))) )
#endif

#if TONETIMER_NUMBER_PRESCALERS == 5
#define TONETIMER_MAXIMUM_DIVISOR  ( (unsigned long)(TONETIMER_(PRESCALER_VALUE_5)) * (1L + (unsigned long)(TONETIMER_(MAXIMUM_OCR))) )
#endif

const unsigned int Tone_Lowest_Frequency = (F_CPU + (2L * TONETIMER_MAXIMUM_DIVISOR - 1L)) / (2L * TONETIMER_MAXIMUM_DIVISOR);

#if (TONETIMER_(MAXIMUM_OCR) == 65535) && (TONETIMER_(PRESCALE_SET) == 1)
#if F_CPU <= 1000000
  #define TONE_FREQUENCY_CUTOFF_2  (7)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 8000000
  #define TONE_FREQUENCY_CUTOFF_3  (7)
  #define TONE_FREQUENCY_CUTOFF_2  (61)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 16000000
  #define TONE_FREQUENCY_CUTOFF_4  (1)
  #define TONE_FREQUENCY_CUTOFF_3  (15)
  #define TONE_FREQUENCY_CUTOFF_2  (122)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU > 16000000
  #define TONE_FREQUENCY_CUTOFF_4  (1)
  #define TONE_FREQUENCY_CUTOFF_3  (18)
  #define TONE_FREQUENCY_CUTOFF_2  (152)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#endif
#endif

#if (TONETIMER_(MAXIMUM_OCR) == 255) && (TONETIMER_(PRESCALE_SET) == 1)
#if F_CPU <= 1000000
  #define TONE_FREQUENCY_CUTOFF_5  (7)
  #define TONE_FREQUENCY_CUTOFF_4  (30)
  #define TONE_FREQUENCY_CUTOFF_3  (243)
  #define TONE_FREQUENCY_CUTOFF_2  (1949)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 8000000
  #define TONE_FREQUENCY_CUTOFF_5  (60)
  #define TONE_FREQUENCY_CUTOFF_4  (243)
  #define TONE_FREQUENCY_CUTOFF_3  (1949)
  #define TONE_FREQUENCY_CUTOFF_2  (15594)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 16000000
  #define TONE_FREQUENCY_CUTOFF_5  (121)
  #define TONE_FREQUENCY_CUTOFF_4  (487)
  #define TONE_FREQUENCY_CUTOFF_3  (3898)
  #define TONE_FREQUENCY_CUTOFF_2  (31189)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU > 16000000 //assume it's around 20 if it's over 16, since avr's don't usually work above low 20's anyway. 
  #define TONE_FREQUENCY_CUTOFF_5  (151)
  #define TONE_FREQUENCY_CUTOFF_4  (584)
  #define TONE_FREQUENCY_CUTOFF_3  (4677)
  #define TONE_FREQUENCY_CUTOFF_2  (37426)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#endif
#endif

#if (TONETIMER_(MAXIMUM_OCR) == 255) && (TONETIMER_(PRESCALE_SET) == 2)
#if F_CPU <= 1000000
  #define TONE_FREQUENCY_CUTOFF_12 (1)
  #define TONE_FREQUENCY_CUTOFF_11 (3)
  #define TONE_FREQUENCY_CUTOFF_10 (7)
  #define TONE_FREQUENCY_CUTOFF_9  (15)
  #define TONE_FREQUENCY_CUTOFF_8  (30)
  #define TONE_FREQUENCY_CUTOFF_7  (60)
  #define TONE_FREQUENCY_CUTOFF_6  (121)
  #define TONE_FREQUENCY_CUTOFF_5  (243)
  #define TONE_FREQUENCY_CUTOFF_4  (487)
  #define TONE_FREQUENCY_CUTOFF_3  (974)
  #define TONE_FREQUENCY_CUTOFF_2  (1949)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 8000000
  #define TONE_FREQUENCY_CUTOFF_15 (1)
  #define TONE_FREQUENCY_CUTOFF_14 (3)
  #define TONE_FREQUENCY_CUTOFF_13 (7)
  #define TONE_FREQUENCY_CUTOFF_12 (15)
  #define TONE_FREQUENCY_CUTOFF_11 (30)
  #define TONE_FREQUENCY_CUTOFF_10 (60)
  #define TONE_FREQUENCY_CUTOFF_9  (121)
  #define TONE_FREQUENCY_CUTOFF_8  (243)
  #define TONE_FREQUENCY_CUTOFF_7  (487)
  #define TONE_FREQUENCY_CUTOFF_6  (974)
  #define TONE_FREQUENCY_CUTOFF_5  (1949)
  #define TONE_FREQUENCY_CUTOFF_4  (3898)
  #define TONE_FREQUENCY_CUTOFF_3  (7797)
  #define TONE_FREQUENCY_CUTOFF_2  (15594)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#elif F_CPU <= 16000000
  #define TONE_FREQUENCY_CUTOFF_15 (3)
  #define TONE_FREQUENCY_CUTOFF_14 (7)
  #define TONE_FREQUENCY_CUTOFF_13 (15)
  #define TONE_FREQUENCY_CUTOFF_12 (30)
  #define TONE_FREQUENCY_CUTOFF_11 (60)
  #define TONE_FREQUENCY_CUTOFF_10 (121)
  #define TONE_FREQUENCY_CUTOFF_9  (243)
  #define TONE_FREQUENCY_CUTOFF_8  (487)
  #define TONE_FREQUENCY_CUTOFF_7  (974)
  #define TONE_FREQUENCY_CUTOFF_6  (1949)
  #define TONE_FREQUENCY_CUTOFF_5  (3898)
  #define TONE_FREQUENCY_CUTOFF_4  (7797)
  #define TONE_FREQUENCY_CUTOFF_3  (15594)
  #define TONE_FREQUENCY_CUTOFF_2  (31189)
  #define TONE_FREQUENCY_CUTOFF_1  (65535)
#endif
#endif


#if DEBUG_TONE
uint16_t debug_tone_last_OCRxA;
uint16_t debug_tone_last_CSV;
#endif


// timerx_toggle_count:
//  > 0 - duration specified
//  = 0 - stopped
//  < 0 - infinitely (until stop() method called, or new play() called)

static volatile long tone_timer_toggle_count;
static volatile uint8_t *tone_timer_pin_register;
static volatile uint8_t tone_timer_pin_mask;

static uint8_t tone_pin = 255;


void tone( uint8_t _pin, unsigned int frequency, unsigned long duration )
{
  tonetimer_(ocr_t)             ocr;
  tonetimer_(prescale_value_t)  csv;
  tonetimer_(cs_t)              csi;
  
  if ( tone_pin == 255 )
  {
    /* Set the timer to power-up conditions so we start from a known state */
    ToneTimer_SetToPowerup();

    /* 
      Compare Output Mode = Normal port operation, OCxA/OCxB disconnected.
      Waveform Generation Mode = 4; 0100; CTC; (Clear Timer on Compare); OCR1A; Immediate; MAX
      Clock Select = No clock source (Timer/Counter stopped).
      Note: Turn off the clock first to avoid ticks and scratches.
    */
    ToneTimer_SetWaveformGenerationMode( ToneTimer_(CTC_OCR) );

    /* If the tone pin can be driven directly from the timer */

    if ( (_pin == ToneTimer_OutputComparePinA) || (_pin == ToneTimer_OutputComparePinB) )
    {
      /* Pin toggling is handled by the hardware */
      tone_timer_pin_register = NULL;
      tone_timer_pin_mask = 0;

      if ( _pin == ToneTimer_OutputComparePinA )
      {
        /* Compare Output Mode = Toggle OCxA on Compare Match. */
        ToneTimer_SetCompareOutputModeA( ToneTimer_(Toggle) );
      }
      else // if ( _pin == ToneTimer_OutputComparePinB )
      {
        /* Compare Output Mode = Toggle OCxA on Compare Match. */
        ToneTimer_SetCompareOutputModeB( ToneTimer_(Toggle) );
      }
    }
    else
    {
      /* Save information needed by the interrupt service routine */
      tone_timer_pin_register = portOutputRegister( digitalPinToPort( _pin ) );
      tone_timer_pin_mask = digitalPinToBitMask( _pin );

      /* Compare Output Mode = Normal port operation, OCxA disconnected. */
      ToneTimer_DisconnectOutputs();
    }

    /* Ensure the pin is configured for output */
    pinMode( _pin, OUTPUT );

    tone_pin = _pin;
  }

  if ( tone_pin == _pin )
  {
    /* Stop the clock while we make changes. */

    ToneTimer_ClockSelect( ToneTimer_(Stopped) );

    /* Start the counter at zero to reduce ticks and scratches. */

    ToneTimer_SetCount( 0 );

    if ( frequency > 0 )
    {
      if ( frequency < Tone_Lowest_Frequency )
      {
        frequency = Tone_Lowest_Frequency;
      }

      /* Determine which prescaler to use */
      /* Set the Output Compare Register (rounding up) */

      #if defined( TONE_FREQUENCY_CUTOFF_15 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_15 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_15);
        csi = ToneTimer_(Prescale_Index_15);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_14 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_14 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_14);
        csi = ToneTimer_(Prescale_Index_14);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_13 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_13 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_13);
        csi = ToneTimer_(Prescale_Index_13);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_12 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_12 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_12);
        csi = ToneTimer_(Prescale_Index_12);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_11 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_11 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_11);
        csi = ToneTimer_(Prescale_Index_11);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_10 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_10 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_10);
        csi = ToneTimer_(Prescale_Index_10);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_9 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_9 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_9);
        csi = ToneTimer_(Prescale_Index_9);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_8 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_8 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_8);
        csi = ToneTimer_(Prescale_Index_8);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_7 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_7 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_7);
        csi = ToneTimer_(Prescale_Index_7);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_6 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_6 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_6);
        csi = ToneTimer_(Prescale_Index_6);
      }
      else
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_5 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_5 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_5);
        csi = ToneTimer_(Prescale_Index_5);
      }
      else 
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_4 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_4 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_4);
        csi = ToneTimer_(Prescale_Index_4);
      }
      else 
      #endif

      #if defined( TONE_FREQUENCY_CUTOFF_3 )
      if ( frequency <= TONE_FREQUENCY_CUTOFF_3 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_3);
        csi = ToneTimer_(Prescale_Index_3);
      }
      else
      #endif

      if ( frequency <= TONE_FREQUENCY_CUTOFF_2 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_2);
        csi = ToneTimer_(Prescale_Index_2);
      }

      else // if ( frequency <= TONE_FREQUENCY_CUTOFF_1 )
      {
        csv = TONETIMER_(PRESCALER_VALUE_1);
        csi = ToneTimer_(Prescale_Index_1);
      }

      ocr = ( (2L * F_CPU) / (frequency * 2L * csv) + 1L ) / 2L - 1L;
      ToneTimer_SetOutputCompareMatchAndClear( ocr );

      #if DEBUG_TONE
        debug_tone_last_OCRxA = ocr;
        debug_tone_last_CSV = csv;
      #endif

      /* Does the caller want a specific duration? */
      if ( duration > 0 )
      {
        /* Determine how many times the value toggles */
        tone_timer_toggle_count = (2 * frequency * duration) / 1000;

        /* Output Compare A Match Interrupt Enable */
        ToneTimer_EnableOutputCompareInterruptA();
      }
      else
      {
        /* Indicate to the interrupt service routine that we'll be running until further notice */
        tone_timer_toggle_count = -1;

        /* All pins but the OCxA / OCxB pins have to be driven by software */
        if ( (_pin != ToneTimer_OutputComparePinA) && (_pin != ToneTimer_OutputComparePinB) )
        {
          /* Output Compare A Match Interrupt Enable */
          ToneTimer_EnableOutputCompareInterruptA();
        }
      }

      /* Start the clock... */

      ToneTimer_ClockSelect( csi );
    }
    else
    {
      /* To be on the safe side, turn off all interrupts */
      ToneTimer_InterruptsOff();

      /* Clock is stopped.  Counter is zero.  The only thing left to do is turn off the output. */
      digitalWrite( _pin, 0 );
    }
  }
}


void noTone( uint8_t _pin )
{
  if ( (tone_pin != 255) 
        && ((tone_pin == _pin) || (_pin == 255)) )
  {
    // Turn off all interrupts
    ToneTimer_InterruptsOff();

    // Stop the clock while we make changes.
    ToneTimer_ClockSelect( ToneTimer_(Stopped) );

    // Set the Tone Timer exactly the same as init did...
    initToneTimer();

//rmv    ToneTimer_SetToPowerup();

  /* rmv
	  // put timer 1 in 8-bit phase correct pwm mode
	  TCCR1A = (0<<COM1A1)|(0<<COM1A0) | (0<<COM1B1)|(0<<COM1B0) | (0<<WGM11)|(1<<WGM10);

    // set timer 1 prescale factor to 64
    // and start the timer
    TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13)|(0<<WGM12) | (0<<CS12)|(1<<CS11)|(1<<CS10);
  */

    // Set the output low
    if ( tone_timer_pin_register != NULL )
    {
      *tone_timer_pin_register &= ~(tone_timer_pin_mask);
    }
    else
    {
      digitalWrite( tone_pin, LOW );
    }
  
    tone_pin = 255;
  }
}


ISR( TONETIMER_COMPA_vect )
{
  if ( tone_timer_toggle_count != 0 )
  {
    if ( tone_timer_toggle_count > 0 )
    {
      --tone_timer_toggle_count;

      if ( tone_timer_toggle_count == 0 )
      {
        // Shutdown the hardware
        noTone( 255 );

        // Skip the rest.  We're finished.
        return;
      }
    }
    *tone_timer_pin_register ^= tone_timer_pin_mask;
  }
  else
  {
    // Shutdown the hardware
    noTone( 255 );
  }
}
