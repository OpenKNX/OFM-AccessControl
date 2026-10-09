#pragma once
#include "Arduino.h"
#define logDebugP(...) ((void)0)
#define logInfoP(...) ((void)0)
#define logErrorP(...) ((void)0)
inline void logIndentUp() {}
inline void logIndentDown() {}
inline uint32_t delayTimerInit() { return 0; }
inline bool delayCheck(uint32_t, uint32_t) { return true; }
struct TestGpio
{
    int calls = 0;
    int32_t lastPin = -1;
    void pinMode(int32_t pin, uint8_t) { ++calls; lastPin = pin; }
    void digitalWrite(int32_t pin, uint8_t) { ++calls; lastPin = pin; }
    int digitalRead(int32_t pin) { ++calls; lastPin = pin; return HIGH; }
};
struct TestOpenKnx { TestGpio gpio; };
inline TestOpenKnx openknx;
inline int interruptCalls = 0;
inline int32_t lastInterruptPin = -1;
inline int32_t digitalPinToInterrupt(int32_t pin) { return pin; }
inline void attachInterrupt(int32_t pin, void (*)(), int)
{
    ++interruptCalls;
    lastInterruptPin = pin;
}
