#include "debug.h"

#include <stdio.h>
#include <string.h>
#include "message.h"

UART* Debug::uart = nullptr;
char Debug::buffer[DEBUG_BUFFER_SIZE];

bool Debug::init() {
    uart = new UART(DEBUG_UART, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, DEBUG_BAUDRATE, DEBUG_BUFFER_SIZE, 0, NULL);
    
    if (uart == nullptr) {
        return false;
    }
    
    if (!uart->init()) {
        delete uart;
        uart = nullptr;
        return false;
    }
    
    return true;
}

void Debug::printFormatted(const char* prefix, const char* format, va_list args) {
    if (uart == nullptr) return;
    
    static size_t message_length = 0;
    message_length = sprintf(buffer, "%s", prefix);
    
    if (message_length > 0 && message_length < DEBUG_BUFFER_SIZE) {
        message_length += vsprintf((buffer + message_length), format, args);
    }

    //message_length += sprintf((buffer + message_length), "\r");

    sMessage_t message = {.data = buffer, .size = message_length};

    uart->transmit(message);
}

void Debug::logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printFormatted("[INFO] ", format, args);
    va_end(args);
}

void Debug::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printFormatted("[ERROR] ", format, args);
    va_end(args);
}

void Debug::logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printFormatted("[WARNING] ", format, args);
    va_end(args);
}
