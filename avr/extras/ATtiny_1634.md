### ATtiny 1634(R)
![1634 Pin Mapping](Pinout_1634.jpg "Arduino Pin Mapping for ATtiny 1634")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 16384b ( 15744b with Optiboot)
RAM  | 1024 bytes
EEPROM | 256 bytes
Bootloader | Yes, Optiboot (serial)
GPIO Pins | 17
ADC Channels | 12
PWM Channels | 4
Interfaces | 2x UART, USI, slave I2C
Clock options | Internal 1/8MHz, external crystal or clock* up to 12MHz, overclocking to 16MHz.

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

The ATtiny1634R has a more tightly factory calibrated internal oscillator. It is otherwise identical, has the same signature, and is interchangible.

### Warning: Pin 14 (PB3) does not work as an input unless watchdog timer is running
This is a design flaw in the chip, as noted in the datasheet errata.
See workaround below.

## Programming
The ATtiny1634 can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny1634, operating on the hardware UART0 (Serial) port at 115200 baud for 12 MHz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 15744b available for user code. In order to work on these parts, which do not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included as well (choose the desired UART when burning the booloader).

## Features

### Internal Oscillator voltage dependence
Prior to 1.4.0, many users had encountered issues due to the voltage dependence of the oscillator. While the calibration is very accurate between 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Internal, Vcc < 4.5v) and 8 MHz (Internal, Vcc > 4.5 MHz ) board definitions - it should generally work between 2.7v and 5.25v - though the extremes of that range may be dicey. We do still provide a >4.5v clock option in order to improve behavior of the running sketch - it will nudge the oscillator calibration down to move it closer to the nominal 8MHz clock speed; sketches uploaded with the higher voltage option. This is not perfect, but it is generally good enough to work with Serial on around 5v (including 5.25v often found on USB ports to facilitate chargeing powerhungry devices), and millis()/micros() will keep better time than in previous versions.

The internal oscillator is factory calibrated to +/- 10% or +/- 2% for the slightly more expensive 1634R. +/- 2% is good enough for serial communication. However, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work absent the above-described countermeasures.

### Tone Support
Tone() uses Timer1. For best results, use pin 2 or 14 (PIN_PA6, PIN_PB3), as this will use the hardware output compare to generate the square wave instead of using interrupts. Any use of tone() will disable PWM on pins 2 and 14

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core. There is also a slave-only hardware TWI, however, the Wire.h library does not make use of this.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There are two hardware serial ports, Serial and Serial1. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels (for Serial1, use UCSR1B instead)
```
UCSR0B&=~(1<<TXEN0); // disable TX
UCSR0B&=~(1<<RXEN0); // disable RX
```

### ADC Reference options
Note that **when using the Internal 1.1v reference, you must not apply an external voltage to AREF pin** - this sometimes appears to work, but other times results in erroneous ADC readings. Unlike some parts, there is no option to use the internal reference without the AREF pin being connected to it!

* DEFAULT: Vcc
* EXTERNAL: External voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference, AREF may have added capacitor for improved ADC stability.
* INTERNAL: synonym for INTERNAL1V1

### Overclocking
Experience has shown that the ATtiny1634, operating at 5v and room temperature, will typically function at 16 MHz at 5v and room temperature without issue, although this is outside of the manufacturer's specification.

### Purchasing ATtiny1634 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny1634 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATtiny1634 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2015-06-21T05:40:17.284Z-T1634AMain3.png.855x570_q85_pad_rcrop.png)
* [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny1634-dev-board-woptiboot-assembled/)
* [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny1634-breakout-wserial-header-bare-board/)

### PB3 silicon errata workaround
If you have no need to use the WDT, but do have a need to use PB3 as an input, you can keep the WDT running by putting it into interrupt mode, with an empty interrupt, at the cost of just 10b of flash, an ISR that executes in 11 clock cycles every 8 seconds, and an extra 1-4uA of power consumption (negligible compared to what the chip consumes when not sleeping, and you'll turn it off while sleeping anyway - see below) - so the real impact of this issue is in fact very low, assuming you know about it and don't waste hours or days trying to figure out what is going on.

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



## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny1634, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000 | RESET_vect | Any reset (pin, WDT, power-on, BOD)
1 | 0x0002 | INT0_vect | External Interrupt Request 0
2 | 0x0004 | PCINT0_vect | Pin Change Interrupt 0 (PORT A)
3 | 0x0006 | PCINT1_vect | Pin Change Interrupt 1 (PORT B)
4 | 0x0008 | PCINT2_vect | Pin Change Interrupt 2 (PORT C)
5 | 0x000A | WDT_vect | Watchdog Time-out (Interrupt Mode)
6 | 0x000C | TIM1_CAPT_vect | Timer/Counter1 Input Capture
6 | 0x000C | TIMER1_CAPT_vect | Alias - provided by ATTinyCore
7 | 0x000E | TIM1_COMPA_vect | Timer/Counter1 Compare Match A
7 | 0x000E | TIMER1_COMPA_vect | Alias - provided by ATTinyCore
8 | 0x0010 | TIM1_COMPB_vect | Timer/Counter1 Compare Match B
8 | 0x0010 | TIMER1_COMPB_vect | Alias - provided by ATTinyCore
9 | 0x0012 | TIM1_OVF_vect | Timer/Counter1 Overflow
9 | 0x0012 | TIMER1_OVF_vect | Alias - provided by ATTinyCore
10 | 0x0014 | TIM0_COMPA_vect | Timer/Counter0 Compare Match A
10 | 0x0014 | TIMER0_COMPA_vect | Alias - provided by ATTinyCore
11 | 0x0016 | TIM0_COMPB_vect | Timer/Counter0 Compare Match B
11 | 0x0016 | TIMER0_COMPB_vect | Alias - provided by ATTinyCore
12 | 0x0018 | TIM0_OVF_vect | Timer/Counter0 Overflow
12 | 0x0018 | TIMER0_OVF_vect | Alias - provided by ATTinyCore
13 | 0x001A | ANA_COMP_vect | Analog Comparator
14 | 0x001C | ADC_READY_vect | ADC Conversion Complete
15 | 0x001E | USART0_RXS_vect | USART0 Rx Start
16 | 0x0020 | USART0_RXC_vect | USART0 Rx Complete
17 | 0x0022 | USART0_DRE_vect | USART0 Data Register Empty
18 | 0x0024 | USART0_TXC_vect | USART0 Tx Complete
19 | 0x0026 | USART1_RXS_vect | USART1 Rx Start
20 | 0x0028 | USART1_RXC_vect | USART1 Rx Complete
21 | 0x002A | USART1_DRE_vect | USART1 Data Register Empty
22 | 0x002C | USART1_TXC_vect | USART1 Tx Complete
23 | 0x002E | USI_STR_vect | USI START
24 | 0x0030 | USI_OVF_vect | USI Overflow
25 | 0x0032 | TWI_vect | Two-Wire Interface
26 | 0x0034 | EE_RDY_vect | EEPROM Ready
27 | 0x0036 | QTRIP_vect | QTouch
