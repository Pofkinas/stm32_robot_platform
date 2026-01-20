#include "uart.h"
#include "esp_log.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "UART";

UART::UART(uart_port_t uart_num, int tx_pin, int rx_pin, int baudrate, int rx_buffer_size, int tx_buffer_size, const char* delimiter):
    uart_num(uart_num),
    tx_pin(tx_pin),
    rx_pin(rx_pin),
    baudrate(baudrate),
    rx_buffer_size(rx_buffer_size),
    tx_buffer_size(tx_buffer_size),
    delimiter(delimiter) {}

bool UART::init() {
    ESP_LOGI(TAG, "Init: port=%d, tx=%d, rx=%d, baud=%d, rxbuf=%d, txbuf=%d", uart_num, tx_pin, rx_pin, baudrate, rx_buffer_size, tx_buffer_size);
    
    uart_config_t uart_config = {};
    
    uart_config.baud_rate = baudrate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    if (uart_param_config(uart_num, &uart_config) != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed");
        
        return false;
    }

    if (uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed");
        
        return false;
    }

    if (uart_driver_install(uart_num, rx_buffer_size, tx_buffer_size, 0, NULL, 0) != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed");
        
        return false;
    }

    ESP_LOGI(TAG, "Init success");
    
    return true;
}

bool UART::transmit(const sMessage_t message) {
    if (message.data == NULL || message.size == 0) {
        return false;
    }

    int written = uart_write_bytes(uart_num, message.data, message.size);
    
    if (written < 0 || (size_t)written < message.size) {
        return false;
    }

    return true;
}

bool UART::receive(sMessage_t *message, const TickType_t timeout) {
    if (message->data == NULL || message->size == 0) {
        return false;
    }

    if (delimiter == NULL) {
        int read = uart_read_bytes(uart_num, message->data, rx_buffer_size, timeout);
        
        if (read <= 0) {
            return false;
        }
        
        message->size = (size_t)read;
        return true;
    }

    size_t total_read = 0;
    uint8_t rx_byte;
    TickType_t start_time = xTaskGetTickCount();

    while (total_read < message->size) {
        int read = uart_read_bytes(uart_num, &rx_byte, 1, timeout);
        
        if (read <= 0) {
            return false;
        }
        
        message->data[total_read++] = rx_byte;
        
        if (rx_byte == delimiter[0]) {
            break;
        }

        if (timeout != portMAX_DELAY) {
            TickType_t elapsed = xTaskGetTickCount() - start_time;
            if (elapsed >= timeout) {
                return false;
            }
        }
    }
    
    if (total_read >= message->size && message->data[total_read - 1] != delimiter[0]) {
        return false;
    }
    
    message->size = total_read;
    
    return true;
}

bool UART::flush() {
    if (uart_flush(uart_num) != ESP_OK) {
        return false;
    }

    return true;
}
