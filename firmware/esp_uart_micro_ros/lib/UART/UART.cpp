#include "UART.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define UART_NUM UART_NUM_2

#define UART2_RX_BUFFER_SIZE 1024
#define UART2_TX_BUFFER_SIZE 0

UART::UART() {}

uart_port_t uart_num = UART_NUM;

void UART::Init() {
    Serial.begin(115200);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(uart_num, UART2_RX_BUFFER_SIZE, UART2_TX_BUFFER_SIZE, 0, NULL, 0));
}

void UART::Receive(uint8_t *data, const size_t length, const TickType_t timeout) {
    if (data == NULL || length == 0) {
        return false;
    }

    int32_t read_bytes = uart_read_bytes(uart_num, data, length, timeout);

    return (read_bytes < 0);
}
