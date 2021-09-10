#include <Arduino.h>
#include <stdarg.h>
#include "Logger.h"

void Logger::Init(uint32_t baud)
{
    Serial.begin(baud);
}

void Logger::Log(const char* format, ...)
{
    if(Serial.availableForWrite())
    {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
    }
}
