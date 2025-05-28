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
        .source_clk = UART_SCLK_REF_TICK
    };

    uart_driver_delete(uart_num);
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, UART2_RX_BUFFER_SIZE, UART2_TX_BUFFER_SIZE, 0, NULL, 0));
}

bool UART::Receive(uint8_t *data, const size_t length, const TickType_t timeout) {
    if (data == NULL || length == 0) {
        return false;
    }

    uart_flush_input(uart_num);

    int32_t read_bytes = uart_read_bytes(uart_num, data, length, timeout);

    if (read_bytes < 0) {
        // Serial.print("UART read error: ");
        // Serial.println(read_bytes);
        return false;
    } else if (read_bytes == 0) {
        // Serial.println("UART read timeout: no data received.");
        return false;
    }

    return true;
}
