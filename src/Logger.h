#pragma once

class Logger
{
public:
    static void Init(uint32_t baud);
    static void Log(const char* format, ...);
};
