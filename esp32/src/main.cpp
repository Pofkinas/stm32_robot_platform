#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "uart.h"
#include "debug.h"
#include "encoder.h"
#include "esp_protocol.h"
#include "main.h"
#include "wifi.h"
#include "web_server.h"
#include "logger.h"
#include "config.h"

static Encoder encoders[eEncoderId_Last] = {
    Encoder(eEncoderId_RIGHT_1, ENCODER_RIGHT_PIN),
    Encoder(eEncoderId_LEFT_1, ENCODER_LEFT_PIN)
};

UART comm_uart(CMD_UART, CMD_UART_TX, CMD_UART_RX, CMD_BAUDRATE, CMD_BUFFER_SIZE, CMD_BUFFER_SIZE, ESP_MSG_DELIMITER);
WiFi wifi(WIFI_SSID, WIFI_PASSWORD);
WebServer web_server(WEB_SERVER_PORT);

char rx_buffer[CMD_BUFFER_SIZE];
sMessage_t rx_message = {.data = rx_buffer, .size = CMD_BUFFER_SIZE};

char response_buffer[CMD_RESPONSE_SIZE];
sMessage_t response_message = {.data = response_buffer, .size = CMD_RESPONSE_SIZE};

static void cmd_task(void *pvParameters);

bool web_command_handler(char* cmd, int type, sMessage_t params, sMessage_t *response) {
    if ((cmd == NULL) || (response == NULL)) {
        return false;
    }
    
    eEspCmd_t cmd_type = static_cast<eEspCmd_t>(type);
    
    DEBUG_INFO("Web command: cmd=%s type=%d params=%s\n", cmd, type, (params.data != NULL) ? params.data : "");
    
    // Build protocol command byte
    uint8_t protocol_cmd = ESP_MAKE_REQUEST(cmd_type);
    
    sMessage_t final_params = {.data = NULL, .size = params.size};
    char param_buffer[CMD_BUFFER_SIZE] = {0};

    switch (cmd_type) {
        case eEspCmd_CMD: {
            int written = 0;
            
            if (params.size > 0 && params.data[0] != '\0') {
                // If cmd already contains the ':' delimiter (e.g. "set_pid:"), don't add another one.
                if (strchr(cmd, ':') != NULL) {
                    written = snprintf(param_buffer, CMD_BUFFER_SIZE, "%s%s", cmd, params.data);
                } else {
                    written = snprintf(param_buffer, CMD_BUFFER_SIZE, "%s:%s", cmd, params.data);
                }
            } else {
                written = snprintf(param_buffer, CMD_BUFFER_SIZE, "%s", cmd);
            }
            
            if (written <= 0) {
                snprintf(response->data, response->size, "Failed to format command parameters");
                
                return false;
            }
            
            final_params.data = param_buffer;
        } break;
        case eEspCmd_RPM: {
            final_params = params;      
        } break;
        default: {
            snprintf(response->data, response->size, "Unknown command type: %d", type);
            
            return false;
        } break;
    }
        
    return comm_send(protocol_cmd, final_params, CMD_COMMS_TIMEOUT);
}

void logger_callback(const char* message) {
    web_server.add_log_message(message);
}

extern "C" void app_main() {
    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    if (!Debug::init()) {
        return;
    }
    
    gpio_install_isr_service(0);
    
    if (!comm_uart.init()) {
        DEBUG_ERROR("Failed to init UART\n");
        
        return;
    }

    if (!comm_uart.flush()) {
        DEBUG_ERROR("Failed to flush UART\n");
        
        return;
    }

    for (eEncoderId_t encoder = eEncoderId_First; encoder < eEncoderId_Last; encoder = static_cast<eEncoderId_t>(encoder + 1)) {
        if (!encoders[encoder].init()) {
            DEBUG_ERROR("Failed to init encoder ID %d\n", encoder);
            
            return;
        }
    }

    if (!wifi.init()) {
        DEBUG_ERROR("Failed to init WiFi\n");
        
        return;
    }
    
    if (!wifi.connect()) {
        DEBUG_ERROR("Failed to connect to WiFi\n");
        
        return;
    }
    
    DEBUG_INFO("Connected to WiFi IP: %s\n", wifi.get_ip_address());
    
    // Initialize Logger with web server callback
    Logger::init();
    Logger::set_callback(logger_callback);
    
    if (!web_server.init()) {
        DEBUG_ERROR("Failed to init web server\n");
        return;
    }
    
    web_server.set_command_handler(web_command_handler);
    DEBUG_INFO("Web server started at http://%s:%d\n", wifi.get_ip_address(), WEB_SERVER_PORT);
    
    TaskHandle_t cmd_task_handle = NULL;

    xTaskCreate(cmd_task, "Cmd_task", CMD_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &cmd_task_handle);

    if (cmd_task_handle == NULL) {
        DEBUG_ERROR("Failed to create cmd_task\n");
        return;
    }
    
    DEBUG_INFO("Start OK\n");
    LOG_ESP("Start OK\n");

    vTaskDelete(NULL);
}

static void cmd_task(void *pvParameters) {
    while (1) {
        rx_message.size = CMD_BUFFER_SIZE;
        if (comm_uart.receive(&rx_message, portMAX_DELAY)) {
            if (rx_message.size != 0) {
                size_t len = rx_message.size;
                if (len >= CMD_BUFFER_SIZE) {
                    len = CMD_BUFFER_SIZE - 1;
                }

                rx_message.data[len] = '\0';
                
                while (len > 0 && (rx_message.data[len - 1] == '\n' || rx_message.data[len - 1] == '\r')) {
                    rx_message.data[len - 1] = '\0';
                    len--;
                }

                if (len > 0) {
                    DEBUG_INFO("Received: %s\n", rx_message.data);
                    LOG_STM("%s\n", rx_message.data);
                }

                if (!esp_protocol_parse(rx_message, &response_message)) {
                    // DEBUG_WARNING("%s", response_message.data);
                    // LOG_ESP("%s", response_message.data);
                }
            } else {
                // DEBUG_WARNING("Received empty message\n");
                // LOG_ESP("Received empty message\n");
            }

            memset(rx_message.data, 0, CMD_BUFFER_SIZE);
        }
    }
}

bool comm_send (const uint8_t command, const sMessage_t params, const uint32_t timeout) {
    static char buffer[CMD_BUFFER_SIZE] = {0};
    
    static sMessage_t output_command = {.data = buffer, .size = CMD_BUFFER_SIZE};
    memset(buffer, 0, CMD_BUFFER_SIZE);

    if (!esp_protocol_form_command(&output_command, command, params)) {
        return false;
    }

    DEBUG_INFO("Sent [%d bytes]: [0x%02X][0x%02X]%s", output_command.size, output_command.data[0], output_command.data[1], (output_command.size > 2) ? (char*)&output_command.data[2] : "");
    //LOG_ESP("Sent [%d bytes]: [0x%02X][0x%02X]%s", output_command.size, output_command.data[0], output_command.data[1], (output_command.size > 2) ? (char*)&output_command.data[2] : "");
    
    return comm_uart.transmit(output_command);
}

uint16_t get_encoder_rpm(const eEncoderId_t encoder) {
    if (encoder < eEncoderId_First || encoder >= eEncoderId_Last) {
        return false;
    }

    return encoders[encoder].getRPM();
}
