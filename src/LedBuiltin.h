#pragma once

#include <Arduino.h>

class LedBuiltin
{
public:
    static void Init()
    {
#ifdef LED_BUILTIN
        pinMode(LED_BUILTIN, OUTPUT);
#else
        pinMode(LED_BUILTIN_RED, OUTPUT);
        pinMode(LED_BUILTIN_GREEN, OUTPUT);
        pinMode(LED_BUILTIN_BLUE, OUTPUT);
#endif
    }

    static void Set(bool high)
    {
#ifdef LED_BUILTIN
        digitalWrite(LED_BUILTIN, high ? HIGH : LOW);
#else
        uint8_t state = high ? 255 : 0;
        digitalWrite(LED_BUILTIN_RED, state);
        digitalWrite(LED_BUILTIN_GREEN, state);
        digitalWrite(LED_BUILTIN_BLUE, state);
#endif
    }
};
