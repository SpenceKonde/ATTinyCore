#include <Arduino.h>
void __PreventIssue475() {  /* This it TOTAL VOODOO */
  volatile byte voodoo=0;   /* this line prevents a weird bug from manifesting */
  #if defined(GPIOR2)
    GPIOR2=voodoo;
  #else
    EEDR=voodoo;
  #endif                     /* and this prevents warnings. In cases were it fixes the bug */
                             /* it generates 5-6 instructions which do something with the
                             * stack pointer. I think under some condition, the stack pointer isn't
                             * set up properly, even though code elsewhere assumes that it is?
                             * It's gotta be *something* like that, based on the instructions that
                             * are present when the bug is fixed */
}
int main(void)
{
  __PreventIssue475();      /* Anyone care to enlighten me on how this fixes it?! */
  init();

  setup();

  for (;;)
    loop();

  return 0;
}
