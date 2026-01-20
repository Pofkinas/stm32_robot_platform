#include "logger.h"

#include <string.h>
#include "esp_log.h"

#include "web_server.h"

char Logger::buffer[LOGGER_BUFFER_SIZE];
Logger::LogCallback Logger::callback = nullptr;

void Logger::init() {
}

void Logger::set_callback(LogCallback cb) {
    callback = cb;
}

void Logger::logFormatted(const char* prefix, const char* format, va_list args) {
    size_t message_length = 0;

    if (prefix != nullptr && prefix[0] != '\0') {
        message_length = snprintf(buffer, LOGGER_BUFFER_SIZE, "%s", prefix);
    }

    if (message_length < LOGGER_BUFFER_SIZE) {
        message_length += vsnprintf(buffer + message_length, LOGGER_BUFFER_SIZE - message_length, format, args);
    }

    if (callback != nullptr) {
        callback(buffer);
    } else if (g_server_instance != nullptr) {
        g_server_instance->add_log_message(buffer);
    }
}

void Logger::log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("< ", format, args);
    va_end(args);
}

void Logger::raw(const char* message) {
    if (message == nullptr) return;
    if (callback != nullptr) {
        callback(message);
    } else if (g_server_instance != nullptr) {
        g_server_instance->add_log_message(message);
    }
}

void Logger::logESP(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("< [ESP]: ", format, args);
    va_end(args);
}

void Logger::logSTM(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("< [STM]: ", format, args);
    va_end(args);
}

void Logger::logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("[INFO] ", format, args);
    va_end(args);
}

void Logger::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("[ERROR] ", format, args);
    va_end(args);
}

void Logger::logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logFormatted("[WARNING] ", format, args);
    va_end(args);
}
