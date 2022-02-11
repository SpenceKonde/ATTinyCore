#if defined(__AVR_ATtinyX5__)
  typedef enum TIMERONE_CLOCK_enum
  {
      TIMER1_FCPU    = (0x00),
      TIMER1_64M_PLL = (0x06),
      TIMER1_32M_PLL = (0x86)
  } TIMERONE_CLOCK_t;
  typedef enum TIMERONE_PRESC_enum
  {
    TIMER1_STOPPED  = (0x00),
    TIMER1_DIV1     = (0x01),
    TIMER1_DIV2     = (0x02),
    TIMER1_DIV4     = (0x03),
    TIMER1_DIV8     = (0x04),
    TIMER1_DIV16    = (0x05),
    TIMER1_DIV32    = (0x06),
    TIMER1_DIV64    = (0x07),
    TIMER1_DIV128   = (0x08),
    TIMER1_DIV256   = (0x09),
    TIMER1_DIV512   = (0x0A),
    TIMER1_DIV1024  = (0x0B),
    TIMER1_DIV2048  = (0x0C),
    TIMER1_DIV4096  = (0x0D),
    TIMER1_DIV8192  = (0x0E),
    TIMER1_DIV16384 = (0x0F)
  } TIMER1_PRESCALE_t;

  void setTimer1Clock(TIMERONE_CLOCK_t clk) {
    uint8_t t = PLLCSR & 0x87;
    if (t & 0x86 == clk) {    // already set right
      return;
    }
    uint8_t oldtccr = TCCR1;
    TCCR1 = oldtccr & 0xF0;
    if ((clk & 0x04) && (t & 0x03 != 3)) {
      // PLL requested and not currently running
      if (t & 0x02 == 0) {
        PLLCSR = clk & 0x02; // enable the PLL
      }
      while (PLLCSR & 0x01 == 0); // Wait for lock
    }
    PLLCSR = clk; // now we can set the PLLCSR.
    TCCR1 = oldtccr; // and turn back on the timer.
  }
  void setTimer1Prescaler(TIMER1_PRESCALE_t presc) {
    uint8_t oldtccr = TCCR1;
    tccr &= 0xF0;
    tccr |= presc;
    TCCR1 = tccr;
  }
  void setTimer1Top(uint8_t newtop) {
    OCR1C = newtop;
  }
  uint8_t setTimer1Frequency(uint32_t target) {
    while (true) {
      uint8_t pll = PLLCSR;
      pll &= 0x84;
      unsigned long speed;
      if (pll == 0x84) {
        #if F_CPU  == 16500000
          speed     = 33000000;
        #else
          speed     = 32000000;
        #endif
      } else if (pll) {
        #if F_CPU  == 16500000
          speed     = 66000000;
        #else
          speed     = 64000000;
        #endif
      } else {
        speed     = F_CPU;
      }
      uint32_t topval = speed/target;
      if (topval < 64 && pll != 0x04) {
        setTimer1Clock(TIMER1_64M_PLL);
      } else if (topval < 128 && pll == 0) {
        setTimer1Clock(TIMER1_32M_PLL);
      } else {
        uint8_t newpresc = 1;
        while (topval > 255 && newpresc < 15) {
          newpresc++;
          topval    >>= 1;
        }
        uint8_t top;
        if (topval > 255) {
          top = 255;
        } else {
          top = topval;
        }
        OCR1C         = top;
        uint8_t tccr  = TCCR1;
        tccr         &= 0xF0;
        tccr         |= newpresc;
        TCCR1         = tccr;
        return top;
      }
    }
  }
#endif

