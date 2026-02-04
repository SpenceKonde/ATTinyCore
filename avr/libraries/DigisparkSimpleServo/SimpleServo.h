/*
 Copyright (c) 2013 Benjamin Holt
 
 The MIT License (MIT) - See README for complete license
*/

#ifndef _SimpleServo_h_
#define _SimpleServo_h_

#include <inttypes.h>
#include <WProgram.h>

class SimpleServo {
  private:
    //  Movement
    uint8_t  _pin;
    uint8_t  _degrees;
    uint16_t _microseconds;
    
    //  Servo Parameters
    // REM: nasty bit about when to constrain vs. not... more below...
    uint8_t  _maxDegrees;
    uint16_t _millisPer60degrees;
    uint16_t _minWriteMillis;
    uint16_t _maxWriteMillis;
    uint16_t _minPulse;
    uint16_t _maxPulse;
    uint8_t  _pulseMillis;
    
    void _pulse(uint16_t);

  public:
    //  TODO: update keywords file
    //  Setup
    SimpleServo();
    uint8_t attach(uint8_t);  // Note: resets position, but not servo parameters, doesn't signal servo
    void detach();
    uint8_t attached();
    
    //  Movement
    void write(uint8_t);
    void writeMicroseconds(uint16_t);
    uint32_t millisToTarget(uint8_t);  // Estimates time from current (expected) position to a target; if millisPer60Degrees is too small, this will come up short and the servo may not reach the target, too large and the write methods may signal longer than needed
    void writeMillis(uint8_t, uint16_t);  // Allows unconstrained millis
    void writeMicrosecondsMillis(uint16_t, uint16_t);  // Allows unconstrained microseconds and millis, full manual control
    uint8_t read();
    uint16_t readMicroseconds();
    
    
    //  Servo Parameters
    void setMaximumDegrees(uint8_t);  // A lot of servos move a bit more than 180
    void setMillisPer60Degrees(uint16_t);  // For estimating time from last set position to newly requested position; different for every servo, http://www.servodatabase.com has many specs, default is 230 from Futaba S3003
    void setMinimumMillis(uint16_t);  // The smallest time to signal the servo; some write methods do not enforce this
    void setMaximumMillis(uint16_t);  // This longest time to signal the servo, even if it probably has not yet reached the requested position; some write methods do not enforce this
    void setMinimumPulse(uint16_t);  // The shortest pulse (in microseconds) to send to the servo, maps to 0deg; only writeMicrosecondsMillis does not enforce this
    void setMaximumPulse(uint16_t);  // The shortest pulse (in microseconds) to send to the servo, maps to 0deg; only writeMicrosecondsMillis does not enforce this
    void setPulseMillis(uint8_t);  // Delay between pulses; defaults to 16 which is a bit quicker than spec.  Experiment, different servos may react differently
};
#endif  // _SimpleServo_h_
