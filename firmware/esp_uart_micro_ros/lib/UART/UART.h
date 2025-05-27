#pragma Once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "driver/uart.h"

class UART {
    public:
        UART();
        void Init();
        bool Receive(uint8_t *data, const size_t length, const TickType_t timeout);
    private:
        uart_port_t uart_num;
};  
