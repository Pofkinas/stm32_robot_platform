#include <stdio.h>

#include "uart.h"
#include "ros_communication.h"

main_loop();

void main () {
    UART uart;
    uart.Init();

    uRosComm rosComm;
    rosComm.Init();

    main_loop();
}

void main_loop() {
    char *data[2];
    size_t length = 2;
    TickType_t timeout = 100 / portTICK_PERIOD_MS;

    while (1) {
        if (uart.Receive(data, length, timeout)) {
            bool sensor_data = atoi(data[0]);

            rosComm.PublishSensorData(sensor_data);
        }
    }
}
