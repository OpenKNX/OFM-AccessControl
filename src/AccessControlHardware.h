#pragma once

#include "OpenKNX.h"

#ifndef SCANNER_TOUCH_PIN
    #define SCANNER_TOUCH_PIN -1
#endif

#ifndef DIRECT_TOUCH_LEFT_PIN
    #define DIRECT_TOUCH_LEFT_PIN -1
#endif

#ifndef DIRECT_TOUCH_RIGHT_PIN
    #define DIRECT_TOUCH_RIGHT_PIN -1
#endif

#ifndef DIRECT_LED_GREEN_PIN
    #define DIRECT_LED_GREEN_PIN -1
#endif

#ifndef DIRECT_LED_RED_PIN
    #define DIRECT_LED_RED_PIN -1
#endif

#ifndef EXTERN_TOUCH_LEFT_PIN
    #define EXTERN_TOUCH_LEFT_PIN -1
#endif

#ifndef EXTERN_TOUCH_RIGHT_PIN
    #define EXTERN_TOUCH_RIGHT_PIN -1
#endif

#ifndef EXTERN_LED_GREEN_PIN
    #define EXTERN_LED_GREEN_PIN -1
#endif

#ifndef EXTERN_LED_RED_PIN
    #define EXTERN_LED_RED_PIN -1
#endif

namespace AccessControlHardware
{
    inline void pinMode(int32_t pin, uint8_t mode)
    {
        if (pin >= 0)
            openknx.gpio.pinMode(pin, mode);
    }

    inline void digitalWrite(int32_t pin, uint8_t value)
    {
        if (pin >= 0)
            openknx.gpio.digitalWrite(pin, value);
    }

    inline int digitalRead(int32_t pin)
    {
        return pin >= 0 ? openknx.gpio.digitalRead(pin) : LOW;
    }

    inline void attachInterrupt(int32_t pin, void (*callback)(), decltype(CHANGE) mode)
    {
#ifdef ARDUINO_ARCH_ESP32
        // Expander touch inputs are polled in AccessControl::loop on ESP32.
        if (pin > 0xFF)
            return;
#endif
        if (pin >= 0)
            ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
    }
}
