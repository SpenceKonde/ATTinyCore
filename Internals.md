### Internals and advanced functionality:




You can identify the core using the following:

```

#define ATTINY_CORE       - Attiny Core

```


These are used to identify features:

```

#define USE_SOFTWARE_SERIAL    (0 = hardware serial, 1 = software serial
#define USE_SOFTWARE_SPI       (not defined if hardware spi present)
#define HAVE_ADC               (1 = has ADC functions)
#define DISABLE_UART           (1 = disables HW serial buffers and interrupts)

```

The following identify board variants (various cores have used both styles of defines, so both are provided here to maximize compatibility):

```
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

```
