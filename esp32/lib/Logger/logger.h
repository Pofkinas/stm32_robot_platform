#pragma once

#include <stdarg.h>
#include <stdio.h>

#define LOGGER_BUFFER_SIZE 256

class Logger {
public:
    typedef void (*LogCallback)(const char* message);
    
    static void init();
    static void set_callback(LogCallback callback);
    static void log(const char* format, ...);
    // Send a raw, preformatted C-string to the callback without treating it as a printf format
    static void raw(const char* message);
    static void logESP(const char* format, ...);
    static void logSTM(const char* format, ...);
    static void logInfo(const char* format, ...);
    static void logError(const char* format, ...);
    static void logWarning(const char* format, ...);
    
private:
    static void logFormatted(const char* prefix, const char* format, va_list args);
    static LogCallback callback;
    static char buffer[LOGGER_BUFFER_SIZE];
};

#define LOG_RAW(msg) Logger::raw(msg)
#define LOG(...) Logger::log(__VA_ARGS__)
#define LOG_ESP(...) Logger::logESP(__VA_ARGS__)
#define LOG_STM(...) Logger::logSTM(__VA_ARGS__)
#define LOG_INFO(...) Logger::logInfo(__VA_ARGS__)
#define LOG_ERROR(...) Logger::logError(__VA_ARGS__)
#define LOG_WARNING(...) Logger::logWarning(__VA_ARGS__)
