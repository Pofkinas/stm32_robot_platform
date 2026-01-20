#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "message.h"
#include "driver/uart.h"
#include "driver/gpio.h"


class UART {
public:
    UART(uart_port_t uart_num, int tx_pin, int rx_pin, int baudrate, int rx_buffer_size, int tx_buffer_size, const char* delimiter);
    
    bool init();
    bool transmit(const sMessage_t message);
    bool receive(sMessage_t *message, const TickType_t timeout);
    bool flush();

private:
    uart_port_t uart_num;
    int tx_pin;
    int rx_pin;
    int baudrate;
    int rx_buffer_size;
    int tx_buffer_size;
    const char* delimiter;
};
