// BigNumber test: multiplication
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
  Serial.println ();
  BigNumber::begin ();  // initialize library

  // test multiplication  
  BigNumber a = "564328376";
  BigNumber b = "18254546";
  BigNumber c = a * b;
  
  printBignum (c);

}  // end of setup

void loop () { }