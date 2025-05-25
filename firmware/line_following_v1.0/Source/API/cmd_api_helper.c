/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cmd_api_helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug_api.h"
#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_CMD_API_HELPER

#define BASE_10 10

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CMD_API_HELPER
CREATE_MODULE_NAME (CMD_API_HELPER)
#else
CREATE_MODULE_NAME_EMPTY
#endif

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

int CMD_API_Helper_FindNextArgUInt (sMessage_t *argument, size_t *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response) {
    if ((argument == NULL) || (return_argument == NULL) || (separator == NULL) || (response == NULL)) {
        TRACE_ERR("Invalid data pointer");
        
        return eErrorCode_INVAL;
    }

    if ((argument->data == NULL) || (response->data == NULL)) {
        TRACE_ERR("Invalid argument/response data pointer");

        return eErrorCode_INVAL;
    }

    if (argument->size == 0) {
        snprintf(response->data, response->size, "No expected arguments\n");

        return eErrorCode_NOMSG;
    }

    char *invalid_character;
    char *argument_token = strstr(argument->data, separator);

    if (argument_token != NULL) {
        *argument_token = '\0';
    }

    *return_argument = strtoul(argument->data, &invalid_character, BASE_10);

    if (*invalid_character != '\0') {
        snprintf(response->data, response->size, "%s: Invalid argument; Use digits separated by: '%s'\n", invalid_character, separator);

        return eErrorCode_INVAL;
    }

    if (argument_token == NULL) {
        argument->size = 0;
        
        return eErrorCode_OSOK;
    }

    argument->size -= (argument_token - argument->data + separator_lenght);
    argument->data = argument_token + separator_lenght;

    return eErrorCode_OSOK;
}
