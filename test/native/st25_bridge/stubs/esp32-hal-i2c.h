#pragma once
#include <Wire.h>
inline bool mismatch = false;
inline bool i2cIsInit(uint8_t) { return initialized; }
inline void* i2cBusHandle(uint8_t) { return mismatch ? &otherBus : &busInstance; }
