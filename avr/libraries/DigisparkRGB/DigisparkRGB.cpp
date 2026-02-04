#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "DigisparkRGB.h"
#include "Arduino.h"

#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)

#define BLUE_CLEAR (pinlevelB &= ~(1 << BLUE)) // map BLUE to PB2
#define GREEN_CLEAR (pinlevelB &= ~(1 << GREEN)) // map BLUE to PB2
#define RED_CLEAR (pinlevelB &= ~(1 << RED)) // map BLUE to PB2
#define PORTB_MASK  (1 << PB0) | (1 << PB1) | (1 << PB2)
#define BLUE PB2
#define GREEN PB1
#define RED PB0


unsigned char DigisparkPWMcompare[3]; 
volatile unsigned char DigisparkPWMcompbuff[3];

void DigisparkRGBBegin() {                

  pinMode(2, OUTPUT); 
  pinMode(1, OUTPUT); 
  pinMode(0, OUTPUT); 
  //CLKPR = (1 << CLKPCE);        // enable clock prescaler update
  //CLKPR = 0;                    // set clock to maximum (= crystal)


  DigisparkPWMcompare[0] = 0x00;// set default PWM values
  DigisparkPWMcompare[1] = 0x00;// set default PWM values
  DigisparkPWMcompare[2] = 0x00;// set default PWM values
  DigisparkPWMcompbuff[0] = 0x00;// set default PWM values
  DigisparkPWMcompbuff[1] = 0x00;// set default PWM values
  DigisparkPWMcompbuff[2] = 0x00;// set default PWM values
  
#ifdef TIFR
  TIFR = (1 << TOV0);           // clear interrupt flag
#endif
#ifdef TIFR0
  TIFR0 = (1 << TOV0);           // clear interrupt flag
#endif
  #ifdef TIMSK
  TIMSK = (1 << TOV0);           // enable overflow interrupt
#endif
#ifdef TIMSK0
  TIMSK0 = (1 << TOV0);           // enable overflow interrupt
#endif
#ifdef TCCR0B
  TCCR0B = (1 << CS00);         // start timer, no prescale
#endif


  sei(); 
}

void DigisparkRGB(int pin,int value){
	DigisparkPWMcompbuff[pin] = value;
}

void DigisparkRGBDelay(int ms) {
  while (ms) {
    _delay_ms(1);
    ms--;
  }
}

ISR (TIM0_OVF_vect) {
  static unsigned char pinlevelB=PORTB_MASK;
  static unsigned char softcount=0xFF;

  PORTB = pinlevelB;            // update outputs
  
  if(++softcount == 0){         // increment modulo 256 counter and update
                                // the compare values only when counter = 0.
    DigisparkPWMcompare[0] = DigisparkPWMcompbuff[0]; // verbose code for speed
    DigisparkPWMcompare[1] = DigisparkPWMcompbuff[1]; // verbose code for speed
    DigisparkPWMcompare[2] = DigisparkPWMcompbuff[2]; // verbose code for speed

    pinlevelB = PORTB_MASK;     // set all port pins high
  }
  // clear port pin on compare match (executed on next interrupt)
  if(DigisparkPWMcompare[0] == softcount) RED_CLEAR;
  if(DigisparkPWMcompare[1] == softcount) GREEN_CLEAR;
  if(DigisparkPWMcompare[2] == softcount) BLUE_CLEAR;
}

