/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "debug_api.h"
#include "cmsis_os2.h"
#include "message.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DELIMITER "\r\n"
#define DEBUG_MESSAGE_SIZE 256
#define DEBUG_MESSAGE_TIMEOUT 1000
#define DEBUG_MUTEX_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static osMutexAttr_t g_debug_api_mutex_attributes = {
    .name = "Debug_API_mutex", 
    .attr_bits = osMutexRecursive | osMutexPrioInherit, 
    .cb_mem = NULL, 
    .cb_size = 0U
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static char g_debug_message_buffer[DEBUG_MESSAGE_SIZE] = {0};
static bool g_is_initialized = false;
static osMutexId_t g_debug_api_mutex = NULL;

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

bool Debug_API_Init (const eUartBaudrate_t baudrate) {
    if (g_is_initialized) {
        return false;
    }
    
    if ((baudrate < eUartBaudrate_First) || (baudrate >= eUartBaudrate_Last)) {
        return false;
    }

    if (g_debug_api_mutex == NULL) {
        g_debug_api_mutex = osMutexNew(&g_debug_api_mutex_attributes);
    }

    g_is_initialized = UART_API_Init(eUart_Debug, baudrate, DELIMITER);

    return g_is_initialized;
}

bool Debug_API_Print (const eTraceLevel_t trace_level, const char *file_trace, const char *file_name, const size_t line_number, const char *format, ...) {
    if ((trace_level < eTraceLevel_First) || (trace_level >= eTraceLevel_Last)) {
        return false;
    }

    if ((file_trace == NULL) || (format == NULL) || (file_name == NULL) || (format == NULL)) {
        return false;
    }

    if (osMutexAcquire(g_debug_api_mutex, DEBUG_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    static sMessage_t debug_message = {.data = NULL, .size = 0};
    static size_t message_lenght = 0;

    va_list arguments;
    
    debug_message.data = g_debug_message_buffer;

    switch (trace_level) {
        case eTraceLevel_Info: {
            message_lenght = sprintf(debug_message.data, "[%s.INF] ", file_trace);
        } break;
        case eTraceLevel_Warning: {
            message_lenght = sprintf(debug_message.data, "[%s.WRN] ", file_trace);
        } break;
        case eTraceLevel_Error: {
            message_lenght = sprintf(debug_message.data, "[%s.ERR] (file: %s, line: %d) ", file_trace, file_name, line_number);
        } break;
        default: {
        } break;
    }

    va_start(arguments, format);

    message_lenght += vsprintf((debug_message.data + message_lenght), format, arguments);

    va_end(arguments);
    
    debug_message.size = message_lenght;
    bool is_sent = UART_API_Send(eUart_Debug, debug_message, DEBUG_MESSAGE_TIMEOUT);
    
    osMutexRelease(g_debug_api_mutex);

    return is_sent;
}
