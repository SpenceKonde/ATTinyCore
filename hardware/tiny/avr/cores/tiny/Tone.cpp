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
0009	T Carpenter 12/08/06 Rewritten to remove requirement for all the wierd timer name creation macros.

*************************************************/

#include <avr/interrupt.h>
#include "Arduino.h"
#include "wiring_private.h"
#include "pins_arduino.h"

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
  if ( tone_pin == 255 )
  {
    /* Set the timer to power-up conditions so we start from a known state */
    // Ensure the timer is in the same state as power-up
    #if (TIMER_TO_USE_FOR_TONE == 0)
    TCCR0B = (0<<FOC0A) | (0<<FOC0B) | (0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
    TCCR0A = (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
    // Reset the count to zero
    TCNT0 = 0;
    // Set the output compare registers to zero
    OCR0A = 0;
    OCR0B = 0;
    #if defined(TIMSK)
    // Disable all Timer0 interrupts
    TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
    // Clear the Timer0 interrupt flags
    TIFR |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
    #elif defined(TIMSK1)
    // Disable all Timer0 interrupts
    TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
    // Clear the Timer0 interrupt flags
    TIFR0 |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
    #endif
      
      
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
    // Turn off Clear on Compare Match, turn off PWM A, disconnect the timer from the output pin, stop the clock
    TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10);
    // Turn off PWM A, disconnect the timer from the output pin, no Force Output Compare Match, no Prescaler Reset
    GTCCR &= ~((1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (1<<FOC1B) | (1<<FOC1A) | (1<<PSR1));
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0;
    // Disable all Timer1 interrupts
    TIMSK &= ~((1<<OCIE1A) | (1<<OCIE1B) | (1<<TOIE1));
    // Clear the Timer1 interrupt flags
    TIFR |= ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));
      
      
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TCCR1D = 0;
    TCCR1E = 0;
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    // Disable all Timer1 interrupts
    TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<OCIE1D));
    // Clear the Timer1 interrupt flags
    TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<OCF1D));
  
  
    #elif (TIMER_TO_USE_FOR_TONE == 1)
    // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
    TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
    // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
    TCCR1A = (0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    // Disable all Timer1 interrupts
    #if defined(TIMSK)
    TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
    // Clear the Timer1 interrupt flags
    TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
    #elif defined(TIMSK1)
    // Disable all Timer1 interrupts
    TIMSK1 &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
    // Clear the Timer1 interrupt flags
    TIFR1 |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
    #endif
      
    #endif

    /* 
      Compare Output Mode = Normal port operation, OCxA/OCxB disconnected.
      Waveform Generation Mode = 4; 0100; CTC; (Clear Timer on Compare); OCR1A; Immediate; MAX
      Clock Select = No clock source (Timer/Counter stopped).
      Note: Turn off the clock first to avoid ticks and scratches.
    */	
    #if TIMER_TO_USE_FOR_TONE == 1
	#if defined(TCCR1)
	sbi(TCCR1,CTC1);
    cbi(TCCR1,PWM1A);
    cbi(GTCCR,PWM1B);
	#elif !defined(TCCR1E)
	cbi(TCCR1A,WGM10);
	cbi(TCCR1A,WGM11);
	sbi(TCCR1B,WGM12);
	cbi(TCCR1B,WGM13);
	#endif
    #elif TIMER_TO_USE_FOR_TONE == 0
	cbi(TCCR0A,WGM00);
	sbi(TCCR0A,WGM01);
	cbi(TCCR0B,WGM02);
    #endif

    /* If the tone pin can be driven directly from the timer */

    #if (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
    if ( (digitalPinToTimer(_pin) == TIMER1A) || (digitalPinToTimer(_pin) == TIMER1B)  || (digitalPinToTimer(_pin) == TIMER1D) )
    {
    #elif (TIMER_TO_USE_FOR_TONE == 1)
    if ( (digitalPinToTimer(_pin) == TIMER1A) || (digitalPinToTimer(_pin) == TIMER1B) )
    {
    #elif (TIMER_TO_USE_FOR_TONE == 0)
    if ( (digitalPinToTimer(_pin) == TIMER0A) || (digitalPinToTimer(_pin) == TIMER0B) )
    {
    #else
	if (0)
	{ //unsupported, so only use software.
	#endif
      /* Pin toggling is handled by the hardware */
      tone_timer_pin_register = NULL;
      tone_timer_pin_mask = 0;
      uint8_t timer = digitalPinToTimer(_pin);
	  #if defined(COM0A1)
	  //Just in case there are now pwm pins on timer0 (ATTiny861)
      if (timer == TIMER0A)
      {
        /* Compare Output Mode = Toggle OC0A on Compare Match. */
		cbi(TCCR0A,COM0A1);
		sbi(TCCR0A,COM0A0);
      }
      else
	  #endif
	  if (timer == TIMER1A)
      {
        /* Compare Output Mode = Toggle OC1A on Compare Match. */
		#if defined(TCCR1)
		cbi(TCCR1,COM1A1);
		sbi(TCCR1,COM1A0);
		#elif defined(TCCR1E)
		cbi(TCCR1C,COM1A1S);
		sbi(TCCR1C,COM1A0S);
		#else
		cbi(TCCR1A,COM1A1);
		sbi(TCCR1A,COM1A0);
		#endif
      }
	  #if defined(COM0B1)
	  //Just in case there are <2 pwm pins on timer0 (ATTiny861)
      else if (timer == TIMER0B)
      {
        /* Compare Output Mode = Toggle OC0B on Compare Match. */
		cbi(TCCR0A,COM0B1);
		sbi(TCCR0A,COM0B0);
      }
	  #endif
	  #if defined(COM1D1)
	  //in case there is a OCRD. (ATtiny861)
	  else if (timer == TIMER1D){
        /* Compare Output Mode = Toggle OC1D on Compare Match. */
		#if defined(TCCR1)
		cbi(TCCR1,COM1D1);
		sbi(TCCR1,COM1D0);
		#elif defined(TCCR1E)
		cbi(TCCR1C,COM1D1);
		sbi(TCCR1C,COM1D0);
		#else
		cbi(TCCR1A,COM1D1);
		sbi(TCCR1A,COM1D0);
		#endif
	  }
	  #endif
      else
      {
        /* Compare Output Mode = Toggle OC1B on Compare Match. */
		#if defined(TCCR1)
		cbi(GTCCR,COM1B1);
		sbi(GTCCR,COM1B0);
		#elif defined(TCCR1E)
		cbi(TCCR1C,COM1B1S);
		sbi(TCCR1C,COM1B0S);
		#else
		cbi(TCCR1A,COM1B1);
		sbi(TCCR1A,COM1B0);
		#endif
      }
    }
    else
    {
      /* Save information needed by the interrupt service routine */
      tone_timer_pin_register = portOutputRegister( digitalPinToPort( _pin ) );
      tone_timer_pin_mask = digitalPinToBitMask( _pin );

      /* Compare Output Mode = Normal port operation, OCxA disconnected. */
    #if (TIMER_TO_USE_FOR_TONE == 0)
      TCCR0A &= ~((1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0));
    #elif (TIMER_TO_USE_FOR_TONE == 1) & defined(TCCR1)
      TCCR1 &= ~((1<<COM1A1)|(1<<COM1A0));
      GTCCR &= ~((1<<COM1B1)|(1<<COM1B0));
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
      TCCR1C &= ~((1<<COM1A1S)|(1<<COM1A0S)|(1<<COM1B1S)|(1<<COM1B0S)|(1<<COM1D1)|(1<<COM1D0));
	#elif (TIMER_TO_USE_FOR_TONE == 1)
      TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B0));
	#endif
    }

    /* Ensure the pin is configured for output */
    pinMode( _pin, OUTPUT );

    tone_pin = _pin;
  }

  if ( tone_pin == _pin )
  {
    /* Stop the clock while we make changes, then set the counter to zero to reduce ticks and scratches. */

    // Millis timer is always processor clock divided by MillisTimer_Prescale_Value (64)
    #if (TIMER_TO_USE_FOR_TONE == 0)
    TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
    TCNT0 = 0;
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
    TCCR1 &= ~((1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10));
    TCNT1 = 0;
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
    TCCR1B &= ~((1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10));
    TCNT1 = 0;
    #elif (TIMER_TO_USE_FOR_TONE == 1)
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
    TCNT1 = 0;
    #endif

    if ( frequency > 0 )
    {
      /* Determine which prescaler to use */
      /* Set the Output Compare Register (rounding up) */
      
      #if TIMER_TO_USE_FOR_TONE == 1
	  uint16_t ocr = F_CPU / frequency / 2;
	  #if defined(TCCR1E)
      uint8_t prescalarbits = 0b0001;
      if (ocr > 256)
      {
        ocr >>= 3; //divide by 8
        prescalarbits = 0b0100;  // ck/8
        if (ocr > 256)
        {
          ocr >>= 3; //divide by a further 8
          prescalarbits = 0b0111; //ck/64
          if (ocr > 256)
          {
            ocr >>= 2; //divide by a further 4
            prescalarbits = 0b1001; //ck/256
            if (ocr > 256)
            {
              // can't do any better than /1024
              ocr >>= 2; //divide by a further 4
              prescalarbits = 0b1011; //ck/1024
            }
          }
        }
      }
	  #else
	  #if defined(TCCR1)
      uint8_t prescalarbits = 0b0001;
	  #else
      uint8_t prescalarbits = 0b001;
      #endif
	  if (ocr > 0xffff)
      {
        ocr /= 64;
	    #if defined(TCCR1)
        prescalarbits = 0b0111;
	    #else
        prescalarbits = 0b011;
        #endif
      }
      #endif
	  ocr -= 1; //Note we are doing the subtraction of 1 here to save repeatedly calculating ocr from just the frequency in the if tree above 
      OCR1A = ocr;
	  
	  #elif TIMER_TO_USE_FOR_TONE == 0
      uint16_t ocr = F_CPU / frequency / 2;
      uint8_t prescalarbits = 0b001;  // ck/1
      if (ocr > 256)
      {
        ocr >>= 3; //divide by 8
        prescalarbits = 0b010;  // ck/8
        if (ocr > 256)
        {
          ocr >>= 3; //divide by a further 8
          prescalarbits = 0b011; //ck/64
          if (ocr > 256)
          {
            ocr >>= 2; //divide by a further 4
            prescalarbits = 0b100; //ck/256
            if (ocr > 256)
            {
              // can't do any better than /1024
              ocr >>= 2; //divide by a further 4
              prescalarbits = 0b101; //ck/1024
            }
          }
        }
      }
	  ocr -= 1; //Note we are doing the subtraction of 1 here to save repeatedly calculating ocr from just the frequency in the if tree above 
      OCR0A = ocr;
      #endif

      /* Does the caller want a specific duration? */
      if ( duration > 0 )
      {
        /* Determine how many times the value toggles */
        tone_timer_toggle_count = (2 * frequency * duration) / 1000;
        /* Output Compare A Match Interrupt Enable */
        #if (TIMER_TO_USE_FOR_TONE == 1)
        #if defined (TIMSK)
        TIMSK |= (1<<OCIE1A);
        #else
        TIMSK1 |= (1<<OCIE1A);
        #endif
        #elif (TIMER_TO_USE_FOR_TONE == 0)
        #if defined (TIMSK)
        TIMSK |= (1<<OCIE0A);
        #else
        TIMSK0 |= (1<<OCIE0A);
        #endif
        #endif
      }
      else
      {
        /* Indicate to the interrupt service routine that we'll be running until further notice */
        tone_timer_toggle_count = -1;

        /* All pins but the OCxA / OCxB pins have to be driven by software */
        #if (TIMER_TO_USE_FOR_TONE == 1)
		#if defined(TCCR1E)
        if ( (digitalPinToTimer(_pin) != TIMER1A) && (digitalPinToTimer(_pin) != TIMER1B) && (digitalPinToTimer(_pin) != TIMER1D) )
		#else
        if ( (digitalPinToTimer(_pin) != TIMER1A) && (digitalPinToTimer(_pin) != TIMER1B) )
        #endif
		{
            /* Output Compare A Match Interrupt Enable (software control)*/
            #if defined (TIMSK)
            TIMSK |= (1<<OCIE1A);
            #else
            TIMSK1 |= (1<<OCIE1A);
            #endif
        }
        #elif (TIMER_TO_USE_FOR_TONE == 0)
        if ( (digitalPinToTimer(_pin) != TIMER0A) && (digitalPinToTimer(_pin) != TIMER0B) )
        {
            /* Output Compare A Match Interrupt Enable (software control)*/
            #if defined (TIMSK)
            TIMSK |= (1<<OCIE0A);
            #else
            TIMSK0 |= (1<<OCIE0A);
            #endif
        }
        #endif
      }
	  
	  //Clock is always stopped before this point, which means all of CS[0..2] are already 0, so can just use a bitwise OR to set required bits
      #if (TIMER_TO_USE_FOR_TONE == 0)
      TCCR0B |= (prescalarbits << CS00);
      #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
      TCCR1 |= (prescalarbits << CS10);
      #elif (TIMER_TO_USE_FOR_TONE == 1)
      TCCR1B |= (prescalarbits << CS10);
      #endif
	  
    }
    else
    {
      /* To be on the safe side, turn off all interrupts */
      #if (TIMER_TO_USE_FOR_TONE == 1)
      #if defined (TIMSK)
      TIMSK |= (1<<OCIE1A);
      TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B));
      #if defined(ICIE1)
      TIMSK &= ~(1<<ICIE1);
      #endif
	  #if defined(OCIE1D)
      TIMSK &= ~(1<<OCIE1D);
	  #endif
      #else
      TIMSK1 |= (1<<OCIE1A);
      TIMSK1 &= ~((1<<ICIE1) | (1<<OCIE1B) | (1<<OCIE1A) | (1<<TOIE1));
      #endif
      #elif (TIMER_TO_USE_FOR_TONE == 0)
      #if defined (TIMSK)
      TIMSK |= (1<<OCIE0A);
      TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
      #else
      TIMSK0 |= (1<<OCIE0A);
      TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
      #endif
      #endif

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
    #if (TIMER_TO_USE_FOR_TONE == 1)
    #if defined (TIMSK)
    TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B));
    #if defined(ICIE1)
    TIMSK &= ~(1<<ICIE1);
    #endif
    #if defined(OCIE1D)
    TIMSK &= ~(1<<OCIE1D);
    #endif
    #else
    TIMSK1 &= ~((1<<ICIE1) | (1<<OCIE1B) | (1<<OCIE1A) | (1<<TOIE1));
    #endif
    #elif (TIMER_TO_USE_FOR_TONE == 0)
    #if defined (TIMSK)
    TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
    #else
    TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
    #endif
    #endif

  
    // This just disables the tone. It doesn't reinitialise the PWM modules.
    #if (TIMER_TO_USE_FOR_TONE == 0)
    TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00)); //stop the clock
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
    TCCR1 &= ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
    TCCR1B &= ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock
    #elif (TIMER_TO_USE_FOR_TONE == 1)
    TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock
    #endif

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


#if (TIMER_TO_USE_FOR_TONE == 0)
ISR(TIMER0_COMPA_vect)
#elif (TIMER_TO_USE_FOR_TONE == 1)
ISR(TIMER1_COMPA_vect)
#else
#error Tone timer Overflow vector not defined!
#endif
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
