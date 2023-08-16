# Internals and advanced functionality

You can identify the core using the following:

```c

#define ATTINY_CORE       - We try to define this as the version string as text. It usually works through the official IDE, otherwise it should have a placeholder string.

//New
#define ATTINY_CORE_MAJOR    (2)   // Major Version
#define ATTINY_CORE_MINOR    (0)   // Minor Version
#define ATTINY_CORE_PATCH    (0)   // Patch Version
#define ATTINY_CORE_RELEASED (0)   // 0 for development, 1  Version

```
## Identify basic features

```c

#define USE_SOFTWARE_SERIAL    //(0 = hardware serial, 1 = software serial
#define USE_SOFTWARE_SPI       //(not defined if hardware spi present)
#define HAVE_ADC               //(1 = has ADC functions)
#define DISABLE_UART           //(1 = disables HW serial buffers and interrupts)
#define DISABLE_MILLIS         //If defined, millis has been disabled.



```

## The following identify board variants (various cores have used both styles of defines, so both are provided here to maximize compatibility)

```c
#define ATTINYX4 1
#define __AVR_ATtinyX4__

#define ATTINYX5 1
#define __AVR_ATtinyX5__

#define ATTINYX61 1
#define __AVR_ATtinyX61__

#define ATTINYX7 1
#define __AVR_ATtinyX7__

#define ATTINYX313 1
#define __AVR_ATtinyX313__

//no backwards compatibility options since no previously existing cores used the other convention.
#define __AVR_ATtinyX41__
#define __AVR_ATtiny1634__
#define __AVR_ATtiny828__
#define __AVR_ATtiny43u__
#define __AVR_ATtiny26__

```
