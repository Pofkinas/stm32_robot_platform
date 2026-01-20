/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sUartDesc_t g_static_uart_lut[eUart_Last] = {
    [eUart_Debug] = {
        .periph = USART2,
        .baud = UART_2_BAUDRATE,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX_RX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB1_GRP1_PERIPH_USART2,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .nvic = USART2_IRQn,
        .ring_buffer_capacity = UART_DEBUG_BUFFER_CAPACITY
    },
    [eUart_Esp] = {
        .periph = USART1,
        .baud = UART_1_BAUDRATE,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX_RX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB2_GRP1_PERIPH_USART1,
        .enable_clock_fp = LL_APB2_GRP1_EnableClock,
        .nvic = USART1_IRQn,
        .ring_buffer_capacity = ESP_COMM_BUFFER_CAPACITY
    }
};

const static sUartApiConst_t g_static_uart_api_lut[eUart_Last] = {
    [eUart_Debug] = {
        .buffer_capacity = UART_DEBUG_BUFFER_CAPACITY,
        .mutex_send_attributes = {.name = "Debug_SendMutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .message_queue_attributes = {.name = "Debug_MessageQueue", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0, .mq_mem = NULL, .mq_size = 0}
    },
    [eUart_Esp] = {
        .buffer_capacity = ESP_COMM_BUFFER_CAPACITY,
        .mutex_send_attributes = {.name = "Esp_SendMutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .message_queue_attributes = {.name = "Esp_MessageQueue", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0, .mq_mem = NULL, .mq_size = 0}
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

const osThreadAttr_t g_fsm_thread_attributes = {
    .name = "UART_API",
    .stack_size = 256 * 8,
    .priority = (osPriority_t) osPriorityNormal
};

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_Config_IsCorrectUart (const eUart_t uart) {
    return (uart >= eUart_First) && (uart < eUart_Last);
}

const sUartDesc_t *UART_Config_GetUartDesc (const eUart_t uart) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return NULL;
    }

    return &g_static_uart_lut[uart];
}

const sUartApiConst_t *UART_Config_GetUartApiConst (const eUart_t uart) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return NULL;
    }

    return &g_static_uart_api_lut[uart];
}
