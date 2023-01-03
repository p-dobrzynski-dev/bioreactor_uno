// Pump.h
#ifndef pump_h
#define pump_h

#include <Arduino.h>

class Pump
{
private:
    int pwmPin;
    int dir1Pin;
    int dir2Pin;
    int currentPwmValue;

public:
    Pump(int pumpPwmPin, int pump1DirPin, int pump2DirPin);
    void setPumpSpeed(int value);
    int GetCurrentSpeed();
};

#endif