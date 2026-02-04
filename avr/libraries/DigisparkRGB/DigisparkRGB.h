/*
  DigisparkPWM.h - Library for pwm on PB2 on ATTINY85.
  Created by Digistump
  Released into the public domain.
*/
#ifndef DigisparkRGB_h
#define DigisparkRGB_h


void DigisparkRGBBegin();
void DigisparkRGBDelay(int ms);
void DigisparkRGB(int pin,int value);

#endif