// BigNumber test: calculate natural logarithm
#include "BigNumber.h"

// function to display a big number and free it afterwards
void printBignum (BigNumber n)
{
  char * s = n.toString ();
  Serial.println (s);
  free (s);
}  // end of printBignum

BigNumber one (1);
BigNumber two (2);
BigNumber half ("0.5");

BigNumber ln (BigNumber x)
  {
   
  // if x <= 0.5 or >= 2 take the square root and double the result  
  if (x <= half || x >= two)
    {
    BigNumber e = ln (x.sqrt ());
    return e * two;
    }  // end of  x <= 0.5 or >= 2

  // some big numbers
  BigNumber a = (x - one) / (x + one);
  BigNumber e;
  BigNumber t = a;
  
  BigNumber i = one;
  BigNumber E;
  
  do
  { 
    E = e;
    e += t / i;
    t *= a * a;
    i += two;
  } while (e != E);

  return e * two;
  } // end of function ln

void setup ()
{
  Serial.begin (115200);
  Serial.println ();
  Serial.println ();
  BigNumber::begin (45);   // max around 45 on the Uno for ln (100)

  printBignum (ln (100)); 
  
} // end of setup

void loop () { }

