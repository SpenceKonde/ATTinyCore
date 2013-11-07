// BigNumber test: calculate e
#include "BigNumber.h"

// function to display a big number and free it afterwards
void printBignum (BigNumber & n)
{
  char * s = n.toString ();
  Serial.println (s);
  free (s);
}  // end of printBignum

void setup ()
{
  Serial.begin (115200);
  Serial.println ();

  BigNumber::begin (50);  // max around 160 on the Uno

  // some big numbers
  BigNumber n = 1, e = 1, one = 1;

  int i = 1;
  BigNumber E;  // previous result

  unsigned long start = millis ();
  do
  { 
    E = e;
    n *= i++;  // n is i factorial
    e += one / n;
  }  while (e != E);
  unsigned long time = millis () - start;

  printBignum (e);
  Serial.print (time);
  Serial.println (" mS");
} // end of setup

void loop () { }

