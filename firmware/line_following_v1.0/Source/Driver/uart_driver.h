#ifndef __UART_DRIVER__H__
#define __UART_DRIVER__H__
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "uart_baudrate.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eUartDriver {
    eUartDriver_First = 0,
    eUartDriver_1 = eUartDriver_First,
    eUartDriver_Last
} eUartDriver_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool UART_Driver_Init (const eUartDriver_t uart, const eUartBaudrate_t baudrate);
bool UART_Driver_SendByte (const eUartDriver_t uart, const uint8_t data);
bool UART_Driver_SendBytes (const eUartDriver_t uart, uint8_t *data, const size_t size);
bool UART_Driver_ReceiveByte (const eUartDriver_t uart, uint8_t *data);

#endif /* __UART_DRIVER__H__ */
