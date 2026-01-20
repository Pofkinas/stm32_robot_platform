#pragma once

// Command UART Configuration
#define CMD_UART UART_NUM_2
#define CMD_BAUDRATE 115200
#define CMD_UART_RX GPIO_NUM_16
#define CMD_UART_TX GPIO_NUM_17
#define CMD_STACK_SIZE 4096

#define CMD_BUFFER_SIZE 256
#define CMD_RESPONSE_SIZE 256

// ESP Protocol Configuration
#define ESP_HEADER_SIZE 2  // CMD_BYTE + ARG_COUNT
#define ESP_MAX_ARGS 10
#define ESP_ARG_SEPARATOR ","
#define ESP_MSG_DELIMITER "\n"

// Debug UART Configuration
#define DEBUG_UART UART_NUM_0
#define DEBUG_BAUDRATE 115200
#define DEBUG_BUFFER_SIZE 256

// Encoder Configuration
#define ENCODER_LEFT_PIN GPIO_NUM_4
#define ENCODER_RIGHT_PIN GPIO_NUM_13

#define ENCODER_USE_HARDWARE_TIMER
// #define ENCODER_USE_FREERTOS_TIMER
// If not using hardware timer, FreeRTOS timers will be used, which have millisecond precision.
#define DEBOUNCE_TIME_US 5 // 5 microseconds


// WiFi Configuration
#define WIFI_SSID "WiFi_SSID"
#define WIFI_PASSWORD "password"

#define WEB_SERVER_PORT 80
#define WEB_SERVER_STACK_SIZE 8192
