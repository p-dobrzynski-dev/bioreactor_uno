// pump.cpp
#include "pump.h"

Pump::Pump(int pumpPwmPin, int pump1DirPin, int pump2DirPin)
{
    pwmPin = pumpPwmPin;
    dir1Pin = pump1DirPin;
    dir2Pin = pump2DirPin;

    pinMode(pwmPin, OUTPUT);
    pinMode(dir1Pin, OUTPUT);
    pinMode(dir2Pin, OUTPUT);
}

void Pump::setPumpSpeed(int value)
{
    if (value >= 0)
    {
      digitalWrite(dir1Pin, HIGH);
      digitalWrite(dir2Pin, LOW);
    }
    else
    {
      digitalWrite(dir1Pin, LOW);
      digitalWrite(dir2Pin, HIGH);
    }
    analogWrite(pwmPin, abs(value));

    currentPwmValue = value;
}

int Pump::GetCurrentSpeed() {
    return currentPwmValue;
}
