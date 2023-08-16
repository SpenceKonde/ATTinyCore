# ATtiny828(R)
![828 Pin Mapping](http://drazzy.com/e/img/PinoutT828.jpg "Arduino Pin Mapping for ATtiny828")

Specification         |    ATtiny828   |    ATtiny828   |
----------------------|----------------|----------------|
Bootloader (if any)   |           None |       Optiboot |
Uploading uses        |   ISP/SPI pins | Serial Adapter |
Flash available user  |     8192 bytes |     7680 bytes |
RAM                   |      512 bytes |      512 bytes |
EEPROM                |      256 bytes |      256 bytes |
GPIO Pins             |     27 + RESET |     27 + RESET |
ADC Channels          |             28 |             28 |
PWM Channels (pins)   |          4 (8) |          4 (8) |
Interfaces            | SPI, I2C slave | SPI, I2C slave |
(contd.)              |          USART |          USART |
Clocking Options      |         in MHz |         in MHz |
Int. Oscillator       |     8, 4, 2, 1 |     8, 4, 2, 1 |
Int. WDT Oscillator   |  Not supported |  Not supported |
Internal, with tuning |    8, 12, 12.8 |    8, 12, 12.8 |
External Crystal      |  Not supported |  Not supported |
External Clock        |   All Standard |   All Standard |


The ATtiny828 is a pretty sorry piece of hardware - it clearly had it's crown jewel, an advanced differential ADC, stolen from it at the last minute, It's got no hardware I2C master, the hardware I2C slave requires the WDT to be enabled because of that nasty silicon bug, the internal oscillator has the strong voltage dependence, it doesn't support a crystal... The sad fact is that unless you need a huge number of single-ended ADC inputs and a hardware serial port, this chip just straight up sucks. Try to avoid using these - unless you're going on a silicon-archeology expedition to searching for the Lost ADC of '828 ([solve the mystery and get free stuff](https://github.com/SpenceKonde/AVR_Research/blob/main/UnsolvedMysteries.md#the-828-and-the-mystery-of-the-adc))

## Warning: Pin 27 (PD3) does not work as an input unless watchdog timer is running
This is a design flaw in the chip, as noted in the datasheet errata. Additionally, when the "ULP" oscillator (used by the WDT, among other things) is not running, it is "internally pulled down"; phrased more pessimistically "if pin is output and high, it will continually draw current even without an external load". Definitely don't try to use power-saving sleep mode with PD3 set OUTPUT and HIGH.
See code for "workaround" below - but the pin is still less useful than it should be; it is best limited to active output while the chip is awake (such as via it's PWM capability). Unless of course you want to use the ULP/WDT.... Worse still, that's the clock pin for the USI, hence I2C slave and SPI generally. I2C, being an open drain bus, will not function when the WDT is disabled because of this unwanted pulldown effect. It really is a crying shame how the 828 and 1634 were denied the fix they deserved - a fix for the pin errata. Note that, partly for this reason, when TWI is used in master-only mode (which is a software I2C master implementation) I2C is on PA4 and PA5, instead of PORTD, and is hence uneffected by this bug.

## Programming
Any of these parts can be programmed by use of any ISP programmer. Optiboot is supported for uploading over serial. Be sure to read the section of the main readme on the ISP programmers and IDE versions. 1.8.13 is recommended for best results.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny828, operating on the hardware UART at the standard ATTinyCore baud rates (which have changed in 2.0.0 for improved reliability see [the Optboot reference](./Ref_Optiboot.md). The bootloader uses 512b of space, leaving 7680b available for user code. Unlike the other ATtiny parts supported by this core, the 828 has hardware bootloader support!

### No Micronucleus support is provided
These chips do not warrant the development effort to make that happen.

### There is no external crystal support, only external clock

### Internal Oscillator voltage dependence
Prior to 1.4.0, many users had encountered issues due to the voltage dependence of the oscillator. While the calibration is very accurate between 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Internal, Vcc < 4.5v) and 8 MHz (Internal, Vcc > 4.5 MHz ) board definitions - it should generally work between 2.7v and 5.25v - though the extremes of that range may be dicey. The new baud rate changes in 2.0.0 should further improve bootloader reliability here (see the Optiboot reference linked above).

We do still provide a >4.5v clock option in order to improve behavior of the running sketch - it will nudge the oscillator calibration down to move it closer to the nominal 8MHz clock speed; sketches uploaded with the higher voltage option. This is not perfect, but it is generally good enough to work with Serial on around 5v (including 5.25v often found on USB ports to facilitate chargeing powerhungry devices), and millis()/micros() will keep better time than in previous versions.

The internal oscillator is factory calibrated to +/- 10% or +/- 2% for the slightly more expensive 828R. +/- 2% is good enough for serial communication. However, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work absent the above-described countermeasures.

### PWM frequency
TC0 is always run in Fast PWM mode: We use TC0 for millis, and phase correct mode can't be used on the millis timer - you need to read the count to get micros, but that doesn't tell you the time in phase correct mode because you don't know if it's upcounting or downcounting in phase correct mode.

| F_CPU  | F_PWM<sub>TC0</sub> | F_PWM<sub>TC1</sub>   | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz | 1/8/256=     488 Hz |  1/8/256=      488 Hz |                              |
| 2  MHz | 2/8/256=     977 Hz |  2/8/256=      977 Hz |                              |
| <4 MHz | x/8/256= 488 * x Hz |  x/8/512=  244 * x Hz | Phase correct TC1            |
| 4  MHz | 4/8/256=    1960 Hz |  4/8/512=      977 Hz | Phase correct TC1            |
| <8 MHz | x/64/256= 61 * x Hz |  x/8/512=  244 * x Hz | Between 4 and 8 MHz, the target range is elusive | Phase correct TC1 |
| 8  MHz | 8/64/256=    488 Hz |  8/64/256=     488 Hz |                              |
| >8 MHz | x/64/256= 61 * x Hz |  x/64/256=  61 * x Hz |                              |
| 12 MHz | 12/64/256=   735 Hz | 12/64/256=     735 Hz |                              |
| 16 MHz | 16/64/256=   977 Hz | 16/64/256=     977 Hz |                              |
|>16 MHz | x/64/256= 61 * x Hz |  x/64/512=  31 * x Hz | Phase correct TC1            |
| 20 MHz | 20/64/256=  1220 Hz | 20/64/512=     610 Hz | Phase correct TC1            |

Phase correct PWM counts up to 255, turning the pin off as it passes the compare value, updates it's double-buffered registers at TOP, then it counts down to 0, flipping the pin back as is passes the compare value. This is considered preferable for motor control applications, though the "Phase and Frequency Correct" mode is better if the period is ever adjusted by a large amount at a time, because it updates the doublebuffered registers at BOTTOM, and thus produces a less problematic glitch in the duty cycle, but doesn't have any modes that don't require setting ICR1 too.

For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### Tone Support
Tone() uses Timer1. For best results, use pin 21 or 22 (PIN_PC5, PIN_PC6), as this will use the hardware output compare to generate the square wave instead of using interrupts. Any use of tone() will disable PWM on pins 21 and 22.

### Servo Support
The standard Servo library is hardcoded to work on specific parts only, we include a builtin Servo library that supports the Tiny x8 series. As always, while a software serial port (including the builtin one, Serial, on these ports, see below) is receiving or transmitting, the servo signal will glitch. See [the Servo/Servo_ATTinyCore library](../libraries/Servo/README.md) for more details. Like tone(), this will disable PWM on PC5 and PC6 . Tone and Servo cannot be used at the same time.

### I2C Support
Slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. This is done automatically with the included version of the Wire.h library. **You must have external pullup resistors installed** in order for I2C functionality to work reliably. Furthermore, the slave functionality requires the WDT to be enabled, otherwise the SCL pin will be pulled low due to a silicon bug. Slave and master I2C use different pins because of this godawful bug. We only support use of the builtin universal Wire.h library. If you try to use other libraries and encounter issues, please contact the author or maintainer of that library - there are too many of these poorly written libraries for us to provide technical support for.

### SPI Support
There is full Hardware SPI support. However, PD3 is one of the pins used by the hardware SPI; you must use the WDT workaround for the PD3 silicon bug if using SPI. Third party SPI libraries designed for tinyAVRs are not supported by the hardware and will not work.

### UART (Serial) Support
There is one hardware serial port, Serial. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels
```c
UCSRB &=~(1<<TXEN); // disable TX
UCSRB &=~(1<<RXEN); // disable RX

```

### ADC support
There is abundant evidence that this device was intended to have a differential ADC; the register layout matches that of the ATtiny841 which has a vere nice differential ADC, except that all the register bits that would be involved in that are marked reserved. If that wasn't enough to convince you, reading that chapter of the datasheet, it is clear that references to a differential ADC were scrubbed in a hurry at the last minute. I suspect it was found to be afflicted by a fatal flaw, whose workaround if any was to onerouos or the nature of the mistake too humiliating to present to customers, managemnt denied the request for a respin to fix it, and they responded by removing it on paper only - and is still actually in the silicon (if they had time to respin, they'd have fixed it, I suspect).  See the link at the top of this page - if you like poking at "reserved" registers and trying to find secret features, and have time on your hands, it could be a lot of fun, and if you solve the mystery, I'll mail you some free boards of your choice from my store.

#### ADC Reference options
Despite having 28 ADC input channels, the 828 only has the two basic reference options.

* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Weird I/O-pin related features
There are a few strange features relating to the GPIO pins on the ATtinyx41 family which are found only in a small number of other parts released around the same time.

#### PD3 silicon errata
As mentioned above, the t828 has a serious silicon bug PD3, made all the worse by the important alternative functions of that pin.

If you have no need to use the WDT, but do have a need to use PD3 as an input, you can keep the WDT running by putting it into interrupt mode, with an empty interrupt, at the cost of just 10b of flash, an ISR that executes in 11 clock cycles every 8 seconds, and an extra 1-4uA of power consumption (negligible compared to what the chip consumes when not sleeping, and you'll turn it off while sleeping anyway - see below) - so the real impact of this issue is in fact very low, assuming you know about it and don't waste hours or days trying to figure out what is going on.

```c
//put these lines in setup
CCP=0xD8; //write key to configuration change protection register
WDTCSR=(1<<WDP3)|(1<<WDP0)|(1<<WDIE); //enable WDT interrupt with longest prescale option (8 seconds)
//put this empty WDT ISR outside of all functions
EMPTY_INTERRUPT(WDT_vect) //empty ISR to work around bug with PB3. EMPTY_INTERRUPT uses 26 bytes less than ISR(WDT_vect){;}
```
If you are using sleep modes, you also need to turn the WDT off while sleeping (both because the interrupts would wake it, and because the WDT is consuming power, and presumably that's what you're trying to avoid by sleeping). Doing so as shown below only uses an extra 12-16 bytes if you call it from a single place, 20 if called from two places, and 2 bytes when you call it thereafter, compared to calling sleep_cpu() directly in those places, as you would on a part that didn't need this workaround.
```c
void startSleep() { //call instead of sleep_cpu()
  CCP=0xD8; //write key to configuration change protection register
  WDTCSR=0; //disable WDT interrupt
  sleep_cpu();
  CCP=0xD8; //write key to configuration change protection register
  WDTCSR=(1<<WDP3)|(1<<WDP0)|(1<<WDIE); //enable WDT interrupt
}
```


#### Special "high sink" port
All pins on PORTC have unusually high sink capability - when sinking a given amount of current, the voltage on these pins is about half that of typical pins. Using the `PHDE` register, these can be set to sink even more aggressively.

```c
PHDE=(1<<PHDEC);
```

This is no great shakes - the Absolute Maximum current rating of 40mA still applies and all... but it does pull closer to ground with a a "large" 10-20mA load. A very strange feature of these parts. The PWM outputs of go on this this port as well, making it of obvious utility for driving LEDs and similar. This also means that, if you are attempting to generate an analog voltage with a PWM pin and an RC filter, your result may be lower than expected, as the pin drivers are not symmetric.

#### Separate pullup-enable register
Like the ATtinyx41 and ATtiny1634, these have a fourth register for each port, PUEx, which controls the pullups (rather than PORTx when DDRx has pin set as input). Like the ATtiny1634, and unlike the ATtiny841, all of these pullup registers are located in the low IO space and sensibly relative to each other and to the other PORT-related registers.

### Purchasing ATtiny828 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny828 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATtiny828 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-05-18T04:57:39.963Z-AZB-8_V2_Asy.png.855x570_q85_pad_rcrop.png)
### [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
### [Bare Boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/)

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny828, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Addresses are in the program space, and are word addresses (not byte addresses). vect_num is the number you are shown in the event of a duplicate vector error, among other things.

| vect_num | Addr.  | Vector Name        | Interrupt Definition                 |
|----------|--------|--------------------|--------------------------------------|
|        0 | 0x0000 | RESET_vect         | Not an interrupt - this is a jump to the start of your code.  |
|        1 | 0x0001 | INT0_vect          | External Interrupt Request 0         |
|        2 | 0x0002 | INT1_vect          | External Interrupt Request 1         |
|        3 | 0x0003 | PCINT0_vect        | Pin Change Interrupt 0 (PORT A)      |
|        4 | 0x0004 | PCINT1_vect        | Pin Change Interrupt 1 (PORT B)      |
|        5 | 0x0005 | PCINT2_vect        | Pin Change Interrupt 2 (PORT C)      |
|        6 | 0x0006 | PCINT3_vect        | Pin Change Interrupt 3 (PORT D)      |
|        7 | 0x0007 | WDT_vect           | Watchdog Time-out (Interrupt Mode)   |
|        8 | 0x0008 | TIMER1_CAPT_vect   | Timer/Counter1 Input Capture         |
|        9 | 0x0009 | TIMER1_COMPA_vect  | Timer/Counter1 Compare Match A       |
|       10 | 0x000A | TIMER1_COMPB_vect  | Timer/Counter1 Compare Match B       |
|       11 | 0x000B | TIMER1_OVF_vect    | Timer/Counter1 Overflow              |
|       12 | 0x000C | TIMER0_COMPA_vect  | Timer/Counter0 Compare Match A       |
|       13 | 0x000D | TIMER0_COMPB_vect  | Timer/Counter0 Compare Match B       |
|       14 | 0x000E | TIMER0_OVF_vect    | Timer/Counter0 Overflow              |
|       15 | 0x000F | SPI_vect           | SPI Serial Transfer Complete         |
|       16 | 0x0010 | USART0_RXS_vect    | USART0 Rx Start                      |
|       17 | 0x0011 | USART0_RXC_vect    | USART0 Rx Complete                   |
|       18 | 0x0012 | USART0_DRE_vect    | USART0 Data Register Empty           |
|       19 | 0x0013 | USART0_TXC_vect    | USART0 Tx Complete                   |
|       20 | 0x0014 | ADC_READY_vect     | ADC Conversion Complete              |
|       21 | 0x0015 | EE_RDY_vect        | EEPROM Ready                         |
|       22 | 0x0016 | ANA_COMP_vect      | Analog Comparator                    |
|       23 | 0x0017 | TWI_vect           | Two-Wire slave Interface             |
|       24 | 0x0018 | SPM_RDY_vect       | Store Program Memory Ready           |
|       25 | 0x0019 | QTRIP_vect         | QTouch - No Arduino support          |
