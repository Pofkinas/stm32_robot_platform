#ifndef CONFIG_UART_CONFIG_H_
#define CONFIG_UART_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "cmsis_os2.h"
#include "platform_config.h"
#include "baudrate.h"

#include UART_DRIVER
#include BUS_DRIVER

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eUart {
    eUart_First = 0,
    eUart_Debug = eUart_First,
    eUart_Esp,
    eUart_Last
} eUart_t;

typedef struct sUartDesc {
    USART_TypeDef *periph;
    eBaudrate_t baud;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t direction;
    uint32_t flow_control;
    uint32_t oversample;
    uint32_t clock;
    void (*enable_clock_fp) (uint32_t);
    IRQn_Type nvic;
    size_t ring_buffer_capacity;
} sUartDesc_t;

typedef struct sUartApiConst {
    size_t buffer_capacity;
    osMutexAttr_t mutex_send_attributes;
    osMessageQueueAttr_t message_queue_attributes;
} sUartApiConst_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osThreadAttr_t g_fsm_thread_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool UART_Config_IsCorrectUart (const eUart_t uart);
const sUartDesc_t *UART_Config_GetUartDesc (const eUart_t uart);
const sUartApiConst_t *UART_Config_GetUartApiConst (const eUart_t uart);

#endif /* CONFIG_UART_CONFIG_H_ */
