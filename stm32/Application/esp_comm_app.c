/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <esp_comm_app.h>

#include <ctype.h>
#include "cmsis_os2.h"
#include "cmd_api.h"
#include "uart_api.h"
#include "heap_api.h"
#include "debug_api.h"
#include "error_messages.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
#ifdef DEBUG_ESP_COMM_APP
CREATE_MODULE_NAME (ESP_COMM)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ESP_COMM_APP */

const static osThreadAttr_t g_esp_comm_thread_attributes = {
    .name = "ESP_Comm",
    .stack_size = ESP_COMM_THREAD_STACK_SIZE,
    .priority = (osPriority_t) ESP_COMM_THREAD_PRIORITY
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static bool g_is_initialized = false;

static osThreadId_t g_esp_comm_thread_id = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void ESP_Comm_Thread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void ESP_Comm_Thread (void *arg) {
    static char response_buffer[ESP_COMM_RESPONSE_MESSAGE_CAPACITY];

    static sMessage_t command = {.data = NULL, .size = 0};
    static sMessage_t response = {.data = response_buffer, .size = ESP_COMM_RESPONSE_MESSAGE_CAPACITY};
    
    while (true) {
        if (UART_API_Receive(ESP_COMM_UART, &command, osWaitForever)) {
            // TRACE_INFO("Received [%d bytes]: [0x%02X][0x%02X]%s\n", command.size, (uint8_t)command.data[0], (uint8_t)command.data[1], (command.size > 2) ? (const char*)&command.data[2] : "");

            eErrorCode_t result = ESP_Protocol_Parse(command, &response);
            
            if ((result != eErrorCode_OK) && (response.data != NULL)) {
                TRACE_WRN("[%s] %s", Error_Message_To_String(result), response.data);
            }
            
            Heap_API_Free(command.data);
        }

        osThreadYield();
    }
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool ESP_Comm_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (Heap_API_Init() == false) {
        return false;
    }

    if (UART_API_Init(ESP_COMM_UART, ESP_COMM_BAUDRATE, ESP_COMM_DELIMITER) == false) {
        return false;
    }

    g_esp_comm_thread_id = osThreadNew(ESP_Comm_Thread, NULL, &g_esp_comm_thread_attributes);

    if (g_esp_comm_thread_id == NULL) {
        return false;
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool ESP_Comm_APP_Send (const uint8_t command, const sMessage_t params, const uint32_t timeout) {
    static char buffer[ESP_COMM_MESSAGE_SIZE];
    
    static sMessage_t output_command = {.data = buffer, .size = ESP_COMM_MESSAGE_SIZE};

    if (!ESP_Protocol_FormCommand(&output_command, command, params)) {
        return false;
    }
    
    // TRACE_INFO("Sent [%d bytes]: [0x%02X][0x%02X]%s", output_command.size, output_command.data[0], output_command.data[1], (output_command.size > 2) ? (char*)&output_command.data[2] : "");

    return UART_API_Send(ESP_COMM_UART, output_command, timeout);
}
