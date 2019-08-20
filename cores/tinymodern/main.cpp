#include <WProgram.h>
#if defined( __AVR_ATtinyX41__ )

#ifdef OLD_PINOUT

#warning "This is the COUNTERCLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in counter clockwise order"
#else

#warning "This is the CLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in clockwise order"
#endif
#endif
int main(void)
{
  init();

  setup();

  for (;;)
    loop();

  return 0;
}
