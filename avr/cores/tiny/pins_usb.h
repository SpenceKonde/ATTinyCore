/* ATTinyCore VUSB pins/etc configuration information. When converting a VUSB library
 * to ATTinyCore, edit the usbconfig.h and remove the "hardware config" and
 * "optional MCU description" sections. In place of the Hardware Config,
 * #include pins_usb.h - that will pull in this.
 *
 */

#ifndef PINS_USB_H
#define PINS_USB_H
/* ---------------------------- Hardware Config ---------------------------- */
#if   defined(__AVR_ATtiny44__)  || defined(__AVR_ATtiny84__) \
   || defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny841__)
   /* Seriously? These both exist? WHYYYYYY? D+ on PB1 is probably better
    * if you're gonna use an INT0 pin, might as well use it for the USB
    * not like it really matters though, since after the two USB pins, you're
    * only giving up a the PCINT on PB2? But ffs... why do both exist?
    * the configs making the rounds also often put it on PB2 for the
    * 84, with 1 for D-  - maybe was an 84-not-A with the crap internal
    * oscillator, and they used with external clock? Anyway, so much
    * for a nice clean set of configurations! I get to build 5 versions of
    * the binaries (no USB_TWOPLUS for 841 unless someone shows me hardware)
    * too, multiplied by 3-5 entry modes each, and for the
    * 841, there's 12 MHz and 16 MHz too, since the internal osc is
    * really able to hit 16 MHz if you crank it to the max. Can't wait!
    */
  #define USB_CFG_IOPORTNAME      B
  #if defined(USB_ZEROPLUS)
    #define USB_CFG_DMINUS_BIT    1
    #define USB_CFG_DPLUS_BIT     0
  #elif defined(USB_TWOPLUS)
    #define USB_CFG_DMINUS_BIT    1
    #define USB_CFG_DPLUS_BIT     2
  #else
    #define USB_CFG_DMINUS_BIT    0
    #define USB_CFG_DPLUS_BIT     1
  #endif
  #define USB_INTR_CFG            PCMSK1
  #define USB_INTR_CFG_SET        (1 << USB_CFG_DPLUS_BIT)
  #define USB_INTR_CFG_CLR        0
  #define USB_INTR_ENABLE         GIMSK
  #define USB_INTR_ENABLE_BIT     PCIE1
  #define USB_INTR_PENDING        GIFR
  #define USB_INTR_PENDING_BIT    PCIF1
  #define USB_INTR_VECTOR         PCINT1_vect


#elif defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #define USB_CFG_IOPORTNAME      B
  #define USB_CFG_DMINUS_BIT      3
  #define USB_CFG_DPLUS_BIT       4
  #define USB_INTR_CFG            PCMSK
  #define USB_INTR_CFG_SET        (1 << USB_CFG_DPLUS_BIT)
  #define USB_INTR_ENABLE_BIT     PCIE
  #define USB_INTR_PENDING_BIT    PCIF
  #define USB_INTR_VECTOR         PCINT0_vect

#elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  #define USB_CFG_IOPORTNAME      B
  #define USB_CFG_DMINUS_BIT      3
  #define USB_CFG_DPLUS_BIT       6
  /* WHY IS THIS A PCINT?!?! D+ IS ON INT0! */
  #define USB_INTR_CFG            PCMSK1
  #define USB_INTR_CFG_SET        (1 << USB_CFG_DPLUS_BIT)
  #define USB_INTR_CFG_CLR        0
  #define USB_INTR_ENABLE         PCICR
  #define USB_INTR_ENABLE_BIT     PCIE1
  #define USB_INTR_PENDING        PCIFR
  #define USB_INTR_PENDING_BIT    PCIF1
  #define USB_INTR_VECTOR         PCINT1_vect


#elif defined(__AVR_ATtiny461__) || defined(__AVR_ATtiny861__)
  #define USB_CFG_IOPORTNAME     B
  // #define USB_CFG_DMINUS_BIT     5
  #define USB_CFG_DMINUS_BIT     4
  #define USB_CFG_DPLUS_BIT      6
  #define USB_INTR_CFG           MCUCR
  #define USB_INTR_CFG_SET       (1) /* change */
  #define USB_INTR_CFG_CLR       0
  #define USB_INTR_ENABLE        GIMSK
  #define USB_INTR_ENABLE_BIT    (1 << 6) /* INT0 */
  #define USB_INTR_PENDING       GIFR
  #define USB_INTR_PENDING_BIT   (1 << 6) /* INTF0 */
  #define USB_INTR_VECTOR        INT0_vect


#elif defined(__AVR_ATtiny48__) || defined(__AVR_ATtiny88__)
  #define USB_CFG_IOPORTNAME     D
  #define USB_CFG_DMINUS_BIT     1
  #define USB_CFG_DPLUS_BIT      2 /* INT0 */
  #define USB_INTR_CFG           EICRA
  #define USB_INTR_CFG_SET       (1) /*ISC01 = 0, ISC00 = 1: Change */
  #define USB_INTR_CFG_CLR       0
  #define USB_INTR_ENABLE        EIMSK
  #define USB_INTR_ENABLE_BIT    (1) /* INT0 */
  #define USB_INTR_PENDING       EIFR
  #define USB_INTR_PENDING_BIT   (1) /* INTF0 */
  #define USB_INTR_VECTOR        INT0_vect

#elif defined(__AVR_ATtiny1634__)
  #define USB_CFG_IOPORTNAME    C
  #define USB_CFG_DMINUS_BIT    5
  #define USB_CFG_DPLUS_BIT     4
  #define USB_INTR_CFG          PCMSK2
  #define USB_INTR_CFG_SET      (1 << USB_CFG_DPLUS_BIT)
  #define USB_INTR_CFG_CLR      0
  #define USB_INTR_ENABLE       GIMSK
  #define USB_INTR_ENABLE_BIT   PCIE2
  #define USB_INTR_PENDING      GIFR
  #define USB_INTR_PENDING_BIT  PCIF2
  #define USB_INTR_VECTOR       PCINT2_vect

#else
  #error "ATTinyCore does not support USB on this device."
#endif

#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)
/* Clock rate of the AVR in kHz. Legal values are 12000, 12800, 15000, 16000,
 * 16500 and 20000. The 12.8 MHz and 16.5 MHz versions of the code require no
 * crystal, they tolerate +/- 1% deviation from the nominal frequency. All
 * other rates require a precision of 2000 ppm and thus a crystal!
 * Default if not specified: 12 MHz
 */
#define USB_CFG_CHECK_CRC       0
/* Define this to 1 if you want that the driver checks integrity of incoming
 * data packets (CRC checks). CRC checks cost quite a bit of code size and are
 * currently only available for 18 MHz crystal clock. You must choose
 * USB_CFG_CLOCK_KHZ = 18000 if you enable this option.
 */

#endif
