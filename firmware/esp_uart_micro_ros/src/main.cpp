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
    TickType_t timeout = 10;

    while (1) {
        if (uart.Receive(&data, length, timeout)) {
            // Serial.print("Received data: ");
            // Serial.println(data);
            rosComm.Publish(data);
        }
    }
}
