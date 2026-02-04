/*
 Copyright (c) 2013 Benjamin Holt
 
 The MIT License (MIT) - See README for complete license
 */


#include <SimpleServo.h>


/////  Setup  /////
SimpleServo::SimpleServo() :
_pin(0xff),
_degrees(0xff),
_microseconds(0),
_maxDegrees(180),
_millisPer60degrees(230),  // default from http://www.servodatabase.com/servo/futaba/s3003
_minWriteMillis(0),
_maxWriteMillis(0xffff),
_minPulse(700),
_maxPulse(2300),
_pulseMillis(16)
{}


uint8_t SimpleServo::attach(uint8_t p) {
    _pin = p;
    _degrees = 0xff;
    _microseconds = 0;
    // Deliberately not (re)initializing servo parameters
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    return 1;
}


void SimpleServo::detach() {
    _pin = 0xff;
}


uint8_t SimpleServo::attached() {
    return _pin != 0xff;
}
/////


/////  Movement  /////
void SimpleServo::_pulse(uint16_t ms) {
    // This actually moves the servo
    uint16_t elapsedMillis = 0;
    while (elapsedMillis < ms) {
        digitalWrite(_pin,HIGH);
        delayMicroseconds(_microseconds);
        digitalWrite(_pin,LOW);
        if (ms > 0) {  // 0ms: do the pulse, but that's it (probably a bad idea but, hey, if it works with your project, cool)
            delay(_pulseMillis);  // ENHANCEME: A hook to allow the client to make use (at their own risk of adding jitter) of these millis would be really nice
        }
        elapsedMillis += _pulseMillis + _microseconds / 1000 + 1;
    }
}


void SimpleServo::writeMicrosecondsMillis(uint16_t us, uint16_t ms) {
    _microseconds = us;  // Allow unconstrained us
    if (_minPulse <= us && us <= _maxPulse) {
        _degrees = map(us, _minPulse, _maxPulse, 0, _maxDegrees);
    } else {
        _degrees = 0xff;  // Out-of-range us do not convert sensibly to degrees
    }
    _pulse(ms);  // And unconstrained ms, don't point this method at your foot.
}


void SimpleServo::writeMillis(uint8_t deg, uint16_t ms) {
    _degrees = constrain(deg, 0, _maxDegrees);
    _microseconds = constrain(map(_degrees, 0, _maxDegrees, _minPulse, _maxPulse), _minPulse, _maxPulse);
    _pulse(ms);  // Allow unconstrained millis
}


uint32_t SimpleServo::millisToTarget(uint8_t deg) {
    uint8_t delta = abs(_degrees - deg);  // if degrees is 0xff, this will over-estimate, so don't do anything special in that case
    return ((uint32_t)_millisPer60degrees * delta) / 60;
}


void SimpleServo::writeMicroseconds(uint16_t us) {
    // Simplest to do the us -> deg conversion here, so just finish the job
    _microseconds = constrain(us, 0, _maxPulse);  // keep degrees sane as much as possible, you have writeMicrosecondsMillis if you need to bend the rules
    uint8_t target = map(us, _minPulse, _maxPulse, 0, _maxDegrees);
    uint32_t millis = millisToTarget(target);
    millis = constrain(millis, _minWriteMillis, _maxWriteMillis);
    _degrees = target;
    _pulse((uint16_t)millis);
}


void SimpleServo::write(uint8_t deg) {
    uint32_t millis = millisToTarget(deg);
    millis = constrain(millis, _minWriteMillis, _maxWriteMillis);
    writeMillis(deg, (uint16_t)millis);
}


uint8_t SimpleServo::read() {
    return _degrees;
}


uint16_t SimpleServo::readMicroseconds() {
    return _microseconds;
}
/////


/////  Servo Parameters  /////
void SimpleServo::setMaximumDegrees(uint8_t deg) {
    _maxDegrees = deg;
}


void SimpleServo::setMillisPer60Degrees(uint16_t ms) {
    _millisPer60degrees = ms;
}


void SimpleServo::setMinimumMillis(uint16_t ms) {
    _minWriteMillis = ms;
}


void SimpleServo::setMaximumMillis(uint16_t ms) {
    _maxWriteMillis = ms;
}


void SimpleServo::setMinimumPulse(uint16_t us) {
    _minPulse = us;
}


void SimpleServo::setMaximumPulse(uint16_t us) {
    _maxPulse = us;
}


void SimpleServo::setPulseMillis(uint8_t ms) {
    _pulseMillis = ms;
}
/////
