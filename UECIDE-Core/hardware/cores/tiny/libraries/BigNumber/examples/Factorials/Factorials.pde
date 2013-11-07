// BigNumber test: factorials
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
  BigNumber::begin ();  // initialize library
 
  //factorials
  BigNumber fact = 1;

  for (int i = 2; i <= 200; i++)
  {
    Serial.print (i);
    Serial.print ("! = ");
    fact *= i;
    printBignum (fact);
  }

}  // end of setup

void loop () { }
