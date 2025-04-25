/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_api.h"
#include "cmsis_os2.h"
#include "uart_driver.h"
#include "heap_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define UART2_BUFFER_CAPACITY 256
#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_PUT_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eState {
    eState_First,
    eState_Setup = eState_First,
    eState_Collect,
    eState_Flush,
    eState_Last
} eState_t;

typedef struct sUartConst {
    size_t queue_capacity;
    osMutexAttr_t mutex_send_attributes;
    osMessageQueueAttr_t message_queue_attributes;
} sUartConst_t;

typedef struct sUartDynamic {
    eState_t current_state;
    bool is_initialized;
    osMutexId_t mutex_send;
    osMessageQueueId_t message_queue;
    sMessage_t message;
    char *delimiter;
    size_t delimiter_length;
} sUartDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static osThreadAttr_t g_fsm_thread_attributes = {
    .name = "FSM_Thread",
    .stack_size = 256 * 8,
    .priority = (osPriority_t) osPriorityNormal
};

/* clang-format off */
const static sUartConst_t g_static_uart_lut[eUart_Last] = {
    [eUart_Debug] = {
        .queue_capacity = UART2_BUFFER_CAPACITY,
        .mutex_send_attributes = {.name = "Debug_SendMutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .message_queue_attributes = {.name = "Debug_MessageQueue", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0, .mq_mem = NULL, .mq_size = 0}
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osThreadId_t g_fsm_thread_id = NULL;

/* clang-format off */
static sUartDynamic_t g_dynamic_uart_lut[eUart_Last] = {
    [eUart_Debug] = {
        .current_state = eState_Setup,
        .is_initialized = false,
        .mutex_send = NULL,
        .message_queue = NULL,
        .message = {.data = NULL, .size = 0},
        .delimiter = NULL,
        .delimiter_length = 0
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void UART_API_FsmThread (void *arg);
static bool UART_API_IsDelimiterReceived (const eUart_t uart);
static void UART_API_BufferIncrement (const eUart_t uart);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void UART_API_FsmThread (void *arg) {
    while (1) {
        for (eUart_t uart = eUart_First; uart < eUart_Last; uart++) {
            if (g_dynamic_uart_lut[uart].is_initialized == false) {
                continue;
            }

            switch (g_dynamic_uart_lut[uart].current_state) {
                case eState_Setup: {
                    g_dynamic_uart_lut[uart].message.data = Heap_API_Calloc(g_static_uart_lut[uart].queue_capacity, sizeof(char));
                    
                    if (g_dynamic_uart_lut[uart].message.data == NULL) {
                            continue;
                    }
                    
                    g_dynamic_uart_lut[uart].message.size = 0;
                    g_dynamic_uart_lut[uart].current_state = eState_Collect;
                }
                case eState_Collect: {
                    uint8_t received_byte = 0;

                    while (UART_Driver_ReceiveByte(uart, &received_byte) == true) {
                        g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size] = received_byte;

                        UART_API_BufferIncrement(uart);

                        if (UART_API_IsDelimiterReceived(uart) == false) {
                            continue;
                        }

                        g_dynamic_uart_lut[uart].message.size -= g_dynamic_uart_lut[uart].delimiter_length;
                        g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size] = '\0';

                        g_dynamic_uart_lut[uart].current_state = eState_Flush;

                        break;
                    }

                    if (g_dynamic_uart_lut[uart].current_state != eState_Flush) {
                        continue;
                    }
                }
                case eState_Flush: {
                    if (osMessageQueuePut(g_dynamic_uart_lut[uart].message_queue, &g_dynamic_uart_lut[uart].message, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_PUT_TIMEOUT) != osOK) {
                        continue;
                    }

                    g_dynamic_uart_lut[uart].current_state = eState_Setup;
                } break;
                default: {  
                } break;
            }
        }
    }

    osThreadYield();
}

static void UART_API_BufferIncrement (const eUart_t uart) {
    g_dynamic_uart_lut[uart].message.size++;

    if (g_dynamic_uart_lut[uart].message.size >= g_static_uart_lut[uart].queue_capacity) {
        g_dynamic_uart_lut[uart].message.size = 0;
    }

    return;
}

static bool UART_API_IsDelimiterReceived (const eUart_t uart) {
    if (g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size - 1] != g_dynamic_uart_lut[uart].delimiter[g_dynamic_uart_lut[uart].delimiter_length - 1]) {
        return false;
    } 

    if (strstr(g_dynamic_uart_lut[uart].message.data, g_dynamic_uart_lut[uart].delimiter) == NULL) {
        return false;
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_API_Init (const eUart_t uart, const eUartBaudrate_t baudrate, const char *delimiter) {
    if ((uart < eUart_First) || (uart >= eUart_Last)) {
        return false;
    }

    if ((baudrate < eUartBaudrate_First) || (baudrate >= eUartBaudrate_Last)) {
        return false;
    }

    if (delimiter == NULL) {
        return false;
    }

    if (g_dynamic_uart_lut[uart].is_initialized == true) {
        return false;
    }
    
    if (UART_Driver_Init(uart, baudrate) == false) {
        return false;
    }

    g_dynamic_uart_lut[uart].mutex_send = osMutexNew(&g_static_uart_lut[uart].mutex_send_attributes);
    
    if (g_dynamic_uart_lut[uart].mutex_send == NULL) {
        return false;
    }

    g_dynamic_uart_lut[uart].message_queue = osMessageQueueNew(g_static_uart_lut[uart].queue_capacity, sizeof(sMessage_t), &g_static_uart_lut[uart].message_queue_attributes);

    if (g_dynamic_uart_lut[uart].message_queue == NULL) {
        return false;
    }

    g_dynamic_uart_lut[uart].delimiter_length = strlen(delimiter);
    g_dynamic_uart_lut[uart].delimiter = Heap_API_Calloc((g_dynamic_uart_lut[uart].delimiter_length + 1), sizeof(char));

    if (g_dynamic_uart_lut[uart].delimiter == NULL) {
        return false;
    }

    memcpy(g_dynamic_uart_lut[uart].delimiter, delimiter, g_dynamic_uart_lut[uart].delimiter_length + 1);

    g_dynamic_uart_lut[uart].is_initialized = true;

    if (g_fsm_thread_id == NULL) {
        g_fsm_thread_id = osThreadNew(UART_API_FsmThread, NULL, &g_fsm_thread_attributes);
    }

    return true;
}

bool UART_API_Send (const eUart_t uart, const sMessage_t message, const uint32_t timeout) {
    if ((uart < eUart_First) || (uart >= eUart_Last)) {
        return false;
    }
    
    if (osMutexAcquire(g_dynamic_uart_lut[uart].mutex_send, timeout) != osOK) {
        return false;
    }

    if (!UART_Driver_SendBytes(uart, (uint8_t*) message.data, message.size)) {
        osMutexRelease(g_dynamic_uart_lut[uart].mutex_send);
        
        return false;
    }

    osMutexRelease(g_dynamic_uart_lut[uart].mutex_send);

    return true;
}

bool UART_API_Receive (const eUart_t uart, sMessage_t *message, const uint32_t timeout) {
    if ((uart < eUart_First) || (uart >= eUart_Last)) {
        return false;
    }

    if (message == NULL) {
        return false;
    }

    if (osMessageQueueGet(g_dynamic_uart_lut[uart].message_queue, message, MESSAGE_QUEUE_PRIORITY, timeout) != osOK) {
        return false;
    }

    return true;
}
