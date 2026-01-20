#pragma once

#include <stdint.h>
#include <stdarg.h>
#include "uart.h"
#include "config.h"

class Debug {
public:
    static bool init();
    static void logError(const char* format, ...);
    static void logWarning(const char* format, ...);
    static void logInfo(const char* format, ...);
private:
    static void printFormatted(const char* prefix, const char* format, va_list args);
    static UART* uart;
    static char buffer[DEBUG_BUFFER_SIZE];
};

#define DEBUG_PRINT(...) Debug::printFormatted(__VA_ARGS__)
#define DEBUG_ERROR(...) Debug::logError(__VA_ARGS__)
#define DEBUG_WARNING(...) Debug::logWarning(__VA_ARGS__)
#define DEBUG_INFO(...) Debug::logInfo(__VA_ARGS__)