#if defined(__AVR_ATtinyX61__)
    typedef enum TIMERONE_CLOCK_enum
  {
      TIMER1_FCPU    = (0x00),
      TIMER1_64M_PLL = (0x06),
      TIMER1_32M_PLL = (0x86)
  } TIMERONE_CLOCK_t;
  typedef enum TIMERONE_PRESC_enum
  {
    TIMER1_STOPPED  = (0x00),
    TIMER1_DIV1     = (0x01),
    TIMER1_DIV2     = (0x02),
    TIMER1_DIV4     = (0x03),
    TIMER1_DIV8     = (0x04),
    TIMER1_DIV16    = (0x05),
    TIMER1_DIV32    = (0x06),
    TIMER1_DIV64    = (0x07),
    TIMER1_DIV128   = (0x08),
    TIMER1_DIV256   = (0x09),
    TIMER1_DIV512   = (0x0A),
    TIMER1_DIV1024  = (0x0B),
    TIMER1_DIV2048  = (0x0C),
    TIMER1_DIV4096  = (0x0D),
    TIMER1_DIV8192  = (0x0E),
    TIMER1_DIV16384 = (0x0F)
  } TIMER1_PRESCALE_t;

  void setTimer1Clock(TIMERONE_CLOCK_t clk) {
    uint8_t t = PLLCSR & 0x87;
    if (t & 0x86 == clk) {    // already set right
      return;
    }
    uint8_t oldtccr = TCCR1B;
    TCCR1B = oldtccr & 0xF0;
    if ((clk & 0x04) && (t & 0x03 != 3)) {
      // PLL requested and not currently running
      if (t & 0x02 == 0) {
        PLLCSR = clk & 0x02; // enable the PLL
      }
      while (PLLCSR & 0x01 == 0); // Wait for lock
    }
    PLLCSR = clk; // now we can set the PLLCSR.
    TCCR1B = oldtccr; // and turn back on the timer.
  }
  void setTimer1Prescaler(TIMER1_PRESCALE_t presc) {
    uint8_t oldtccr = TCCR1B;
    tccr &= 0xF0;
    tccr |= presc;
    TCCR1B = tccr;
  }
  void setTimer1Top(uint8_t newtop) {
    uint8_t oldsreg = SREG;
    cli();
    TC1H  = 0;
    OCR1C = newtop;
    SREG  = oldsreg;
  }

  void setTimer1Top_10bit(uint16_t newtop) {
    TC1H = (newtop >> 8);
    OCR1C = newtop & 0xFF;
  }
  uint8_t setTimer1Frequency(uint32_t target) {
    while (true) {
      unsigned long speed;
      uint8_t pll   = PLLCSR;
      pll          &= 0x84;
      if (pll == 0x84) {
        #if F_CPU  == 16500000
          speed     = 33000000;
        #else
          speed     = 32000000;
        #endif
      } else if (pll) {
        #if F_CPU  == 16500000
          speed     = 66000000;
        #else
          speed     = 64000000;
        #endif
      } else {
        speed       = F_CPU;
      }
      uint32_t topval = speed/target;
      if (topval < 64 && pll != 0x04) {
        setTimer1Clock(TIMER1_64M_PLL);
      } else if (topval < 128 && pll == 0) {
        setTimer1Clock(TIMER1_32M_PLL);
      } else {
        uint8_t newpresc = 1;
        while (topval > 255 && newpresc < 15) {
          newpresc++;
          topval    >>= 1;
        }
        uint8_t top;
        if (topval > 256) {
          top = 255;
        } else {
          top = (uint8_t)topval - 1;
        }
        OCR1C         = top;
        uint8_t tccr  = TCCR1B;
        tccr         &= 0xF0;
        tccr         |= newpresc;
        TCCR1B        = tccr;
        return top;
      }
    }
  }
  uint16_t setTimer1Frequency_10bit(uint32_t target) {
    while (true) {
      unsigned long speed;
      uint8_t pll   = PLLCSR;
      pll          &= 0x84;
      if (pll == 0x84) {
        #if F_CPU  == 16500000
          speed     = 33000000;
        #else
          speed     = 32000000;
        #endif
      } else if (pll) {
        #if F_CPU  == 16500000
          speed     = 66000000;
        #else
          speed     = 64000000;
        #endif
      } else {
        speed       = F_CPU;
      }
      uint32_t topval = speed/target;
      if (topval < 10 && pll != 0x04) {
        setTimer1Clock(TIMER1_64M_PLL);
      } else if (topval < 128 && pll == 0) {
        setTimer1Clock(TIMER1_32M_PLL);
      } else {
        uint8_t newpresc = 1;
        while (topval > 1024 && newpresc < 15) {
          newpresc++;
          topval    >>= 1;
        }
        uint16_t top;
        if (topval >= 1024) {
          top = 1023;
        } else {
          top = (uint16_t)topval - 1;
          setTimer1Top_10bit(top);
        }
        uint8_t tccr  = TCCR1B;
        tccr         &= 0xF0;
        tccr         |= newpresc;
        TCCR1B        = tccr;
        return top;
      }
    }
  }
  uint8_t pwmchannneltopin(uint8_t ch) {
    #if defined (PINMAPPING_NEW)
      if (ch < 3) {
        return (uint8_t)9 + ch + ch;
      }
    #else
      if (ch <3) {
        return (uint8_t)8 - ch - ch;
      }
    #endif
    return NOT_A_PIN;
  }
  void setTimer1DutyCycle_10bit(uint8_t channel, uint16_t duty) {
    if (channel > 2 || duty > 1023) {
      return; // invalid channel
    }
    if (duty == 0) {
      digitalWrite(pwmchanneltopin(channel), LOW);
    } else if (duty == 1023) {
      digitalWrite(pwmchanneltopin(channel), HIGH; );
    } else {
      uint8_t oldsreg = SREG;
      cli();
      TC1H    = duty >> 8;
      if (channel == 0) {
        TCCR1C |= (1 << COM1A1S);
        OCR1A = (uint8_t) duty;
      } else if (channel == 1) {
        TCCR1C |= (1 << COM1B1S);
        OCR1B = (uint8_t) duty;
      } else {
        TCCR1C |= (1 << COM1D1);
        OCR1D = (uint8_t) duty;
      }
      SREG    = oldsreg;
    }
  }
