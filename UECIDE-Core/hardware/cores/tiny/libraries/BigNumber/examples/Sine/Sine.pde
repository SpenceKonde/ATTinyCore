// BigNumber test: calculate sines
#include "BigNumber.h"

// function to display a big number and free it afterwards
void printBignum (BigNumber n)
{
  char * s = n.toString ();
  Serial.println (s);
  free (s);
}  // end of printBignum

// calculate sine of x with 'precision' iterations
BigNumber sine (const BigNumber x, BigNumber precision)
{
  BigNumber val = 1;
  const BigNumber one = 1, two = 2;
  while (precision > 0)
  {
    val = one - val * x * x / (two * precision) / (two * precision + one);
    precision--;
  }
  val = x * val;
  return val;
} // end of function sine

void setup ()
{
  Serial.begin (115200);
  Serial.println ();
  Serial.println ();

  BigNumber::begin (50);  

  BigNumber E  ("2.7182818284590452353602874713526624977572470936999595749669676277240766303535");
  BigNumber pi ("3.1415926535897932384626433832795028841971693993751058209749445923078164062862");

  for (int deg = 0; deg <= 90; deg++)
  {
    Serial.print ("sin(");
    Serial.print (deg);
    Serial.print (") = ");
    BigNumber rad (deg);
    rad *= pi / BigNumber (180);  
    printBignum (sine (rad, 30));  
  }  // end of for loop

} // end of setup

void loop () { }


