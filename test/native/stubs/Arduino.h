#pragma once
#include <cstdint>
#include <cstring>
using ulong = unsigned long;
class Stream {};
class HardwareSerial : public Stream {};
constexpr int LOW = 0;
constexpr int HIGH = 1;
constexpr int CHANGE = 2;
inline void delay(uint32_t) {}
