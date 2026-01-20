/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "esp_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_api.h"
#include "default_cli_lut.h"
#include "custom_cli_lut.h"
#include "cmd_api_helper.h"
#include "odometry_api.h"
#include "error_messages.h"
#include "debug_api.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define ESP_COMM_SEPARATOR_LENGTH (sizeof(ESP_COMM_ARG_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef eErrorCode_t (*ESP_Protocol_Handler_t)(sMessage_t arguments, sMessage_t *response);

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

eErrorCode_t ESP_Protocol_Rsp_Handlers_RPM (sMessage_t arguments, sMessage_t *response);
eErrorCode_t ESP_Protocol_Req_Handlers_CMD (sMessage_t arguments, sMessage_t *response);

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_ESP_PROTOCOL
CREATE_MODULE_NAME (ESP_PROTOCOL)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ESP_COMM */

/* clang-format off */
const ESP_Protocol_Handler_t g_esp_response_handlers[eEspCmd_Last] = {
    [eEspCmd_Cmd] = NULL,
    [eEspCmd_Rpm] = ESP_Protocol_Rsp_Handlers_RPM
};

const ESP_Protocol_Handler_t g_esp_request_handlers[eEspCmd_Last] = {
    [eEspCmd_Cmd] = ESP_Protocol_Req_Handlers_CMD,
    [eEspCmd_Rpm] = NULL
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
eErrorCode_t ESP_Protocol_Rsp_Handlers_RPM (sMessage_t arguments, sMessage_t *response) {
    if ((response == NULL) || (response->data == NULL)) {
        return eErrorCode_NULLPTR;
    }
    
    if (arguments.data == NULL) {
        sprintf(response->data, "Invalid arguments data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size == 0) {
        sprintf(response->data, "Missing arguments\n");
        
        return eErrorCode_PARSE;
    }

    // TODO: Parse encoder IDs from arguments binary data.
    // arguments.size indicates how many encoder IDs are requested.
    // arguments.data first arg before separator is some size_t representing Encoder ID combination, 
    // eg. 0b00000001 for ENCODER_RIGHT_1, 0b00000010 for ENCODER_LEFT_1, etc.

    uint16_t encoder_rpms[arguments.size];
    eErrorCode_t error = eErrorCode_Last;
    size_t encoders = arguments.size;
    arguments.size = strlen(arguments.data); // Reset size to actual data length for parsing.
    size_t arg = 0;

    for (; (arg < encoders && arg < eEncoder_Last);) {
        size_t rmp = 0;
        error = CMD_API_Helper_FindNextArgUInt(&arguments, &rmp, ESP_COMM_ARG_SEPARATOR, ESP_COMM_SEPARATOR_LENGTH, response);
    
        if (error != eErrorCode_OK) {
            return error;
        }
        
        encoder_rpms[arg] = (uint16_t)rmp;
        arg++;
    }

    if (!Odometry_API_UpdateRpm(encoder_rpms, arg)) {
        sprintf(response->data, "Failed to update odometry\n");
        
        return eErrorCode_FAILED;
    }

    return eErrorCode_OK;
}

eErrorCode_t ESP_Protocol_Req_Handlers_CMD (sMessage_t arguments, sMessage_t *response) {
    if ((response == NULL) || (response->data == NULL)) {
        return eErrorCode_NULLPTR;
    }
    
    if (arguments.data == NULL) {
        sprintf(response->data, "Invalid arguments data pointer\n");

        return eErrorCode_NULLPTR;
    }

    eErrorCode_t error_code = eErrorCode_NOTFOUND;
    sMessage_t args_message = {.data = arguments.data, .size = strlen((char *)arguments.data)};
    
    #ifdef ENABLE_DEFAULT_CMD
    error_code = CMD_API_FindCommand(args_message, response, g_default_cmd_lut, eCliDefaultCmd_Last);
    #endif /* ENABLE_DEFAULT_CMD */

    #ifdef ENABLE_CUSTOM_CMD
    if (error_code == eErrorCode_NOTFOUND) {
        error_code = CMD_API_FindCommand(args_message, response, g_custom_cmd_lut, eCliCustomCmd_Last);
    }
    #endif /* ENABLE_CUSTOM_CMD */

    return error_code;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool ESP_Protocol_FormCommand (sMessage_t *output_command, const uint8_t command, const sMessage_t params) {
    if (output_command == NULL || output_command->data == NULL) {
        return false;
    }

    uint8_t param_count = (params.data != NULL) ? params.size : 0;
    size_t message_size = (params.data != NULL) ? strlen((char *)params.data) : 0;
    size_t delimiter_length = sizeof(ESP_COMM_DELIMITER) - 1;
    
    // Command byte
    output_command->data[0] = command;
    // Argument count
    output_command->data[1] = param_count;
    
    if (params.data != NULL) {
        memcpy(&output_command->data[ESP_HEADER_SIZE], params.data, message_size);
    }
    
    memcpy(&output_command->data[ESP_HEADER_SIZE + message_size], ESP_COMM_DELIMITER, delimiter_length);
    output_command->size = ESP_HEADER_SIZE + message_size + delimiter_length;
    
    return true;
}

bool ESP_Protocol_InterpretCommand (const sMessage_t command, uint8_t *output_command, sMessage_t *output_params) {
    if ((sizeof(command.data) < 2) || (output_command == NULL) || (output_params == NULL)) {
        return false;
    }

    *output_command = command.data[0];
    output_params->size = command.data[1];
    output_params->data = &command.data[2];

    return true;
}

eErrorCode_t ESP_Protocol_Parse(const sMessage_t message, sMessage_t *response) {
    if ((response == NULL) || (response->data == NULL)) {
        return eErrorCode_NULLPTR;
    }

    uint8_t command_byte = 0;
    sMessage_t arguments = {.data = NULL, .size = 0};

    if (!ESP_Protocol_InterpretCommand(message, &command_byte, &arguments)) {
        sprintf(response->data, "Failed to interpret command\n");

        return eErrorCode_PARSE;
    }
    
    eEspCmd_t command = ESP_GET_CMD(command_byte);
    bool is_response = ESP_IS_RESPONSE(command_byte);
    
    if ((command < eEspCmd_First) || (command >= eEspCmd_Last)) {
        sprintf(response->data, "Invalid command ID: %d\n", command);
        
        return eErrorCode_PARSE;
    }

    TRACE_INFO("Parsed cmd: ID=%d, is_response=%d, arg_size=%u\n", command, is_response, arguments.size);
    
    if (is_response) {
        if (g_esp_response_handlers[command] != NULL) {
            return g_esp_response_handlers[command](arguments, response);
        } else {
            sprintf(response->data, "No response handler for command %d\n", command);
            
            return eErrorCode_NOTFOUND;
        }
    } else {
        if (g_esp_request_handlers[command] != NULL) {
            return g_esp_request_handlers[command](arguments, response);
        } else {
            sprintf(response->data, "No request handler for command %d\n", command);
            
            return eErrorCode_NOTFOUND;
        }
    }
}
