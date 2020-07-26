void setup() {
  #if (F_CPU==16000000L)
    #define TOPVAL 62499
    #define OCVAL 31249
    #define TIMEINT 500
  #elif (F_CPU==20000000L)
    #define TOPVAL 62499
    #define OCVAL 31249
    #define TIMEINT 400
  #elif (F_CPU==12800000L)
    #define TOPVAL 49999
    #define OCVAL 24999
    #define TIMEINT 500
  #elif (F_CPU==12000000L)
    #define TOPVAL 48749
    #define OCVAL 24374
    #define TIMEINT 520
  #elif (F_CPU==16500000L)
    #define TOPVAL 57749
    #define OCVAL 28874
    #define TIMEINT 448
  #elif (F_CPU==8000000L)
    #define TOPVAL 31249
    #define OCVAL 15624
    #define TIMEINT 500
  #else
    #error "not supported"
  #endif
  DDRB=0x1E;  //pins 9,10,11,12 OUTPUT
  DDRD=0x01;
  GTCCR=0x80;
  GTCCR=0x81;  //stop time
  TCCR1A=0xA2; //Toggle OC1A, normal mode OC1B.
  TCCR1B=0x1C; //WGM14 fast PWM, 1/256 prescale
  OCR1A=OCVAL; //transition once per second on A
  OCR1B=OCVAL; //1 Hz squarewave on B
  ICR1=TOPVAL;
  TCNT1=0;
  unsigned long currentmillis=millis();
  unsigned long currentmicros=micros();
  GTCCR=0; //start time
  while(1){ // transition once per second on pin 11
    //delay(500);
    PIND=0x01;
    if(millis()-currentmillis>TIMEINT){
      PINB=0x08;
      currentmillis+=TIMEINT;
    }
    if(micros()-currentmicros>(TIMEINT*1000UL)){
      PINB=0x10;
      currentmicros+=(TIMEINT*1000UL);
    }
  }
  
}



void loop() {
  // put your main code here, to run repeatedly:
  
}
