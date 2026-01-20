#include "esp_protocol.h"
#include <stdio.h>
#include <string.h>
#include "encoder.h"
#include "main.h"
#include "debug.h"
#include "config.h"

#define DEFINE_CMD(command_string) .command = command_string, .command_length = sizeof(command_string) - 1

typedef bool (*ESP_Protocol_Handler_t)(sMessage_t arguments, sMessage_t *response);

static bool esp_protocol_req_handlers_rpm (sMessage_t arguments, sMessage_t *response);

static const ESP_Protocol_Handler_t g_esp_response_handlers[eEspCmd_Last] = {
    [eEspCmd_CMD] = NULL,
    [eEspCmd_RPM] = NULL
};

static const ESP_Protocol_Handler_t g_esp_request_handlers[eEspCmd_Last] = {
    [eEspCmd_CMD] = NULL,
    [eEspCmd_RPM] = esp_protocol_req_handlers_rpm
};

static bool esp_protocol_req_handlers_rpm (sMessage_t arguments, sMessage_t *response) {
    if ((response == NULL) || (response->data == NULL)) {
        return false;
    }

    char args_buffer[CMD_BUFFER_SIZE - ESP_HEADER_SIZE] = {0};
    size_t param_count = 0;

    if (arguments.size == 0) {
        for (eEncoderId_t encoder_id = eEncoderId_First; encoder_id < eEncoderId_Last; encoder_id = static_cast<eEncoderId_t>(encoder_id + 1)) {
            uint16_t rpm = get_encoder_rpm(encoder_id);
            char rpm_str[32];
            snprintf(rpm_str, sizeof(rpm_str), "%u", rpm);
            strcat(args_buffer, rpm_str);

            if (encoder_id < (eEncoderId_Last - 1)) {
                memcpy(args_buffer + strlen(args_buffer), ESP_ARG_SEPARATOR, sizeof(ESP_ARG_SEPARATOR) - 1);
            }

            param_count++;
        }
    } else {
        // TODO: Parse encoder IDs from arguments binary data.
        // arguments.size indicates how many encoder IDs are requested.
        // arguments.data first arg before separator is some size_t representing Encoder ID combination, 
        // eg. 0b00000001 for ENCODER_RIGHT_1, 0b00000010 for ENCODER_LEFT_1, etc.

        if (arguments.data == NULL) {
            sprintf((char*)response->data, "Invalid arguments data pointer\n");

            return false;
        }

        sprintf((char*)response->data, "No single encoder RPM request handling implemented\n");

        return false;
    }

    sMessage_t params = {.data = args_buffer, .size = param_count};

    return comm_send(ESP_MAKE_RESPONSE(eEspCmd_RPM), params, CMD_COMMS_TIMEOUT);
}

bool esp_protocol_form_command (sMessage_t *output_command, const uint8_t command, const sMessage_t params) {
    if (output_command == NULL || output_command->data == NULL) {
        return false;
    }

    uint8_t param_count = (params.data != NULL) ? params.size : 0;
    size_t message_size = (params.data != NULL) ? strlen(params.data) : 0;
    size_t delimiter_length = sizeof(ESP_MSG_DELIMITER) - 1;
    
    // Command byte
    output_command->data[0] = command;
    // Argument count
    output_command->data[1] = param_count;
    
    if (params.data != NULL) {
        memcpy(&output_command->data[ESP_HEADER_SIZE], params.data, message_size);
    }
    
    memcpy(&output_command->data[ESP_HEADER_SIZE + message_size], ESP_MSG_DELIMITER, delimiter_length);
    output_command->size = ESP_HEADER_SIZE + message_size + delimiter_length;
    
    return true;
}

bool esp_protocol_interpret_command (const sMessage_t command, uint8_t *output_command, sMessage_t *output_params) {
    if ((command.size < 2) || (output_command == NULL) || (output_params == NULL)) {
        return false;
    }

    *output_command = command.data[0];
    output_params->size = command.data[1];
    output_params->data = &command.data[2];

    return true;
}

bool esp_protocol_parse(const sMessage_t message, sMessage_t *response) {
    if ((response == NULL) || (response->data == NULL)) {
        return false;
    }

    uint8_t command_byte = 0;
    sMessage_t arguments = {.data = NULL, .size = 0};

    if (!esp_protocol_interpret_command(message, &command_byte, &arguments)) {
        sprintf((char*)response->data, "Failed to interpret command\n");

        return false;
    }
    
    size_t command_value = ESP_GET_CMD(command_byte);
    eEspCmd_t command = static_cast<eEspCmd_t>(command_value);
    bool is_response = ESP_IS_RESPONSE(command_byte);
    
    if ((command < eEspCmd_First) || (command >= eEspCmd_Last)) {
        sprintf((char*)response->data, "Invalid command ID: %d\n", command);
        
        return false;
    }

    //DEBUG_INFO("Parsed command: ID=%d, is_response=%d, arg_size=%d\n", command, is_response, arguments.size);

    if (is_response) {
        if (g_esp_response_handlers[command] != NULL) {
            return g_esp_response_handlers[command](arguments, response);
        } else {
            sprintf((char*)response->data, "No response handler for command %d\n", command);
            
            return false;
        }
    } else {
        if (g_esp_request_handlers[command] != NULL) {
            return g_esp_request_handlers[command](arguments, response);
        } else {
            sprintf((char*)response->data, "No request handler for command %d\n", command);
            
            return false;
        }
    }
}