#endif

/* a grab-bag of miscellaneous functionality */



uint8_t enableHighSinkPort(uint8_t port, bool mode) {
  #if defined(__AVR_ATtiny828__)
    if (port == PC) {
      PHDE = mode ? 0x04 : 0;
      return 0;
    } else {
      return 1;
    }
  #else
    badCall("Only the 828 has a whole port that can be set high-sink, though the 841/441 can set PA5 and PA7 high sink with enableHighSinkPin()");
  #endif
}

uint8_t enableHighSinkPin(uint8_t pin, bool mode) {
  #if defined(__AVR_ATtinyx41__)
    if (pin == PIN_PA7 || pin == PIN_PA5) {
      if (mode) {
        PHDE |= (pin==PIN_PA7,2,1);
      } else {
        PHDE &= (pin==PIN_PA7,0xFD,0xFE);
      }
      return 0;
    } else {
      return 1;
    }
  #else
    badCall("Only the 841 and 441 support this. The 828 can set PORTC high-sink with enableHighSinkPort()");
  #endif
}

uint8_t enableISRC(bool mode) {
// The T167 can connect PA3 toan internal current source. This is much more precise than the internal pullups, and does not vary over voltage changes like they do. It was proposed for LIN addressing and a variety of other uncommon tasks,
  #if defined(__AVR_ATtinyx7__)
    if (mode) {
      AMICR |= 1;
    } else {
      AMICR &= 0xFE;
    }
  #else
    badCall("The internal current source is only available on the t87 and t167");
  #endif
}

void disableAllPullups(bool mode) {
  #if defined(PUD)
    if (mode) {
      MCUCR |= 1 << PUD;
    } else {
      MCUCR &= ~(1 << PUD);
    }
  #else
    badCall("The global pullup disable feature is not available on this part");
  #endif
}

uint8_t enableBBM(uint8_t port, uint8_t mode) {
  #if defined(PORTCR) && defined(PUDB) //All parts have PORTB
    #if defined(PORTA) && defined(PUDA)
      if (port == PA) {
        PORTCR |= 1 << PUDA;
        return 0;
      } else
    #endif
    #if defined(PORTB) && defined(PUDB)
      if (PORT == PB) {
        PORTCR |= 1 << PUDB;
        return 0;
      } else
    #endif
    #if defined(PORTC) && defined(PUDC)
      if (PORT == PC) {
        PORTCR |= 1 << PUDA;
        return 0;
      } else
    #endif
    #if defined(PORTD) && defined(PUDD)
      if (PORT == PD) {
        PORTCR |= 1 << PUDD;
        return 0;
      } else
    #endif
    {
      return 1;
    }
  #else
    badCall("This part does not support portwise pullup disabling");
  #endif
}

uint8_t enableBBM(uint8_t port, uint8_t mode) {
  #if defined(PORTCR)
    #if defined(PORTA) && defined(BBMA)
      if (port == PA) {
        PORTCR |= 1 << BBMA;
        return 0;
      } else
    #endif
    #if defined(PORTB) && defined(BBMB)
      if (PORT == PB) {
        PORTCR |= 1 << BBMB;
        return 0;
      } else
    #endif
    #if defined(PORTC) && defined(BBMC)
      if (PORT == PC) {
        PORTCR |= 1 << BBMC;
        return 0;
      } else
    #endif
    #if defined(PORTD) && defined(BBMD)
      if (PORT == PD) {
        PORTCR |= 1 << BBMD;
        return 0;
      } else
    #endif
    {
      return 1;
    }
  #else
    badCall("This port does not support BBM mode.");
  #endif
}
