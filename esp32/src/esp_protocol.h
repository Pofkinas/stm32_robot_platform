#pragma once

/**********************************************************************************************************************
 * Binary Protocol Format:
 *  [CMD_BYTE:1][ARG_COUNT:1][ARGS:n][\n]
 * 
 *  CMD_BYTE format: [CMD_ID:7bits][DIR:1bit]
 *    - Upper 7 bits = command ID (0-127)
 *    - Lowest bit = direction (0=request, 1=response)
 * 
 *  Example - RPM request from STM32:  [0x04][0x00][\n]
 *  Example - RPM response from ESP32: [0x05][0x02][150;200\n]
 *********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "message.h"

#define ESP_MAKE_REQUEST(command) ((command) << 1 | 0)
#define ESP_MAKE_RESPONSE(command) ((command) << 1 | 1)
#define ESP_GET_CMD(byte) ((byte) >> 1)
#define ESP_IS_RESPONSE(byte) ((byte) & 0x01)

typedef enum eEspCmd {
    eEspCmd_First = 0,
    eEspCmd_CMD = eEspCmd_First,
    eEspCmd_RPM,
    eEspCmd_Last
} eEspCmd_t;

typedef struct sCmdDesc {
    const char* command;
    size_t command_length;
} sCmdDesc_t;

bool esp_protocol_form_command(sMessage_t *output_command, const uint8_t command, const sMessage_t params);
bool esp_protocol_interpret_command(const sMessage_t command, uint8_t *output_command, sMessage_t *output_params);
bool esp_protocol_parse(const sMessage_t message, sMessage_t *response);
