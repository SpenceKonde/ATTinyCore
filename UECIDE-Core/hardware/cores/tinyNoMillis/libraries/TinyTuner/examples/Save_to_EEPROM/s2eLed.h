#ifndef s2eLed_h
#define s2eLed_h

#include <core_build_options.h>


#if defined( __AVR_ATtiny2313__ )

  const uint8_t PinLED = 4;

#elif defined( __AVR_ATtinyX4__ )

  const uint8_t PinLED = 4;

#elif defined( __AVR_ATtinyX5__ )

  const uint8_t PinLED = 0;

#endif


#endif
