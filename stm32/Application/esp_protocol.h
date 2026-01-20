#ifndef APPLICATION_ESP_PROTOCOL_H_
#define APPLICATION_ESP_PROTOCOL_H_
/**********************************************************************************************************************
 * Binary Protocol Format:
 *  [CMD_BYTE:1][ARG_COUNT:1][ARGS:n][\n]
 * 
 *  CMD_BYTE format: [CMD_ID:7bits][DIR:1bit]
 *    - Upper 7 bits = command ID (0-127)
 *    - Lowest bit = direction (0=request, 1=response)
 * 
 *  Example - RPM request:  [0x04][0x00][\n]
 *  Example - RPM response: [0x05][0x02][-150,200\n]
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "message.h"
#include "error_messages.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define ESP_HEADER_SIZE 2  // CMD_BYTE + ARG_COUNT

#define ESP_MAKE_REQUEST(command) ((command) << 1 | 0)
#define ESP_MAKE_RESPONSE(command) ((command) << 1 | 1)
#define ESP_GET_CMD(byte) ((byte) >> 1)
#define ESP_IS_RESPONSE(byte) ((byte) & 0x01)

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eEspCmd {
    eEspCmd_First = 0,
    eEspCmd_Cmd = eEspCmd_First,
    eEspCmd_Rpm,
    eEspCmd_Last
} eEspCmd_t;

typedef enum eEspStatus {
    eEspStatus_First = 0,
    eEspStatus_Ok = eEspStatus_First,
    eEspStatus_Error,
    eEspStatus_Last
} eEspStatus_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool ESP_Protocol_FormCommand(sMessage_t *output_command, const uint8_t command, const sMessage_t params);
bool ESP_Protocol_InterpretCommand(const sMessage_t command, uint8_t *output_command, sMessage_t *output_params);
eErrorCode_t ESP_Protocol_Parse(const sMessage_t message, sMessage_t *response);

#endif /* APPLICATION_ESP_PROTOCOL_H_ */
