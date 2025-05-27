#include <Arduino.h>
#include <stdio.h>

#include "UART.h"
#include "uROS_Comms.h"

UART uart;
uRosComms rosComm;

void main_loop();

void setup () {
    uart.Init();
    rosComm.Init();
}

void loop () {
    uint8_t data;
    size_t length = 1;
    TickType_t timeout = 100 / portTICK_PERIOD_MS;

    while (1) {
        if (uart.Receive(&data, length, timeout)) {
            rosComm.Publish(data);
        }

        delay(100);
    }
}
