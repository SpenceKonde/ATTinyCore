//
//  BigNumber.h
//  
//
//  Author:  Nick Gammon
//  Date:    7th January 2012.
//  Version: 2.0
//  Released into the public domain.

#ifndef _BigNumber_h
#define _BigNumber_h

#include <stddef.h>

extern "C" 
{
 #include "number.h" 
}

class BigNumber 
{
  
  // the current scaling amount - shared amongst all BigNumbers
  static int scale_;

  // member variable (the big number)
  bc_num        num_;

public:
 
  // constructors
  BigNumber ();  // default constructor
  BigNumber (const char * s);   // constructor from string
  BigNumber (const int n);  // constructor from int
  // copy constructor
  BigNumber (const BigNumber & rhs); 
 
  // destructor  
  ~BigNumber ();
  
  // static methods: initialize package, and set global scaling factor
  static void begin (const int scale = 0);
  static void finish ();  // free memory used by 'begin' method
  static int setScale (const int scale = 0);
  
  // for outputting purposes ...
  char * toString () const;  // returns number as string, MUST FREE IT after use!
  operator long () const;

  // operators ... assignment
  BigNumber & operator= (const BigNumber & rhs);
  
  // operations on the number which change it (eg. a += 5; )
  BigNumber & operator+= (const BigNumber & n);
  BigNumber & operator-= (const BigNumber & n);
  BigNumber & operator/= (const BigNumber & n);
  BigNumber & operator*= (const BigNumber & n);
  BigNumber & operator%= (const BigNumber & n);  // modulo

  // operations on the number which do not change it (eg. a = b + 5; )
  BigNumber operator+ (const BigNumber & n) const { BigNumber temp = *this; temp += n; return temp; };
  BigNumber operator- (const BigNumber & n) const { BigNumber temp = *this; temp -= n; return temp; };
  BigNumber operator/ (const BigNumber & n) const { BigNumber temp = *this; temp /= n; return temp; };
  BigNumber operator* (const BigNumber & n) const { BigNumber temp = *this; temp *= n; return temp; };
  BigNumber operator% (const BigNumber & n) const { BigNumber temp = *this; temp %= n; return temp; };
  
  // prefix operations
  BigNumber & operator++ () { *this += 1; return *this; }
  BigNumber & operator-- () { *this -= 1; return *this; }
  
  // postfix operations (cannot return by reference)
  // we make a temporary object, change our current object, return the temporary one
  // if we returned a reference it would cease to exist, so we have to return a copy
  BigNumber operator++ (int) { BigNumber temp = *this; *this += 1; return temp; }
  BigNumber operator-- (int) { BigNumber temp = *this; *this -= 1; return temp; }
  
  // comparisons
  bool operator<  (const BigNumber & rhs) const;
  bool operator<  (const int rhs) const { return *this < BigNumber (rhs); }
  bool operator>  (const BigNumber & rhs) const;
  bool operator>  (const int rhs) const { return *this > BigNumber (rhs); }
  bool operator<= (const BigNumber & rhs) const;
  bool operator<= (const int rhs) const { return *this <= BigNumber (rhs); }
  bool operator>= (const BigNumber & rhs) const;
  bool operator>= (const int rhs) const { return *this >= BigNumber (rhs); }
  bool operator!= (const BigNumber & rhs) const;
  bool operator!= (const int rhs) const { return *this != BigNumber (rhs); }
  bool operator== (const BigNumber & rhs) const;
  bool operator== (const int rhs) const { return *this == BigNumber (rhs); }

  // quick sign test
  bool isNegative () const;
  // quick zero test
  bool isZero () const;
  
  // In some places we need to check if the number is almost zero.
  // Specifically, all but the last digit is 0 and the last digit is 1.
  // Last digit is defined by scale.
  bool isNearZero () const;
  
  // other mathematical operations
  BigNumber sqrt () const;
  BigNumber pow (const BigNumber power) const;
  // divide number by divisor, give quotient and remainder
  void divMod (const BigNumber divisor, BigNumber & quotient, BigNumber & remainder) const;
  
};  // end class declaration


#endif
